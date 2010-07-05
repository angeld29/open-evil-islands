/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
#include "cememfile.h"
#include "cemobfile.h"

static void ce_mob_decrypt_script(char* data, size_t size, uint32_t key)
{
	for (size_t i = 0; i < size; ++i) {
		key += (((((key * 13) << 4) + key) << 8) - key) * 4 + 2531011;
		data[i] ^= key >> 16;
	}
}

static ce_string* ce_mob_file_read_string(ce_mem_file* mem_file, size_t size)
{
	char data[size];
	ce_mem_file_read(mem_file, data, 1, size);
	return ce_string_new_str_n(data, size);
}

// callbacks

static void ce_mob_file_block_loop(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size);

static void ce_mob_file_block_unknown(ce_mob_file* CE_UNUSED(mob_file), ce_mem_file* mem_file, size_t size)
{
	ce_mem_file_skip(mem_file, size);
}

static void ce_mob_file_block_main(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	ce_mob_file_block_loop(mob_file, mem_file, size);
}

static void ce_mob_file_block_quest(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(0 == size);
	ce_mob_file_block_loop(mob_file, mem_file, size);
}

static void ce_mob_file_block_zonal(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(0 == size);
	ce_mob_file_block_loop(mob_file, mem_file, size);
}

static void ce_mob_file_block_text(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	// WARNING: string is encrypted and may contain null characters
	assert(NULL == mob_file->script);
	char data[size -= 4];
	uint32_t key = ce_mem_file_read_u32le(mem_file);
	ce_mem_file_read(mem_file, data, 1, size);
	ce_mob_decrypt_script(data, size, key);
	mob_file->script = ce_string_new_str_n(data, size);
}

static void ce_mob_file_block_object(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(NULL == mob_file->objects);
	mob_file->objects = ce_vector_new();
	ce_mob_file_block_loop(mob_file, mem_file, size);
}

static void ce_mob_file_block_object_object(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(NULL != mob_file->objects);
	ce_mob_object* object = ce_alloc_zero(sizeof(ce_mob_object));
	ce_vector_push_back(mob_file->objects, object);
	ce_mob_file_block_loop(mob_file, mem_file, size);
}

static void ce_mob_file_block_object_object_parts(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	if (NULL == object->parts) {
		uint32_t count = ce_mem_file_read_u32le(mem_file);
		object->parts = ce_vector_new_reserved(count);
		ce_mob_file_block_loop(mob_file, mem_file, size -= 4);
	} else {
		ce_string* part = ce_mob_file_read_string(mem_file, size);
		ce_vector_push_back(object->parts, part);
	}
}

static void ce_mob_file_block_object_object_owner(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(1 == size);
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	object->owner = ce_mem_file_read_u8(mem_file);
}

static void ce_mob_file_block_object_object_id(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(4 == size);
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	object->id = ce_mem_file_read_u32le(mem_file);
}

static void ce_mob_file_block_object_object_type(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(4 == size);
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	object->type = ce_mem_file_read_u32le(mem_file);
}

static void ce_mob_file_block_object_object_name(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	assert(NULL == object->name);
	object->name = ce_mob_file_read_string(mem_file, size);
}

static void ce_mob_file_block_object_object_model_name(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	assert(NULL == object->model_name);
	object->model_name = ce_mob_file_read_string(mem_file, size);
}

static void ce_mob_file_block_object_object_parent_name(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	assert(NULL == object->parent_name);
	object->parent_name = ce_mob_file_read_string(mem_file, size);
}

static void ce_mob_file_block_object_object_primary_texture(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	assert(NULL == object->primary_texture);
	object->primary_texture = ce_mob_file_read_string(mem_file, size);
}

static void ce_mob_file_block_object_object_secondary_texture(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	assert(NULL == object->secondary_texture);
	object->secondary_texture = ce_mob_file_read_string(mem_file, size);
}

static void ce_mob_file_block_object_object_comment(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	assert(NULL == object->comment);
	object->comment = ce_mob_file_read_string(mem_file, size);
}

static void ce_mob_file_block_object_object_position(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	float position[3];
	assert(sizeof(position) == size);
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	ce_mem_file_read(mem_file, position, sizeof(float), 3);
	ce_vec3_init_array(&object->position, position);
}

static void ce_mob_file_block_object_object_rotation(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	float rotation[4];
	assert(sizeof(rotation) == size);
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	ce_mem_file_read(mem_file, rotation, sizeof(float), 4);
	ce_quat_init_array(&object->rotation, rotation);
}

static void ce_mob_file_block_object_object_quest(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(1 == size);
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	object->quest = ce_mem_file_read_u8(mem_file);
}

static void ce_mob_file_block_object_object_shadow(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(1 == size);
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	object->shadow = ce_mem_file_read_u8(mem_file);
}

