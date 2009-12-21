#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "cestr.h"
#include "byteorder.h"
#include "memfile.h"
#include "resfile.h"

enum {
	RES_SIGNATURE = 0x19ce23c
};

typedef struct {
	char* name;
	int32_t next_index;
	int32_t data_length;
	int32_t data_offset;
	int32_t modified;
	int16_t name_length;
	int32_t name_offset;
} resfile_node;

struct resfile {
	char* name;
	int32_t node_count;
	int32_t metadata_offset;
	int32_t names_length;
	char* names;
	resfile_node* nodes;
	memfile* mem;
};

static int name_hash(const char* name, int limit)
{
	int sum = 0;
	while (*name) {
		sum += tolower(*name++);
	}
	return sum % limit;
}

extern resfile* resfile_open_memfile(const char* name, memfile* mem)
{
	resfile* res = calloc(1, sizeof(resfile));
	if (NULL == res) {
		return NULL;
	}

	res->name = strdup(name);
	if (NULL == res->name) {
		resfile_close(res);
		return NULL;
	}

	uint32_t signature;
	if (1 != memfile_read(&signature, sizeof(uint32_t), 1, mem)) {
		resfile_close(res);
		return NULL;
	}

	le2cpu32s(&signature);
	if (RES_SIGNATURE != signature) {
		resfile_close(res);
		return NULL;
	}

	if (1 != memfile_read(&res->node_count, sizeof(int32_t), 1, mem) ||
			1 != memfile_read(&res->metadata_offset, sizeof(int32_t), 1, mem) ||
			1 != memfile_read(&res->names_length, sizeof(int32_t), 1, mem)) {
		resfile_close(res);
		return NULL;
	}

	le2cpu32s((uint32_t*)&res->node_count);
	le2cpu32s((uint32_t*)&res->metadata_offset);
	le2cpu32s((uint32_t*)&res->names_length);

	res->nodes = calloc(res->node_count, sizeof(resfile_node));
	if (NULL == res->nodes) {
		resfile_close(res);
		return NULL;
	}

	if (0 != memfile_seek(res->metadata_offset, SEEK_SET, mem)) {
		resfile_close(res);
		return NULL;
	}

	for (int i = 0; i < res->node_count; ++i) {
		resfile_node* node = res->nodes + i;
		if (1 != memfile_read(&node->next_index, sizeof(int32_t), 1, mem) ||
				1 != memfile_read(&node->data_length, sizeof(int32_t), 1, mem) ||
				1 != memfile_read(&node->data_offset, sizeof(int32_t), 1, mem) ||
				1 != memfile_read(&node->modified, sizeof(int32_t), 1, mem) ||
				1 != memfile_read(&node->name_length, sizeof(int16_t), 1, mem) ||
				1 != memfile_read(&node->name_offset, sizeof(int32_t), 1, mem)) {
			resfile_close(res);
			return NULL;
		}
		le2cpu32s((uint32_t*)&node->next_index);
		le2cpu32s((uint32_t*)&node->data_length);
		le2cpu32s((uint32_t*)&node->data_offset);
		le2cpu32s((uint32_t*)&node->modified);
		le2cpu16s((uint16_t*)&node->name_length);
		le2cpu32s((uint32_t*)&node->name_offset);
	}

	res->names = malloc(res->names_length);
	if (NULL == res->names ||
			1 != memfile_read(res->names, res->names_length, 1, mem)) {
		resfile_close(res);
		return NULL;
	}

	for (int i = 0; i < res->node_count; ++i) {
		resfile_node* node = res->nodes + i;
		node->name = strndup(res->names + node->name_offset, node->name_length);
		if (NULL == node->name) {
			resfile_close(res);
			return NULL;
		}
	}

	res->mem = mem;

	return res;
}

resfile* resfile_open_file(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL == file) {
		return NULL;
	}

	memfile* mem = memfile_open_callbacks(IO_CALLBACKS_FILE, file);
	if (NULL == mem) {
		fclose(file);
		return NULL;
	}

	const char* name = strrpbrk(path, "\\/");
	if (NULL == name) {
		name = path;
	} else {
		++name;
	}

	resfile* res = resfile_open_memfile(name, mem);
	if (NULL == res) {
		memfile_close(mem);
		return NULL;
	}

	return res;
}

int resfile_close(resfile* res)
{
	if (NULL == res) {
		return 0;
	}

	if (NULL != res->nodes) {
		for (int i = 0; i < res->node_count; ++i) {
			free(res->nodes[i].name);
		}
	}

	free(res->name);
	free(res->names);
	free(res->nodes);

	memfile_close(res->mem);

	free(res);

	return 0;
}

const char* resfile_name(const resfile* res)
{
	return res->name;
}

int resfile_node_count(const resfile* res)
{
	return res->node_count;
}

int resfile_node_index(const char* name, const resfile* res)
{
	const resfile_node* node;
	int index = name_hash(name, res->node_count);
	for (; index >= 0; index = node->next_index) {
		node = res->nodes + index;
		if (0 == strcasecmp(name, node->name)) {
			break;
		}
	}
	return index;
}

const char* resfile_node_name(int index, const resfile* res)
{
	return res->nodes[index].name;
}

size_t resfile_node_size(int index, const resfile* res)
{
	return res->nodes[index].data_length;
}

time_t resfile_node_modified(int index, const resfile* res)
{
	return res->nodes[index].modified;
}

memfile* resfile_node_memfile(int index, resfile* res)
{
	resfile_node* node = res->nodes + index;

	void* data = malloc(node->data_length);
	if (NULL == data) {
		return NULL;
	}

	if (0 != memfile_seek(node->data_offset, SEEK_SET, res->mem) ||
			1 != memfile_read(data, node->data_length, 1, res->mem)) {
		free(data);
		return NULL;
	}

	memfile* mem = memfile_open_data(data, node->data_length, "rb");
	if (NULL == mem) {
		free(data);
		return NULL;
	}

	return mem;
}
