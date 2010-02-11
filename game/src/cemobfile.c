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

static bool ce_mobfile_block_skip(ce_memfile* mem, size_t size)
{
	if (0 != ce_memfile_seek(mem, size, SEEK_CUR)) {
		ce_logging_error("mobfile: io error occured");
		return false;
	}

	return true;
}

static bool ce_mobfile_block_read_generic(ce_memfile* mem, void* data,
											size_t size, size_t count)
{
	if (count != ce_memfile_read(mem, data, size, count)) {
		ce_logging_error("mobfile: io error occured");
		return false;
	}

	return true;
}

static ce_string* ce_mobfile_block_read_string(ce_memfile* mem, size_t size)
{
	char data[size];
	if (size != ce_memfile_read(mem, data, 1, size)) {
		ce_logging_error("mobfile: io error occured");
		return NULL;
	}

	ce_string* str = ce_string_new();
	if (NULL != str) {
		ce_string_assign_n(str, data, size);
	}

	return str;
}

static bool
ce_mobfile_block_unknown(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	ce_unused(mob);
	return ce_mobfile_block_skip(mem, size);
}

static bool
ce_mobfile_block_main(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	ce_unused(mob);
	ce_unused(mem);
	ce_unused(size);
	return true;
}

static bool
ce_mobfile_block_main_quest(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	assert(0 == size);
	ce_unused(mob);
	ce_unused(mem);
	ce_unused(size);
	return true;
}

static bool
ce_mobfile_block_main_zonal(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	assert(0 == size);
	ce_unused(mob);
	ce_unused(mem);
	ce_unused(size);
	return true;
}

static bool
ce_mobfile_block_text(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	uint32_t key;
	if (1 != ce_memfile_read(mem, &key, sizeof(key), 1)) {
		ce_logging_error("mobfile: io error occured");
		return false;
	}

	ce_le2cpu32s(&key);

	size -= sizeof(key);

	char data[size];
	if (size != ce_memfile_read(mem, data, 1, size)) {
		ce_logging_error("mobfile: io error occured");
		return false;
	}

	for (int i = 0, n = size; i < n; ++i) {
		key += (((((key * 13) << 4) + key) << 8) - key) * 4 + 2531011;
		data[i] ^= key >> 16;
	}

	assert(NULL == mob->script);
	if (NULL == (mob->script = ce_string_new())) {
		return false;
	}

	ce_string_assign_n(mob->script, data, size);

	return true;
}

static bool
ce_mobfile_block_object(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	ce_unused(mem);
	ce_unused(size);

	assert(NULL == mob->objects);
	if (NULL == (mob->objects = ce_vector_new())) {
		return false;
	}

	return true;
}

static bool
ce_mobfile_block_object_object(ce_mobfile* mob, ce_memfile* mem, size_t size)
{
	ce_unused(mem);
	ce_unused(size);

	ce_mobobject_object* object = ce_alloc_zero(sizeof(ce_mobobject_object));
	if (NULL == object) {
		return false;
	}

	assert(NULL != mob->objects);
	ce_vector_push_back(mob->objects, object);

	return true;
}

static bool
ce_mobfile_block_object_object_parts(ce_mobfile* mob,
									ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	if (NULL == object->parts) {
		if (NULL == (object->parts = ce_vector_new())) {
			return false;
		}

		uint32_t count;
		if (1 != ce_memfile_read(mem, &count, sizeof(count), 1)) {
			ce_logging_error("mobfile: io error occured");
			return false;
		}

		ce_le2cpu32s(&count);
	} else {
		ce_string* part = ce_mobfile_block_read_string(mem, size);
		if (NULL == part) {
			return false;
		}

		ce_vector_push_back(object->parts, part);
		printf("block_object_object_parts: %s\n", ce_string_cstr(part));
	}

	return true;
}

static bool
ce_mobfile_block_object_object_owner(ce_mobfile* mob,
									ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(sizeof(object->owner) == size);
	return ce_mobfile_block_read_generic(mem, &object->owner,
											sizeof(object->owner), 1);
}

static bool
ce_mobfile_block_object_object_id(ce_mobfile* mob,
								ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(sizeof(object->id) == size);
	return ce_mobfile_block_read_generic(mem, &object->id,
											sizeof(object->id), 1);
}

static bool
ce_mobfile_block_object_object_type(ce_mobfile* mob,
									ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(sizeof(object->type) == size);
	return ce_mobfile_block_read_generic(mem, &object->type,
											sizeof(object->type), 1);
}

static bool
ce_mobfile_block_object_object_name(ce_mobfile* mob,
									ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	if (NULL == (object->name = ce_mobfile_block_read_string(mem, size))) {
		return false;
	}

	printf("block_object_object_name: %s\n", ce_string_cstr(object->name));
	return true;
}

static bool
ce_mobfile_block_object_object_model_name(ce_mobfile* mob,
											ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static bool
ce_mobfile_block_object_object_parent_name(ce_mobfile* mob,
											ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static bool
ce_mobfile_block_object_object_primary_texture(ce_mobfile* mob,
												ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static bool
ce_mobfile_block_object_object_secondary_texture(ce_mobfile* mob,
												ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static bool
ce_mobfile_block_object_object_comment(ce_mobfile* mob,
										ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static bool
ce_mobfile_block_object_object_position(ce_mobfile* mob,
										ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static bool
ce_mobfile_block_object_object_rotation(ce_mobfile* mob,
										ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static bool
ce_mobfile_block_object_object_quest(ce_mobfile* mob,
									ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static bool
ce_mobfile_block_object_object_shadow(ce_mobfile* mob,
										ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static bool
ce_mobfile_block_object_object_parent_id(ce_mobfile* mob,
										ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static bool
ce_mobfile_block_object_object_quest_info(ce_mobfile* mob,
											ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static bool
ce_mobfile_block_object_object_complection(ce_mobfile* mob,
											ce_memfile* mem, size_t size)
{
	assert(!ce_vector_empty(mob->objects));
	ce_mobobject_object* object = ce_vector_back(mob->objects);

	assert(false);
}

static const ce_mobfile_block_pair ce_mobfile_block_pairs[] = {
	{ 0xa000, ce_mobfile_block_main },
	{ 0xd000, ce_mobfile_block_main_quest },
	{ 0xc000, ce_mobfile_block_main_zonal },
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
		if (1 != ce_memfile_read(mem, &child_type, sizeof(child_type), 1) ||
				1 != ce_memfile_read(mem, &child_size, sizeof(child_size), 1)) {
			ce_logging_error("mobfile: io error occured");
			return false;
		}

		ce_le2cpu32s(&child_type);
		ce_le2cpu32s(&child_size);

		size -= child_size;

		child_size -= sizeof(child_type);
		child_size -= sizeof(child_size);

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
	ce_memfile* mem = ce_memfile_open_path(path, "rb");
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
