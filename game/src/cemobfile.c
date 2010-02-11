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
#include <stdbool.h>
#include <assert.h>

#include "celib.h"
#include "cebyteorder.h"
#include "celogging.h"
#include "cealloc.h"
#include "cememfile.h"
#include "cemobfile.h"

static const uint32_t MOB_SIGNATURE = 0xa000;

static const uint64_t MAIN_BLOCK_QUEST = 0x80000d000;
static const uint64_t MAIN_BLOCK_ZONAL = 0x80000c000;

static const uint32_t BLOCK_TEXT = 0xacceeccb;
static const uint32_t BLOCK_OBJECT = 0xb000;

static const uint32_t BLOCK_OBJECT_UNIT = 0xbbbb0000;
static const uint32_t BLOCK_OBJECT_OBJECT = 0xb001;
static const uint32_t BLOCK_OBJECT_LEVER = 0xbbac0000;
static const uint32_t BLOCK_OBJECT_TRAP = 0xbbab0000;
static const uint32_t BLOCK_OBJECT_FLAME = 0xbbbf;
static const uint32_t BLOCK_OBJECT_PARTICLE1 = 0xaa01;
static const uint32_t BLOCK_OBJECT_PARTICLE2 = 0xcc01;
static const uint32_t BLOCK_OBJECT_PARTICLE3 = 0xdd01;

static const uint32_t BLOCK_OBJECT_OBJECT_PARTS = 0xb00d;
static const uint32_t BLOCK_OBJECT_OBJECT_OWNER = 0xb011;
static const uint32_t BLOCK_OBJECT_OBJECT_ID = 0xb002;
static const uint32_t BLOCK_OBJECT_OBJECT_TYPE = 0xb003;
static const uint32_t BLOCK_OBJECT_OBJECT_NAME = 0xb004;
static const uint32_t BLOCK_OBJECT_OBJECT_MODEL_NAME = 0xb006;
static const uint32_t BLOCK_OBJECT_OBJECT_PARENT_NAME = 0xb00e;
static const uint32_t BLOCK_OBJECT_OBJECT_PRIMARY_TEXTURE = 0xb007;
static const uint32_t BLOCK_OBJECT_OBJECT_SECONDARY_TEXTURE = 0xb008;
static const uint32_t BLOCK_OBJECT_OBJECT_COMMENT = 0xb00f;
static const uint32_t BLOCK_OBJECT_OBJECT_POSITION = 0xb009;
static const uint32_t BLOCK_OBJECT_OBJECT_ROTATION = 0xb00a;
static const uint32_t BLOCK_OBJECT_OBJECT_QUEST = 0xb013;
static const uint32_t BLOCK_OBJECT_OBJECT_SHADOW = 0xb014;
static const uint32_t BLOCK_OBJECT_OBJECT_PARENT_ID = 0xb012;
static const uint32_t BLOCK_OBJECT_OBJECT_QUEST_INFO = 0xb016;
static const uint32_t BLOCK_OBJECT_OBJECT_COMPLECTION = 0xb00c;

typedef bool (*ce_mobfile_read_block)(ce_mobfile*, int, ce_memfile*);

static void ce_mobfile_decrypt_script(char* str, int length, uint32_t key)
{
	for (int i = 0; i < length; ++i) {
		key += (((((key * 13) << 4) + key) << 8) - key) * 4 + 2531011;
		str[i] ^= key >> 16;
	}
}

static bool ce_mobfile_read_block_unknown(ce_mobfile* mob,
											int block_length, ce_memfile* mem)
{
	ce_unused(mob);
	if (0 != ce_memfile_seek(mem, block_length, SEEK_CUR)) {
		ce_logging_error("mobfile: io error occured");
		return false;
	}
	return true;
}

