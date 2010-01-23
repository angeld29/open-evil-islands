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
#include "memfile.h"

struct memfile {
	ceio_callbacks callbacks;
	void* client_data;
};

memfile* memfile_open_callbacks(ceio_callbacks callbacks, void* client_data)
{
	memfile* mem = cealloc(sizeof(memfile));
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

	memfile* mem = memfile_open_callbacks(CEIO_CALLBACKS_FILE, file);
	if (NULL == mem) {
		fclose(file);
		return NULL;
	}

	return mem;
}

void memfile_close(memfile* mem)
{
	if (NULL == mem) {
		return;
	}

	if (NULL != mem->callbacks.close && NULL != mem->client_data) {
		(mem->callbacks.close)(mem->client_data);
	}

	cefree(mem, sizeof(memfile));
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
