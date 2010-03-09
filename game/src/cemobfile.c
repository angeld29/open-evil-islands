/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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
#include <stdint.h>
#include <assert.h>

#include "celib.h"
#include "cebyteorder.h"
#include "celogging.h"
#include "cealloc.h"
#include "cememfile.h"
#include "cemobfile.h"

typedef bool
(*ce_mobfile_block_callback)(ce_mobfile* mob, ce_memfile* mem, size_t size);

typedef struct {
	unsigned int type;
	ce_mobfile_block_callback callback;
} ce_mobfile_block_pair;

// Helpers.

static bool ce_mobfile_decrypt_script(char* data, size_t size, uint32_t key)
{
	for (int i = 0, n = size; i < n; ++i) {
		key += (((((key * 13) << 4) + key) << 8) - key) * 4 + 2531011;
		data[i] ^= key >> 16;
	}
	return true;
}

static bool ce_mobfile_skip(ce_memfile* mem, size_t size)
{
	if (0 != ce_memfile_seek(mem, size, SEEK_CUR)) {
		ce_logging_error("mobfile: io error occured");
		return false;
	}
	return true;
}

static bool ce_mobfile_read_generic(ce_memfile* mem, void* data,
										size_t size, size_t count)
{
	if (count != ce_memfile_read(mem, data, size, count)) {
		ce_logging_error("mobfile: io error occured");
		return false;
	}
	return true;
}

static bool ce_mobfile_read_uint8(ce_memfile* mem, uint8_t* value)
{
	return ce_mobfile_read_generic(mem, value, sizeof(uint8_t), 1);
}

static bool ce_mobfile_read_uint32(ce_memfile* mem, uint32_t* value)
{
	return ce_mobfile_read_generic(mem, value, sizeof(uint32_t), 1) &&
		(ce_le2cpu32s(value), true);
}

static ce_string* ce_mobfile_read_string(ce_memfile* mem, size_t size)
{
	char data[size];
	return ce_mobfile_read_generic(mem, data, 1, size) ?
		ce_string_new_cstr_n(data, size) : NULL;
}

// Callbacks.

static bool ce_mobfile_block_loop(ce_mobfile* mob, ce_memfile* mem, size_t size);

static bool
ce_mobfile_block_unknown(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	ce_unused(mob);
	return ce_mobfile_skip(mem, size);
}

static bool
ce_mobfile_block_main(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	return ce_mobfile_block_loop(mob, mem, size);
}

static bool
ce_mobfile_block_quest(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	assert(0 == size);
	return ce_mobfile_block_loop(mob, mem, size);
}

static bool
ce_mobfile_block_zonal(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	assert(0 == size);
	return ce_mobfile_block_loop(mob, mem, size);
}

static bool
ce_mobfile_block_text(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	// Do not load string directly through read_string because
	// string is encrypted and may contain null characters.
	uint32_t key;
	char data[size -= sizeof(key)];
	assert(NULL == mob->script);
	return ce_mobfile_read_uint32(mem, &key) &&
		ce_mobfile_read_generic(mem, data, 1, size) &&
		ce_mobfile_decrypt_script(data, size, key) &&
		NULL != (mob->script = ce_string_new_cstr_n(data, size));
}

static bool
ce_mobfile_block_object(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	assert(NULL == mob->objects);
	return NULL != (mob->objects = ce_vector_new()) &&
		ce_mobfile_block_loop(mob, mem, size);
}

static bool
ce_mobfile_block_object_object(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	assert(NULL != mob->objects);
	ce_mobobject_object* object = ce_alloc_zero(sizeof(ce_mobobject_object));
	return NULL != object && ce_vector_push_back(mob->objects, object) &&
		ce_mobfile_block_loop(mob, mem, size);
}

static bool
ce_mobfile_block_object_object_parts(ce_mobfile* mob,
									ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	if (NULL == object->parts) {
		uint32_t count;
		return ce_mobfile_read_uint32(mem, &count) &&
			NULL != (object->parts = ce_vector_new_reserved(count)) &&
			ce_mobfile_block_loop(mob, mem, size -= sizeof(count));
	}

	ce_string* part = ce_mobfile_read_string(mem, size);
	return NULL != part && ce_vector_push_back(object->parts, part);
}

static bool
ce_mobfile_block_object_object_owner(ce_mobfile* mob,
									ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	uint8_t owner;

	assert(sizeof(owner) == size);
	return ce_mobfile_read_uint8(mem, &owner) && (object->owner = owner, true);
}

static bool
ce_mobfile_block_object_object_id(ce_mobfile* mob,
								ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	uint32_t id;

	assert(sizeof(id) == size);
	return ce_mobfile_read_uint32(mem, &id) && (object->id = id, true);
}

static bool
ce_mobfile_block_object_object_type(ce_mobfile* mob,
									ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	uint32_t type;

	assert(sizeof(type) == size);
	return ce_mobfile_read_uint32(mem, &type) && (object->type = type, true);
}

static bool
ce_mobfile_block_object_object_name(ce_mobfile* mob,
									ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(NULL == object->name);
	return NULL != (object->name = ce_mobfile_read_string(mem, size));
}

