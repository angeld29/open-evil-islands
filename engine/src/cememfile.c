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
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceerror.h"
#include "cememfile.h"

typedef struct {
	char* data;
	size_t size;
	size_t pos;
} ce_datafile;

static int ce_datafile_close(void* client_data)
{
	ce_datafile* datafile = client_data;
	ce_free(datafile->data, datafile->size);
	ce_free(datafile, sizeof(ce_datafile));
	return 0;
}

static size_t
ce_datafile_read(void* client_data, void* data, size_t size, size_t n)
{
	ce_datafile* datafile = client_data;
	if (datafile->pos == datafile->size) {
		return 0;
	}
	size_t avail_n = ce_smin((datafile->size - datafile->pos) / size, n);
	size_t avail_size = size * avail_n;
	memcpy(data, datafile->data + datafile->pos, avail_size);
	datafile->pos += avail_size;
	return avail_n;
}

static int ce_datafile_seek(void* client_data, long int offset, int whence)
{
	ce_datafile* datafile = client_data;
	// FIXME: not clean
	long int size = datafile->size, pos = datafile->pos;
	pos = SEEK_SET == whence ? offset :
		(SEEK_END == whence ? size - offset : pos + offset);
	return pos < 0 || pos > size ? -1 : (datafile->pos = pos, 0);
}

static long int ce_datafile_tell(void* client_data)
{
	ce_datafile* datafile = client_data;
	return datafile->pos;
}

ce_memfile* ce_memfile_open_callbacks(ce_io_callbacks callbacks,
											void* client_data)
{
	ce_memfile* memfile = ce_alloc(sizeof(ce_memfile));
	memfile->callbacks = callbacks;
	memfile->client_data = client_data;
	return memfile;
}

ce_memfile* ce_memfile_open_data(void* data, size_t size)
{
	ce_datafile* datafile = ce_alloc(sizeof(ce_datafile));
	datafile->data = data;
	datafile->size = size;
	datafile->pos = 0;
	return ce_memfile_open_callbacks((ce_io_callbacks){ce_datafile_close,
		ce_datafile_read, ce_datafile_seek, ce_datafile_tell}, datafile);
}

ce_memfile* ce_memfile_open_path(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL == file) {
		ce_error_report_c_last("memfile");
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