static void ce_mob_file_block_object_object_parent_id(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(4 == size);
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	object->parent_id = ce_mem_file_read_u32le(mem_file);
}

static void ce_mob_file_block_object_object_quest_info(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	assert(NULL == object->quest_info);
	object->quest_info = ce_mob_file_read_string(mem_file, size);
}

static void ce_mob_file_block_object_object_complection(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	float complection[3];
	assert(sizeof(complection) == size);
	assert(!ce_vector_empty(mob_file->objects));
	ce_mob_object* object = ce_vector_back(mob_file->objects);
	ce_mem_file_read(mem_file, complection, sizeof(float), 3);
	ce_complection_init_array(&object->complection, complection);
}

typedef void (*ce_mob_file_block_callback)(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size);

typedef struct {
	uint32_t type;
	ce_mob_file_block_callback callback;
} ce_mob_file_block_pair;

static const ce_mob_file_block_pair ce_mob_file_block_pairs[] = {
	{ 0xa000, ce_mob_file_block_main },
	{ 0xd000, ce_mob_file_block_quest },
	{ 0xc000, ce_mob_file_block_zonal },
	{ 0xacceeccb, ce_mob_file_block_text },
	{ 0xb000, ce_mob_file_block_object },
	{ 0xb001, ce_mob_file_block_object_object },
	{ 0xb00d, ce_mob_file_block_object_object_parts },
	{ 0xb011, ce_mob_file_block_object_object_owner },
	{ 0xb002, ce_mob_file_block_object_object_id },
	{ 0xb003, ce_mob_file_block_object_object_type },
	{ 0xb004, ce_mob_file_block_object_object_name },
	{ 0xb006, ce_mob_file_block_object_object_model_name },
	{ 0xb00e, ce_mob_file_block_object_object_parent_name },
	{ 0xb007, ce_mob_file_block_object_object_primary_texture },
	{ 0xb008, ce_mob_file_block_object_object_secondary_texture },
	{ 0xb00f, ce_mob_file_block_object_object_comment },
	{ 0xb009, ce_mob_file_block_object_object_position },
	{ 0xb00a, ce_mob_file_block_object_object_rotation },
	{ 0xb013, ce_mob_file_block_object_object_quest },
	{ 0xb014, ce_mob_file_block_object_object_shadow },
	{ 0xb012, ce_mob_file_block_object_object_parent_id },
	{ 0xb016, ce_mob_file_block_object_object_quest_info },
	{ 0xb00c, ce_mob_file_block_object_object_complection },
	{ 0xbbbb0000, ce_mob_file_block_object_object }
	//{ 0xbbbb0000, ce_mob_file_block_object_unit },
	//{ 0xbbac0000, ce_mob_file_block_object_lever },
	//{ 0xbbab0000, ce_mob_file_block_object_trap },
	//{ 0xbbbf, ce_mob_file_block_object_flame },
	//{ 0xaa01, ce_mob_file_block_object_particle1 },
	//{ 0xcc01, ce_mob_file_block_object_particle2 },
	//{ 0xdd01, ce_mob_file_block_object_particle3 },
};

static ce_mob_file_block_callback ce_mob_choose_callback(uint32_t type)
{
	for (size_t i = 0; i < sizeof(ce_mob_file_block_pairs) /
							sizeof(ce_mob_file_block_pairs[0]); ++i) {
		if (ce_mob_file_block_pairs[i].type == type) {
			return ce_mob_file_block_pairs[i].callback;
		}
	}
	return ce_mob_file_block_unknown;
}