static bool
ce_mobfile_block_object_object_model_name(ce_mobfile* mob,
											ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(NULL == object->model_name);
	return NULL != (object->model_name = ce_mobfile_read_string(mem, size));
}

static bool
ce_mobfile_block_object_object_parent_name(ce_mobfile* mob,
											ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(NULL == object->parent_name);
	return NULL != (object->parent_name = ce_mobfile_read_string(mem, size));
}

static bool
ce_mobfile_block_object_object_primary_texture(ce_mobfile* mob,
												ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(NULL == object->primary_texture);
	return NULL != (object->primary_texture = ce_mobfile_read_string(mem, size));
}

static bool
ce_mobfile_block_object_object_secondary_texture(ce_mobfile* mob,
												ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(NULL == object->secondary_texture);
	return NULL != (object->secondary_texture = ce_mobfile_read_string(mem, size));
}

static bool
ce_mobfile_block_object_object_comment(ce_mobfile* mob,
										ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(NULL == object->comment);
	return NULL != (object->comment = ce_mobfile_read_string(mem, size));
}

static bool
ce_mobfile_block_object_object_position(ce_mobfile* mob,
										ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	float position[3];

	assert(sizeof(position) == size);
	return ce_mobfile_read_generic(mem, position, sizeof(position), 1) &&
		ce_vec3_init_array(&object->position, position);
}

static bool
ce_mobfile_block_object_object_rotation(ce_mobfile* mob,
										ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	float rotation[4];

	assert(sizeof(rotation) == size);
	return ce_mobfile_read_generic(mem, rotation, sizeof(rotation), 1) &&
		ce_quat_init_array(&object->rotation, rotation);
}

static bool
ce_mobfile_block_object_object_quest(ce_mobfile* mob,
									ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	uint8_t quest;

	assert(sizeof(quest) == size);
	return ce_mobfile_read_uint8(mem, &quest) && (object->quest = quest, true);
}

static bool
ce_mobfile_block_object_object_shadow(ce_mobfile* mob,
										ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	uint8_t shadow;

	assert(sizeof(shadow) == size);
	return ce_mobfile_read_uint8(mem, &shadow) &&
			(object->shadow = shadow, true);
}

static bool
ce_mobfile_block_object_object_parent_id(ce_mobfile* mob,
										ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	uint32_t parent_id;

	assert(sizeof(parent_id) == size);
	return ce_mobfile_read_uint32(mem, &parent_id) &&
			(object->parent_id = parent_id, true);
}

static bool
ce_mobfile_block_object_object_quest_info(ce_mobfile* mob,
											ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(NULL == object->quest_info);
	return NULL != (object->quest_info = ce_mobfile_read_string(mem, size));
}

static bool
ce_mobfile_block_object_object_complection(ce_mobfile* mob,
											ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	float complection[3];

	assert(sizeof(complection) == size);
	return ce_mobfile_read_generic(mem, complection, sizeof(complection), 1) &&
		ce_complection_init_array(&object->complection, complection);
}

static const ce_mobfile_block_pair ce_mobfile_block_pairs[] = {
	{ 0xa000, ce_mobfile_block_main },
	{ 0xd000, ce_mobfile_block_quest },
	{ 0xc000, ce_mobfile_block_zonal },
	{ 0xacceeccb, ce_mobfile_block_text },
	{ 0xb000, ce_mobfile_block_object },
	//{ 0xbbbb0000, ce_mobfile_block_object_unit },
	{ 0xb001, ce_mobfile_block_object_object },
	{ 0xb00d, ce_mobfile_block_object_object_parts },
	{ 0xb011, ce_mobfile_block_object_object_owner },
	{ 0xb002, ce_mobfile_block_object_object_id },
	{ 0xb003, ce_mobfile_block_object_object_type },
	{ 0xb004, ce_mobfile_block_object_object_name },
	{ 0xb006, ce_mobfile_block_object_object_model_name },
	{ 0xb00e, ce_mobfile_block_object_object_parent_name },
	{ 0xb007, ce_mobfile_block_object_object_primary_texture },
	{ 0xb008, ce_mobfile_block_object_object_secondary_texture },
	{ 0xb00f, ce_mobfile_block_object_object_comment },
	{ 0xb009, ce_mobfile_block_object_object_position },
	{ 0xb00a, ce_mobfile_block_object_object_rotation },
	{ 0xb013, ce_mobfile_block_object_object_quest },
	{ 0xb014, ce_mobfile_block_object_object_shadow },
	{ 0xb012, ce_mobfile_block_object_object_parent_id },
	{ 0xb016, ce_mobfile_block_object_object_quest_info },
	{ 0xb00c, ce_mobfile_block_object_object_complection }
	//{ 0xbbac0000, ce_mobfile_block_object_lever },
	//{ 0xbbab0000, ce_mobfile_block_object_trap },
	//{ 0xbbbf, ce_mobfile_block_object_flame },
	//{ 0xaa01, ce_mobfile_block_object_particle1 },
	//{ 0xcc01, ce_mobfile_block_object_particle2 },
	//{ 0xdd01, ce_mobfile_block_object_particle3 },
};