static bool ce_mobfile_read_block_text(ce_mobfile* mob,
											int block_length, ce_memfile* mem)
{
	uint32_t key;
	if (1 != ce_memfile_read(mem, &key, sizeof(key), 1)) {
		ce_logging_error("mobfile: io error occured");
		return false;
	}

	ce_le2cpu32s(&key);
	block_length -= sizeof(key);

	char script[block_length];
	if (block_length != (int)ce_memfile_read(mem, script, 1, block_length)) {
		ce_logging_error("mobfile: io error occured");
		return false;
	}

	ce_mobfile_decrypt_script(script, block_length, key);
	ce_string_assign_n(mob->script, script, block_length);

	return true;
}

static bool ce_mobfile_read_block_object_object(ce_mobfile* mob,
											int block_length, ce_memfile* mem)
{
	uint32_t type, length;

	ce_mobobject_object* object = ce_alloc_zero(sizeof(ce_mobobject_object));
	if (NULL == object) {
		return false;
	}

	ce_vector_push_back(mob->objects, object);

	while (0 != block_length) {
		if (1 != ce_memfile_read(mem, &type, sizeof(type), 1) ||
				1 != ce_memfile_read(mem, &length, sizeof(length), 1)) {
			ce_logging_error("mobfile: io error occured");
			return false;
		}

		ce_le2cpu32s(&type);
		ce_le2cpu32s(&length);

		block_length -= length;
		length -= sizeof(type);
		length -= sizeof(length);

		bool ok;

		if (BLOCK_OBJECT_OBJECT_PARTS == type) {
			assert(NULL == object->parts);
			if (NULL == (object->parts = ce_vector_new())) {
				return false;
			}

			uint32_t count;
			if (1 != ce_memfile_read(mem, &count, sizeof(count), 1)) {
				ce_logging_error("mobfile: io error occured");
				return false;
			}

			ce_le2cpu32s(&count);

			for (unsigned int i = 0; i < count; ++i) {
				if (1 != ce_memfile_read(mem, &type, sizeof(type), 1) ||
						1 != ce_memfile_read(mem, &length, sizeof(length), 1)) {
					ce_logging_error("mobfile: io error occured");
					return false;
				}

				ce_le2cpu32s(&type);
				ce_le2cpu32s(&length);

				assert(BLOCK_OBJECT_OBJECT_PARTS == type);

				length -= sizeof(type);
				length -= sizeof(length);

				char cpart[length];
				if (length != ce_memfile_read(mem, cpart, 1, length)) {
					ce_logging_error("mobfile: io error occured");
					return false;
				}

				ce_string* part = ce_string_new();
				if (NULL == part) {
					return false;
				}

				ce_vector_push_back(object->parts, part);
				ce_string_assign_n(part, cpart, length);

				//printf("name: %s\n", ce_string_cstr(part));
			}

			ok = true;
		} else if (BLOCK_OBJECT_OBJECT_OWNER == type) {
			assert(sizeof(object->owner) == length);
			ok = 1 == ce_memfile_read(mem, &object->owner, length, 1);
			//printf("owner: %hhu\n", object->owner);
		} else if (BLOCK_OBJECT_OBJECT_ID == type) {
			assert(sizeof(object->id) == length);
			ok = 1 == ce_memfile_read(mem, &object->id, length, 1);
			//printf("id: %u\n", object->id);
		} else if (BLOCK_OBJECT_OBJECT_TYPE == type) {
			assert(sizeof(object->type) == length);
			ok = 1 == ce_memfile_read(mem, &object->type, length, 1);
			//printf("type: %u\n", object->type);
		} else if (BLOCK_OBJECT_OBJECT_NAME == type) {
			//printf("name: %u\n", length);
			char data[length];
			if (length != ce_memfile_read(mem, data, 1, length)) {
				ce_logging_error("mobfile: io error occured");
				return false;
			}

			if (NULL == (object->name = ce_string_new())) {
				return false;
			}

			ce_string_assign_n(object->name, data, length);
			//printf("name: %s\n", ce_string_cstr(object->name));

			ok = true;
		} else if (BLOCK_OBJECT_OBJECT_MODEL_NAME == type) {
			//printf("model name: %u\n", length);
			ok = 0 == ce_memfile_seek(mem, length, SEEK_CUR);
		} else if (BLOCK_OBJECT_OBJECT_PARENT_NAME == type) {
			//printf("parent name: %u\n", length);
			ok = 0 == ce_memfile_seek(mem, length, SEEK_CUR);
		} else if (BLOCK_OBJECT_OBJECT_PRIMARY_TEXTURE == type) {
			//printf("pr tex: %u\n", length);
			char data[length];
			if (length != ce_memfile_read(mem, data, 1, length)) {
				ce_logging_error("mobfile: io error occured");
				return false;
			}

			if (NULL == (object->primary_texture = ce_string_new())) {
				return false;
			}

			ce_string_assign_n(object->primary_texture, data, length);
			//printf("pri tex: %s\n", ce_string_cstr(object->primary_texture));

			ok = true;
		} else if (BLOCK_OBJECT_OBJECT_SECONDARY_TEXTURE == type) {
			//printf("sec tex: %u\n", length);
			ok = 0 == ce_memfile_seek(mem, length, SEEK_CUR);
		} else if (BLOCK_OBJECT_OBJECT_COMMENT == type) {
			//printf("comment: %u\n", length);
			ok = 0 == ce_memfile_seek(mem, length, SEEK_CUR);
		} else if (BLOCK_OBJECT_OBJECT_POSITION == type) {
			assert(3 * sizeof(float) == length);
			ok = 1 == ce_memfile_read(mem,
						&object->position.x, sizeof(float), 1)
				&& 1 == ce_memfile_read(mem,
						&object->position.y, sizeof(float), 1)
				&& 1 == ce_memfile_read(mem,
						&object->position.z, sizeof(float), 1);
			//printf("pos: %f, %f, %f\n", object->position.x,
			//	object->position.y, object->position.z);
		} else if (BLOCK_OBJECT_OBJECT_ROTATION == type) {
			assert(4 * sizeof(float) == length);
			ok = 1 == ce_memfile_read(mem,
						&object->rotation.w, sizeof(float), 1)
				&& 1 == ce_memfile_read(mem,
						&object->rotation.x, sizeof(float), 1)
				&& 1 == ce_memfile_read(mem,
						&object->rotation.y, sizeof(float), 1)
				&& 1 == ce_memfile_read(mem,
						&object->rotation.z, sizeof(float), 1);
			//printf("rot: %f, %f, %f, %f\n", object->rotation.w,
			//	object->rotation.x, object->rotation.y, object->rotation.z);
		} else if (BLOCK_OBJECT_OBJECT_QUEST == type) {
			assert(sizeof(object->quest) == length);
			ok = 1 == ce_memfile_read(mem, &object->quest, length, 1);
			//printf("quest: %hhu\n", object->quest);
		} else if (BLOCK_OBJECT_OBJECT_SHADOW == type) {
			assert(sizeof(object->shadow) == length);
			ok = 1 == ce_memfile_read(mem, &object->shadow, length, 1);
			//printf("shadow: %hhu\n", object->shadow);
		} else if (BLOCK_OBJECT_OBJECT_PARENT_ID == type) {
			assert(sizeof(object->parent_id) == length);
			ok = 1 == ce_memfile_read(mem, &object->parent_id, length, 1);
			//printf("shadow: %u\n", object->parent_id);
		} else if (BLOCK_OBJECT_OBJECT_QUEST_INFO == type) {
			//printf("quest info: %u\n", length);
			ok = 0 == ce_memfile_seek(mem, length, SEEK_CUR);
		} else if (BLOCK_OBJECT_OBJECT_COMPLECTION == type) {
			assert(3 * sizeof(float) == length);
			ok = 1 == ce_memfile_read(mem,
						&object->strength, sizeof(float), 1)
				&& 1 == ce_memfile_read(mem,
						&object->dexterity, sizeof(float), 1)
				&& 1 == ce_memfile_read(mem,
						&object->tallness, sizeof(float), 1);
			//printf("comp: %f, %f, %f\n", object->strength,
			//	object->dexterity, object->tallness);
		} else {
			assert(false);
			ok = false;
		}

		if (!ok) {
			ce_logging_error("mobfile: io error occured");
			return false;
		}
	}

	return true;
}

