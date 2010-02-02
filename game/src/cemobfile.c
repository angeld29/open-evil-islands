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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "celib.h"
#include "cebyteorder.h"
#include "cestr.h"
#include "cemobfile.h"

enum {
	MOB_SIGNATURE = 0xa000
};

//static const uint64_t MAIN_BLOCK_QUEST_SIGNATURE = 0x80000d000;
//static const uint64_t MAIN_BLOCK_ZONAL_SIGNATURE = 0x80000c000;

//static const uint32_t TEXT_BLOCK_SIGNATURE = 0xacceeccb;
//static const uint64_t OBJECT_BLOCK_SIGNATURE = 0xb000;

/*static const uint64_t OBJECT_BLOCK_UNIT_SIGNATURE = 0xbbbb0000;
static const uint64_t OBJECT_BLOCK_OBJECT_SIGNATURE = 0xb001;
static const uint64_t OBJECT_BLOCK_LEVER_SIGNATURE = 0xbbac0000;
static const uint64_t OBJECT_BLOCK_TRAP_SIGNATURE = 0xbbab0000;
static const uint64_t OBJECT_BLOCK_FLAME_SIGNATURE = 0xbbbf;
static const uint64_t OBJECT_BLOCK_PARTICLE1_SIGNATURE = 0xaa01;
static const uint64_t OBJECT_BLOCK_PARTICLE2_SIGNATURE = 0xcc01;
static const uint64_t OBJECT_BLOCK_PARTICLE3_SIGNATURE = 0xdd01;*/

struct ce_mobfile {
	int stub;
};

/*static void decrypt_script(char* buf, uint32_t key, int32_t size)
{
	for (int i = 0; i < size; ++i) {
		key += (((((key * 13) << 4) + key) << 8) - key) * 4 + 2531011;
		buf[i] ^= key >> 16;
	}
}*/

/*static mobfile* open_callbacks(ce_io_callbacks callbacks,
								void* client_data, const char* name)
{
	mobfile* mob = calloc(1, sizeof(mobfile));
	if (NULL == mob) {
		return NULL;
	}

	uint32_t signature;
	if (1 != (callbacks.read)(&signature, sizeof(uint32_t), 1, client_data)) {
		mobfile_close(mob);
		return NULL;
	}

	ce_le2cpu32s(&signature);
	if (MOB_SIGNATURE != signature) {
		mobfile_close(mob);
		return NULL;
	}

	int32_t main_block_length;
	if (1 != (callbacks.read)(&main_block_length, sizeof(int32_t), 1, client_data)) {
		mobfile_close(mob);
		return NULL;
	}

	ce_le2cpu32s((uint32_t*)&main_block_length);
	printf("main_block_length: %d\n", main_block_length);

	uint64_t main_block_type;
	if (1 != (callbacks.read)(&main_block_type, sizeof(uint64_t), 1, client_data)) {
		mobfile_close(mob);
		return NULL;
	}

	ce_le2cpu64s(&main_block_type);
	printf("main_block_type: %#llx\n", main_block_type);
	if (MAIN_BLOCK_QUEST_SIGNATURE != main_block_type &&
			MAIN_BLOCK_ZONAL_SIGNATURE != main_block_type) {
		mobfile_close(mob);
		return NULL;
	}

	do {
		uint32_t signature;
		if (1 != (callbacks.read)(&signature, sizeof(uint32_t), 1, client_data)) {
			mobfile_close(mob);
			return NULL;
		}

		ce_le2cpu32s(&signature);
		printf("signature: %u %#x\n", signature, signature);

		int32_t length;
		if (1 != (callbacks.read)(&length, sizeof(int32_t), 1, client_data)) {
			mobfile_close(mob);
			return NULL;
		}

		ce_le2cpu32s((uint32_t*)&length);
		printf("length: %d\n", length);

		if (TEXT_BLOCK_SIGNATURE == signature) {
			int32_t script_key;
			(callbacks.read)(&script_key, sizeof(int32_t), 1, client_data);

			ce_le2cpu32s((uint32_t*)&script_key);
			printf("script_key: %d\n", script_key);

			char* data = malloc(length - 4 - 4 - 4);
			(callbacks.read)(data, 1, length - 4 - 4 - 4, client_data);
			decrypt_script(data, script_key, length - 4 - 4 - 4);
			for (int i = 0; i < length - 4 - 4 - 4; ++i) {
				printf("%c", data[i]);
			}
			printf("\n");
			free(data);
		} else {
			char* data = malloc(length - 4 - 4);
			(callbacks.read)(data, 1, length - 4 - 4, client_data);
			free(data);
		}
	} while(main_block_length != (callbacks.tell)(client_data));

	mob->callbacks = callbacks;
	mob->client_data = client_data;

	return mob;
}*/

ce_mobfile* ce_mobfile_open(const char* path)
{
	ce_unused(path);
	assert(false && "Not implemented");
	return NULL;

	/*FILE* file = fopen(path, "rb");
	if (NULL == file) {
		return NULL;
	}

	const char* name = ce_strrpbrk(path, "\\/");
	if (NULL == name) {
		name = path;
	} else {
		++name;
	}

	mobfile* mob = open_callbacks(CE_IO_CALLBACKS_FILE, file, name);
	if (NULL == mob) {
		fclose(file);
		return NULL;
	}

	return mob;*/
}

void ce_mobfile_close(ce_mobfile* mob)
{
	if (NULL == mob) {
		return;
	}

	//free(mob);
}