static void ce_mob_file_block_loop(ce_mob_file* mob_file, ce_mem_file* mem_file, size_t size)
{
	while (0 != size) {
		uint32_t child_type = ce_mem_file_read_u32le(mem_file);
		uint32_t child_size = ce_mem_file_read_u32le(mem_file);

		size -= child_size;

		child_size -= 4 * 2;

#if 0
		// WARNING: graph data reversing
		if (0x31415926 == child_type) {
			int sz = child_size;
			uint32_t s1, s2;
			int16_t v;
			ce_mem_file_read(mem_file, &s1, sizeof(s1), 1);
			ce_mem_file_read(mem_file, &s2, sizeof(s2), 1);
			printf("%u %u\n", s1 / 8u, s2 / 8u);
			printf("%u %u\n", s1, s2);
			sz -= sizeof(s1) + sizeof(s2);
			printf("%d\n\n", sz);

			// 1: s1/8 matrices 8 x 8
			printf("---1---\n\n");
			for (unsigned int i = 0; i < s1 / 8; ++i) {
				for (unsigned int j = 0; j < 8; ++j) {
					for (unsigned int k = 0; k < 8; ++k) {
						ce_mem_file_read(mem_file, &v, sizeof(v), 1);
						sz -= sizeof(v);
						printf("%hd ", v);
					}
					printf("\n");
				}
				printf("\n");
			}
			printf("\n");

			// zonemenu 2x2   38912   sz, 127 loops, 304  b per loop, 48  b tail
			// bz8k     4x4   155648  sz, 255 loops, 608  b per loop, 96  b tail
			// zone2    4x6   233472  sz, 383 loops, 608  b per loop, 96  b tail
			// zone20   5x5   243200  sz, 319 loops, 760  b per loop, 120 b tail
			// zone6lmp 7x12  817152  sz, 767 loops, 1064 b per loop, 168 b tail
			// zone1    8x6   466944  sz, 383 loops, 1216 b per loop, 192 b tail
			// zone8    11x11 1177088 sz, 703 loops, 1672 b per loop, 264 b tail
			// zone7    15x10 1459200 sz, 639 loops, 2280 b per loop, 360 b tail
			// zone11   16x12 1867776 sz, 767 loops, 2432 b per loop, 384 b tail

			// 2: in loop
			for (unsigned int i = 0; i < 127; ++i) {
				printf("---2.1 loop %u---%d\n\n", i + 1, sz);
				// 2.1: 4*s1/8 numbers
				for (unsigned int j = 0; j < 4 * s1 / 8; ++j) {
					ce_mem_file_read(mem_file, &v, sizeof(v), 1);
					sz -= sizeof(v);
					printf("%hd ", v);
					if (0 == (j + 1) % 8) printf("\n");
				}

				// 2.2: 1 matrix s1/8 x 8
				printf("\n\n---2.2 loop %u---\n\n", i + 1);
				for (unsigned int j = 0; j < s1 / 8; ++j) {
					for (unsigned int k = 0; k < 8; ++k) {
						ce_mem_file_read(mem_file, &v, sizeof(v), 1);
						sz -= sizeof(v);
						printf("%hd ", v);
					}
					printf("\n");
				}

				// 2.3: 1 matrix s1 x 8
				printf("\n---2.3 loop %u---\n\n", i + 1);
				for (unsigned int j = 0; j < s1; ++j) {
					for (unsigned int k = 0; k < 8; ++k) {
						ce_mem_file_read(mem_file, &v, sizeof(v), 1);
						sz -= sizeof(v);
						printf("%hd ", v);
					}
					printf("\n");
				}
				printf("\n");
			}
			printf("---3---\n\n");

			// 3: s1/8 x 12 numbers
			for (unsigned int i = 0; i < s1 / 8; ++i) {
				for (unsigned int j = 0; j < 12; ++j) {
					ce_mem_file_read(mem_file, &v, sizeof(v), 1);
					sz -= sizeof(v);
					printf("%hd ", v);
				}
				printf("\n");
			}

			printf("\n%d\n", sz);
		}
#endif

		(*ce_mob_choose_callback(child_type))(mob_file, mem_file, child_size);
	}
}

static ce_mob_file* ce_mob_file_open_mem_file(const char* name, ce_mem_file* mem_file)
{
	ce_mob_file* mob_file = ce_alloc_zero(sizeof(ce_mob_file));
	mob_file->name = ce_string_new_str_n(name,
		ce_min(size_t, strlen(name), strlen(name) - 4));

	ce_mob_file_block_loop(mob_file, mem_file, ce_mem_file_size(mem_file));

	return mob_file;
}

ce_mob_file* ce_mob_file_open(const char* path)
{
	ce_mem_file* mem_file = ce_mem_file_new_path(path);
	if (NULL == mem_file) {
		return NULL;
	}

	const char* name = ce_strrpbrk(path, "\\/");
	if (NULL == name) {
		name = path;
	} else {
		++name;
	}

	ce_mob_file* mob_file = ce_mob_file_open_mem_file(name, mem_file);
	ce_mem_file_del(mem_file);

	return mob_file;
}

void ce_mob_file_close(ce_mob_file* mob_file)
{
	if (NULL != mob_file) {
		if (NULL != mob_file->objects) {
			for (size_t i = 0; i < mob_file->objects->count; ++i) {
				ce_mob_object* object = mob_file->objects->items[i];
				ce_vector_for_each(object->parts, ce_string_del);
				ce_string_del(object->quest_info);
				ce_string_del(object->comment);
				ce_string_del(object->secondary_texture);
				ce_string_del(object->primary_texture);
				ce_string_del(object->parent_name);
				ce_string_del(object->model_name);
				ce_string_del(object->name);
				ce_vector_del(object->parts);
				ce_free(object, sizeof(ce_mob_object));
			}
			ce_vector_del(mob_file->objects);
		}
		ce_string_del(mob_file->script);
		ce_string_del(mob_file->name);
		ce_free(mob_file, sizeof(ce_mob_file));
	}
}