static bool ce_mobfile_read_block_object(ce_mobfile* mob,
											int block_length, ce_memfile* mem)
{
	uint32_t type, length;

	while (0 != block_length) {
		if (1 != ce_memfile_read(mem, &type, sizeof(type), 1) ||
				1 != ce_memfile_read(mem, &length, sizeof(length), 1)) {
			ce_logging_error("mobfile: io error occured");
			return false;
		}

		ce_le2cpu32s(&type);
		ce_le2cpu32s(&length);

		block_length -= length;
		length -= sizeof(type);
		length -= sizeof(length);

		ce_mobfile_read_block read_block;

		if (BLOCK_OBJECT_OBJECT == type) {
			read_block = ce_mobfile_read_block_object_object;
		} else {
			read_block = ce_mobfile_read_block_unknown;
		}

		if (!read_block(mob, length, mem)) {
			return false;
		}
	}

	return true;
}

static bool ce_mobfile_read_block_main(ce_mobfile* mob,
											int block_length, ce_memfile* mem)
{
	uint32_t type, length;

	while (block_length != ce_memfile_tell(mem)) {
		if (1 != ce_memfile_read(mem, &type, sizeof(type), 1) ||
				1 != ce_memfile_read(mem, &length, sizeof(length), 1)) {
			ce_logging_error("mobfile: io error occured");
			return false;
		}

		ce_le2cpu32s(&type);
		ce_le2cpu32s(&length);

		length -= sizeof(type);
		length -= sizeof(length);

		ce_mobfile_read_block read_block;

		if (BLOCK_TEXT == type) {
			read_block = ce_mobfile_read_block_text;
		} else if (BLOCK_OBJECT == type) {
			read_block = ce_mobfile_read_block_object;
		} else {
			read_block = ce_mobfile_read_block_unknown;
		}

		if (!read_block(mob, length, mem)) {
			return false;
		}
	}

	return true;
}

