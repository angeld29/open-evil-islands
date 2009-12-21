#include <stdlib.h>
#include <stdio.h>

#include "memfile.h"

typedef struct {
	void* data;
	FILE* file;
} data_file;

struct memfile {
	io_callbacks callbacks;
	void* client_data;
};

static int data_file_close(void* client_data)
{
	data_file* df = client_data;
	int r = fclose(df->file);
	free(df->data);
	free(df);
	return r;
}

static size_t data_file_read(void* data, size_t size,
							size_t n, void* client_data)
{
	return fread(data, size, n, ((data_file*)client_data)->file);
}

static size_t data_file_write(const void* data, size_t size,
								size_t n, void* client_data)
{
	return fwrite(data, size, n, ((data_file*)client_data)->file);
}

static int data_file_seek(long int offset, int whence, void* client_data)
{
	return fseek(((data_file*)client_data)->file, offset, whence);
}

static long int data_file_tell(void* client_data)
{
	return ftell(((data_file*)client_data)->file);
}

memfile* memfile_open_callbacks(io_callbacks callbacks, void* client_data)
{
	memfile* mem = calloc(1, sizeof(memfile));
	if (NULL == mem) {
		return NULL;
	}

	mem->callbacks = callbacks;
	mem->client_data = client_data;

	return mem;
}

memfile* memfile_open_data(void* data, size_t size, const char* mode)
{
	FILE* file = fmemopen(data, size, mode);
	if (NULL == file) {
		return NULL;
	}

	data_file* df = calloc(1, sizeof(data_file));
	if (NULL == df) {
		fclose(file);
		return NULL;
	}

	df->data = data;
	df->file = file;

	io_callbacks callbacks = { data_file_close, data_file_read,
		data_file_write, data_file_seek, data_file_tell };

	memfile* mem = memfile_open_callbacks(callbacks, df);
	if (NULL == mem) {
		fclose(file);
		return NULL;
	}

	return mem;
}

int memfile_close(memfile* mem)
{
	if (NULL == mem) {
		return 0;
	}

	if (NULL != mem->callbacks.close && NULL != mem->client_data) {
		(mem->callbacks.close)(mem->client_data);
	}

	free(mem);

	return 0;
}

size_t memfile_read(void* data, size_t size, size_t n, memfile* mem)
{
	return (mem->callbacks.read)(data, size, n, mem->client_data);
}

size_t memfile_write(const void* data, size_t size, size_t n, memfile* mem)
{
	return (mem->callbacks.write)(data, size, n, mem->client_data);
}

int memfile_seek(long int offset, int whence, memfile* mem)
{
 	return (mem->callbacks.seek)(offset, whence, mem->client_data);
}

long int memfile_tell(memfile* mem)
{
	return (mem->callbacks.tell)(mem->client_data);
}
