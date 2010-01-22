#include <stdio.h>

#include "memfile.h"

memfile* memfile_open_data(void* data, size_t size, const char* mode)
{
	FILE* file = fmemopen(data, size, mode);
	if (NULL == file) {
		return NULL;
	}

	memfile* mem = memfile_open_callbacks(CEIO_CALLBACKS_FILE, file);
	if (NULL == mem) {
		fclose(file);
		return NULL;
	}

	return mem;
}