static bool ce_mobfile_open_memfile_impl(ce_mobfile* mob, ce_memfile* mem)
{
	if (NULL == (mob->script = ce_string_new()) ||
			NULL == (mob->objects = ce_vector_new())) {
		return false;
	}

	uint32_t signature;
	if (1 != ce_memfile_read(mem, &signature, sizeof(signature), 1)) {
		ce_logging_error("mobfile: io error occured");
		return false;
	}

	ce_le2cpu32s(&signature);
	if (MOB_SIGNATURE != signature) {
		ce_logging_error("mobfile: wrong signature");
		return false;
	}

	uint32_t main_block_length;
	uint64_t main_block_type;

	if (1 != ce_memfile_read(mem, &main_block_length,
							sizeof(main_block_length), 1) ||
			1 != ce_memfile_read(mem, &main_block_type,
								sizeof(main_block_type), 1)) {
		ce_logging_error("mobfile: io error occured");
		return false;
	}

	ce_le2cpu32s(&main_block_length);
	ce_le2cpu64s(&main_block_type);

	if (MAIN_BLOCK_QUEST != main_block_type &&
			MAIN_BLOCK_ZONAL != main_block_type) {
		ce_logging_error("mobfile: wrong main block type");
		return false;
	}

	return ce_mobfile_read_block_main(mob, main_block_length, mem);
}

static ce_mobfile* ce_mobfile_open_memfile(ce_memfile* mem)
{
	ce_mobfile* mob = ce_alloc_zero(sizeof(ce_mobfile));
	if (NULL == mob) {
		ce_logging_error("mobfile: could not allocate memory");
		return NULL;
	}

	if (!ce_mobfile_open_memfile_impl(mob, mem)) {
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
