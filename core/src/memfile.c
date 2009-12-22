#include <stdlib.h>
#include <stdio.h>

#include "memfile.h"

struct memfile {
	io_callbacks callbacks;
	void* client_data;
};

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

memfile* memfile_open_path(const char* path, const char* mode)
{
	FILE* file = fopen(path, mode);
	if (NULL == file) {
		return NULL;
	}

	memfile* mem = memfile_open_callbacks(IO_CALLBACKS_FILE, file);
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
