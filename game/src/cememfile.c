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

#include "celogging.h"
#include "cealloc.h"
#include "cememfile.h"

ce_memfile* ce_memfile_open_callbacks(ce_io_callbacks callbacks,
											void* client_data)
{
	ce_memfile* memfile = ce_alloc(sizeof(ce_memfile));
	memfile->callbacks = callbacks;
	memfile->client_data = client_data;
	return memfile;
}

ce_memfile* ce_memfile_open_file(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL == file) {
		ce_logging_error("memfile: could not open file: '%s'", path);
		return NULL;
	}
	return ce_memfile_open_callbacks(CE_IO_CALLBACKS_FILE, file);
}

void ce_memfile_close(ce_memfile* memfile)
{
	if (NULL != memfile) {
		if (NULL != memfile->callbacks.close) {
			(memfile->callbacks.close)(memfile->client_data);
		}
		ce_free(memfile, sizeof(ce_memfile));
	}
}

size_t ce_memfile_read(ce_memfile* memfile, void* data, size_t size, size_t n)
{
	return (memfile->callbacks.read)(memfile->client_data, data, size, n);
}

int ce_memfile_seek(ce_memfile* memfile, long int offset, int whence)
{
 	return (memfile->callbacks.seek)(memfile->client_data, offset, whence);
}

long int ce_memfile_tell(ce_memfile* memfile)
{
	return (memfile->callbacks.tell)(memfile->client_data);
}
