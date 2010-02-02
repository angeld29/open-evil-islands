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

#include "cealloc.h"
#include "cememfile.h"

struct ce_memfile {
	ce_io_callbacks callbacks;
	void* client_data;
};

ce_memfile* ce_memfile_open_callbacks(ce_io_callbacks callbacks,
											void* client_data)
{
	ce_memfile* mem = ce_alloc(sizeof(ce_memfile));
	if (NULL == mem) {
		return NULL;
	}

	mem->callbacks = callbacks;
	mem->client_data = client_data;

	return mem;
}

ce_memfile* ce_memfile_open_path(const char* path, const char* mode)
{
	FILE* file = fopen(path, mode);
	if (NULL == file) {
		return NULL;
	}

	ce_memfile* mem = ce_memfile_open_callbacks(CE_IO_CALLBACKS_FILE, file);
	if (NULL == mem) {
		fclose(file);
		return NULL;
	}

	return mem;
}

void ce_memfile_close(ce_memfile* mem)
{
	if (NULL == mem) {
		return;
	}

	if (NULL != mem->callbacks.close) {
		(mem->callbacks.close)(mem->client_data);
	}

	ce_free(mem, sizeof(ce_memfile));
}

size_t ce_memfile_read(ce_memfile* mem, void* data, size_t size, size_t n)
{
	return (mem->callbacks.read)(mem->client_data, data, size, n);
}

size_t ce_memfile_write(ce_memfile* mem, const void* data, size_t size, size_t n)
{
	return (mem->callbacks.write)(mem->client_data, data, size, n);
}

int ce_memfile_seek(ce_memfile* mem, long int offset, int whence)
{
 	return (mem->callbacks.seek)(mem->client_data, offset, whence);
}

long int ce_memfile_tell(ce_memfile* mem)
{
	return (mem->callbacks.tell)(mem->client_data);
}
