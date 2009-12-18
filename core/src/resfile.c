#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "byteorder.h"
#include "str.h"
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
	io_callbacks callbacks;
	void* client_data;
	resfile_node* nodes;
};

static int name_hash(const char* name, int limit)
{
	int sum = 0;
	while (*name) {
		sum += tolower(*name++);
	}
	return sum % limit;
}

resfile* resfile_open_callbacks(io_callbacks callbacks,
								void* client_data, const char* name)
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

	int32_t signature;
	if (1 != (callbacks.read)(&signature, sizeof(int32_t), 1, client_data)) {
		resfile_close(res);
		return NULL;
	}

	le2cpu32s((uint32_t*)&signature);
	if (RES_SIGNATURE != signature) {
		resfile_close(res);
		return NULL;
	}

	if (1 != (callbacks.read)(&res->node_count,
						sizeof(int32_t), 1, client_data) ||
			1 != (callbacks.read)(&res->metadata_offset,
						sizeof(int32_t), 1, client_data) ||
			1 != (callbacks.read)(&res->names_length,
						sizeof(int32_t), 1, client_data)) {
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

	if (0 != (callbacks.seek)(res->metadata_offset, SEEK_SET, client_data)) {
		resfile_close(res);
		return NULL;
	}

	for (int i = 0; i < res->node_count; ++i) {
		resfile_node* node = res->nodes + i;
		if (1 != (callbacks.read)(&node->next_index,
							sizeof(int32_t), 1, client_data) ||
				1 != (callbacks.read)(&node->data_length,
							sizeof(int32_t), 1, client_data) ||
				1 != (callbacks.read)(&node->data_offset,
							sizeof(int32_t), 1, client_data) ||
				1 != (callbacks.read)(&node->modified,
							sizeof(int32_t), 1, client_data) ||
				1 != (callbacks.read)(&node->name_length,
							sizeof(int16_t), 1, client_data) ||
				1 != (callbacks.read)(&node->name_offset,
							sizeof(int32_t), 1, client_data)) {
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
			(size_t)res->names_length != (callbacks.read)(res->names,
									1, res->names_length, client_data)) {
		resfile_close(res);
		return NULL;
	}

	for (int i = 0; i < res->node_count; ++i) {
		resfile_node* node = res->nodes + i;
		node->name = strndup(res->names +
			node->name_offset, node->name_length);
		if (NULL == node->name) {
			resfile_close(res);
			return NULL;
		}
	}

	res->callbacks = callbacks;
	res->client_data = client_data;

	return res;
}

resfile* resfile_open_file(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL == file) {
		return NULL;
	}

	const char* name = strrpbrk(path, "\\/");
	if (NULL == name) {
		name = path;
	} else {
		++name;
	}

	resfile* res = resfile_open_callbacks(IO_CALLBACKS_FILE, file, name);
	if (NULL == res) {
		fclose(file);
		return NULL;
	}

	return res;
}

int resfile_close(resfile* res)
{
	if (NULL == res) {
		return 0;
	}

	if (NULL != res->callbacks.close && NULL != res->client_data) {
		(res->callbacks.close)(res->client_data);
	}

	if (NULL != res->nodes) {
		for (int i = 0; i < res->node_count; ++i) {
			free(res->nodes[i].name);
		}
	}

	free(res->name);
	free(res->names);
	free(res->nodes);

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

	if (0 != (res->callbacks.seek)(node->data_offset,
									SEEK_SET, res->client_data) ||
			(size_t)node->data_length != (res->callbacks.read)(data,
							1, node->data_length, res->client_data)) {
		free(data);
		return NULL;
	}

	memfile* mem = memfile_open(data, node->data_length, "rb");
	if (NULL == mem) {
		free(data);
		return NULL;
	}

	return mem;
}
