#include <stdlib.h>
#include <stdio.h>

#include "memfile.h"

struct memfile
{
	void* data;
	FILE* file;
};

memfile* memfile_open(void* data, size_t size, const char* mode)
{
	memfile* mem = calloc(1, sizeof(memfile));
	if (NULL == mem) {
		return NULL;
	}

	mem->file = fmemopen(data, size, mode);
	if (NULL == mem->file) {
		memfile_close(mem);
		return NULL;
	}

	mem->data = data;

	return mem;
}

int memfile_close(memfile* mem)
{
	if (NULL == mem) {
		return 0;
	}

	if (NULL != mem->file) {
		fclose(mem->file);
	}

	free(mem->data);

	free(mem);

	return 0;
}

size_t memfile_read(void* data, size_t size, size_t n, memfile* mem)
{
	return fread(data, size, n, mem->file);
}

size_t memfile_write(const void* data, size_t size, size_t n, memfile* mem)
{
	return fwrite(data, size, n, mem->file);
}

int memfile_seek(long int offset, int whence, memfile* mem)
{
	return fseek(mem->file, offset, whence);
}

long int memfile_tell(memfile* mem)
{
	return ftell(mem->file);
}
