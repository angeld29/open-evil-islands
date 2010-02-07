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
#include <string.h>
#include <ctype.h>

#include "cestr.h"
#include "cebyteorder.h"
#include "celogging.h"
#include "cealloc.h"
#include "ceresfile.h"

enum {
	RES_SIGNATURE = 0x19ce23c
};

typedef struct {
	char* name;
	int32_t next_index;
	uint32_t data_length;
	uint32_t data_offset;
	int32_t modified;
	uint16_t name_length;
	uint32_t name_offset;
} ce_resfile_node;

struct ce_resfile {
	size_t name_length;
	char* name;
	uint32_t node_count;
	uint32_t metadata_offset;
	uint32_t names_length;
	char* names;
	ce_resfile_node* nodes;
	ce_memfile* mem;
};

static int name_hash(const char* name, int lim)
{
	int sum = 0;
	while (*name) {
		sum += tolower(*name++);
	}
	return sum % lim;
}

ce_resfile* ce_resfile_open_memfile(const char* name, ce_memfile* mem)
{
	ce_resfile* res = ce_alloc_zero(sizeof(ce_resfile));
	if (NULL == res) {
		ce_logging_error("resfile: could not allocate memory");
		return NULL;
	}

	res->name_length = strlen(name);
	res->name = ce_strdup(name);

	uint32_t signature;
	if (1 != ce_memfile_read(mem, &signature, sizeof(uint32_t), 1)) {
		ce_logging_error("resfile: io error occured");
		ce_resfile_close(res);
		return NULL;
	}

	ce_le2cpu32s(&signature);
	if (RES_SIGNATURE != signature) {
		ce_logging_error("resfile: wrong signature");
		ce_resfile_close(res);
		return NULL;
	}

	if (1 != ce_memfile_read(mem, &res->node_count, sizeof(uint32_t), 1) ||
			1 != ce_memfile_read(mem, &res->metadata_offset, sizeof(uint32_t), 1) ||
			1 != ce_memfile_read(mem, &res->names_length, sizeof(uint32_t), 1)) {
		ce_logging_error("resfile: io error occured");
		ce_resfile_close(res);
		return NULL;
	}

	ce_le2cpu32s(&res->node_count);
	ce_le2cpu32s(&res->metadata_offset);
	ce_le2cpu32s(&res->names_length);

	if (NULL == (res->nodes = ce_alloc_zero(sizeof(ce_resfile_node) *
												res->node_count))) {
		ce_logging_error("resfile: could not allocate memory");
		ce_resfile_close(res);
		return NULL;
	}

	if (0 != ce_memfile_seek(mem, res->metadata_offset, SEEK_SET)) {
		ce_logging_error("resfile: io error occured");
		ce_resfile_close(res);
		return NULL;
	}

	for (size_t i = 0; i < res->node_count; ++i) {
		ce_resfile_node* node = res->nodes + i;
		if (1 != ce_memfile_read(mem, &node->next_index, sizeof(int32_t), 1) ||
				1 != ce_memfile_read(mem, &node->data_length, sizeof(uint32_t), 1) ||
				1 != ce_memfile_read(mem, &node->data_offset, sizeof(uint32_t), 1) ||
				1 != ce_memfile_read(mem, &node->modified, sizeof(int32_t), 1) ||
				1 != ce_memfile_read(mem, &node->name_length, sizeof(uint16_t), 1) ||
				1 != ce_memfile_read(mem, &node->name_offset, sizeof(uint32_t), 1)) {
			ce_logging_error("resfile: io error occured");
			ce_resfile_close(res);
			return NULL;
		}
		ce_le2cpu32s((uint32_t*)&node->next_index);
		ce_le2cpu32s(&node->data_length);
		ce_le2cpu32s(&node->data_offset);
		ce_le2cpu32s((uint32_t*)&node->modified);
		ce_le2cpu16s(&node->name_length);
		ce_le2cpu32s(&node->name_offset);
	}

	if (NULL == (res->names = ce_alloc(res->names_length + 1))) {
		ce_logging_error("resfile: could not allocate memory");
		ce_resfile_close(res);
		return NULL;
	}

	if (1 != ce_memfile_read(mem, res->names, res->names_length, 1)) {
		ce_logging_error("resfile: io error occured");
		ce_resfile_close(res);
		return NULL;
	}
	res->names[res->names_length] = '\0';

	for (size_t i = 0; i < res->node_count; ++i) {
		ce_resfile_node* node = res->nodes + i;
		node->name = ce_strndup(res->names + node->name_offset, node->name_length);
	}

	res->mem = mem;

	return res;
}

ce_resfile* ce_resfile_open_file(const char* path)
{
	ce_memfile* mem = ce_memfile_open_path(path, "rb");
	if (NULL == mem) {
		return NULL;
	}

	const char* name = ce_strrpbrk(path, "\\/");
	if (NULL == name) {
		name = path;
	} else {
		++name;
	}

	ce_resfile* res = ce_resfile_open_memfile(name, mem);
	if (NULL == res) {
		ce_memfile_close(mem);
		return NULL;
	}

	return res;
}

void ce_resfile_close(ce_resfile* res)
{
	if (NULL == res) {
		return;
	}

	ce_memfile_close(res->mem);

	if (NULL != res->nodes) {
		for (size_t i = 0; i < res->node_count; ++i) {
			ce_resfile_node* node = res->nodes + i;
			ce_free(node->name, node->name_length + 1);
		}
	}

	ce_free(res->nodes, sizeof(ce_resfile_node) * res->node_count);
	ce_free(res->names, res->names_length + 1);
	ce_free(res->name, res->name_length + 1);

	ce_free(res, sizeof(ce_resfile));
}

const char* ce_resfile_name(const ce_resfile* res)
{
	return res->name;
}

int ce_resfile_node_count(const ce_resfile* res)
{
	return res->node_count;
}

int ce_resfile_node_index(const ce_resfile* res, const char* name)
{
	const ce_resfile_node* node;
	int index = name_hash(name, res->node_count);
	for (; index >= 0; index = node->next_index) {
		node = res->nodes + index;
		if (0 == ce_strcasecmp(name, node->name)) {
			break;
		}
	}
	return index;
}

const char* ce_resfile_node_name(const ce_resfile* res, int index)
{
	return res->nodes[index].name;
}

size_t ce_resfile_node_size(const ce_resfile* res, int index)
{
	return res->nodes[index].data_length;
}

time_t ce_resfile_node_modified(const ce_resfile* res, int index)
{
	return res->nodes[index].modified;
}

bool ce_resfile_node_data(ce_resfile* res, int index, void* data)
{
	ce_resfile_node* node = res->nodes + index;
	return 0 == ce_memfile_seek(res->mem, node->data_offset, SEEK_SET) &&
		1 == ce_memfile_read(res->mem, data, node->data_length, 1);
}