static ce_mobfile_block_callback
ce_mobfile_block_callback_choose(unsigned int type)
{
	for (int i = 0, n = sizeof(ce_mobfile_block_pairs) /
						sizeof(ce_mobfile_block_pairs[0]); i < n; ++i) {
		if (ce_mobfile_block_pairs[i].type == type) {
			return ce_mobfile_block_pairs[i].callback;
		}
	}
	return ce_mobfile_block_unknown;
}

static bool ce_mobfile_block_loop(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	uint32_t child_type, child_size;

	while (0 != size) {
		if (!ce_mobfile_read_uint32(mem, &child_type) ||
				!ce_mobfile_read_uint32(mem, &child_size)) {
			return false;
		}

		size -= child_size;

		child_size -= sizeof(child_type);
		child_size -= sizeof(child_size);

#if 0
		// WARNING: graph data reversing
		if (0x31415926 == child_type) {
			int sz = child_size;
			uint32_t s1, s2;
			int16_t v;
			ce_memfile_read(mem, &s1, sizeof(s1), 1);
			ce_memfile_read(mem, &s2, sizeof(s2), 1);
			printf("%u %u\n", s1 / 8u, s2 / 8u);
			printf("%u %u\n", s1, s2);
			sz -= sizeof(s1) + sizeof(s2);
			printf("%d\n\n", sz);

			// 1: s1/8 matrices 8 x 8
			printf("---1---\n\n");
			for (unsigned int i = 0; i < s1 / 8; ++i) {
				for (unsigned int j = 0; j < 8; ++j) {
					for (unsigned int k = 0; k < 8; ++k) {
						ce_memfile_read(mem, &v, sizeof(v), 1);
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
					ce_memfile_read(mem, &v, sizeof(v), 1);
					sz -= sizeof(v);
					printf("%hd ", v);
					if (0 == (j + 1) % 8) printf("\n");
				}

				// 2.2: 1 matrix s1/8 x 8
				printf("\n\n---2.2 loop %u---\n\n", i + 1);
				for (unsigned int j = 0; j < s1 / 8; ++j) {
					for (unsigned int k = 0; k < 8; ++k) {
						ce_memfile_read(mem, &v, sizeof(v), 1);
						sz -= sizeof(v);
						printf("%hd ", v);
					}
					printf("\n");
				}

				// 2.3: 1 matrix s1 x 8
				printf("\n---2.3 loop %u---\n\n", i + 1);
				for (unsigned int j = 0; j < s1; ++j) {
					for (unsigned int k = 0; k < 8; ++k) {
						ce_memfile_read(mem, &v, sizeof(v), 1);
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
					ce_memfile_read(mem, &v, sizeof(v), 1);
					sz -= sizeof(v);
					printf("%hd ", v);
				}
				printf("\n");
			}

			printf("\n%d\n", sz);
		}
#endif

		ce_mobfile_block_callback callback =
			ce_mobfile_block_callback_choose(child_type);

		if (!callback(mob, mem, child_size)) {
			return false;
		}
	}

	return true;
}

static ce_mobfile* ce_mobfile_open_memfile(ce_memfile* mem)
{
	ce_mobfile* mob = ce_alloc_zero(sizeof(ce_mobfile));
	if (NULL == mob) {
		ce_logging_error("mobfile: could not allocate memory");
		return NULL;
	}

	ce_memfile_seek(mem, 0, SEEK_END);
	size_t size = ce_memfile_tell(mem);
	ce_memfile_seek(mem, 0, SEEK_SET);

	if (!ce_mobfile_block_loop(mob, mem, size)) {
		ce_mobfile_close(mob);
		return NULL;
	}

	return mob;
}

ce_mobfile* ce_mobfile_open(const char* path)
{
	ce_memfile* mem = ce_memfile_open_file(path, "rb");
	if (NULL == mem) {
		return NULL;
	}

	ce_mobfile* mob = ce_mobfile_open_memfile(mem);
	ce_memfile_close(mem);

	return mob;
}

void ce_mobfile_close(ce_mobfile* mob)
{
	if (NULL != mob) {
		if (NULL != mob->objects) {
			for (int i = 0, n = ce_vector_count(mob->objects); i < n; ++i) {
				ce_mobobject_object* object = ce_vector_at(mob->objects, i);
				ce_string_del(object->quest_info);
				ce_string_del(object->comment);
				ce_string_del(object->secondary_texture);
				ce_string_del(object->primary_texture);
				ce_string_del(object->parent_name);
				ce_string_del(object->model_name);
				ce_string_del(object->name);
				if (NULL != object->parts) {
					for (int j = 0,
							m = ce_vector_count(object->parts); j < m; ++j) {
						ce_string_del(ce_vector_at(object->parts, j));
					}
					ce_vector_del(object->parts);
				}
				ce_free(object, sizeof(ce_mobobject_object));
			}
			ce_vector_del(mob->objects);
		}
		ce_string_del(mob->script);
		ce_free(mob, sizeof(ce_mobfile));
	}
}
