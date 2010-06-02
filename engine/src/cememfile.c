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

int CE_MEMFILE_SEEK_CUR = SEEK_CUR;
int CE_MEMFILE_SEEK_END = SEEK_END;
int CE_MEMFILE_SEEK_SET = SEEK_SET;

ce_memfile* ce_memfile_open(ce_memfile_vtable vtable)
{
	ce_memfile* memfile = ce_alloc_zero(sizeof(ce_memfile) + vtable.size);
	memfile->vtable = vtable;
	return memfile;
}

void ce_memfile_close(ce_memfile* memfile)
{
	if (NULL != memfile) {
		if (NULL != memfile->vtable.close) {
			(memfile->vtable.close)(memfile);
		}
		ce_free(memfile, sizeof(ce_memfile) + memfile->vtable.size);
	}
}

typedef struct {
	size_t size, pos;
	char* data;
} ce_datafile;

static int ce_datafile_close(ce_memfile* memfile)
{
	ce_datafile* datafile = (ce_datafile*)memfile->impl;
	ce_free(datafile->data, datafile->size);
	return 0;
}

static size_t ce_datafile_read(ce_memfile* memfile, void* data, size_t size, size_t n)
{
	ce_datafile* datafile = (ce_datafile*)memfile->impl;
	if (datafile->pos == datafile->size) {
		return 0;
	}
	size_t avail_n = ce_smin((datafile->size - datafile->pos) / size, n);
	size_t avail_size = size * avail_n;
	memcpy(data, datafile->data + datafile->pos, avail_size);
	datafile->pos += avail_size;
	return avail_n;
}

static int ce_datafile_seek(ce_memfile* memfile, long int offset, int whence)
{
	ce_datafile* datafile = (ce_datafile*)memfile->impl;
	// FIXME: not clean
	long int size = datafile->size, pos = datafile->pos;
	pos = SEEK_SET == whence ? offset :
		(SEEK_END == whence ? size - offset : pos + offset);
	return pos < 0 || pos > size ? -1 : (datafile->pos = pos, 0);
}

static long int ce_datafile_tell(ce_memfile* memfile)
{
	ce_datafile* datafile = (ce_datafile*)memfile->impl;
	return datafile->pos;
}

static int ce_datafile_eof(ce_memfile* memfile)
{
	ce_datafile* datafile = (ce_datafile*)memfile->impl;
	return datafile->pos == datafile->size;
}

static int ce_datafile_error(ce_memfile* memfile)
{
	// always successful
	ce_unused(memfile);
	return 0;
}

ce_memfile* ce_memfile_open_data(void* data, size_t size)
{
	ce_memfile* memfile = ce_memfile_open((ce_memfile_vtable)
		{sizeof(ce_datafile), ce_datafile_close,
		ce_datafile_read, ce_datafile_seek, ce_datafile_tell,
		ce_datafile_eof, ce_datafile_error});

	ce_datafile* datafile = (ce_datafile*)memfile->impl;
	datafile->size = size;
	datafile->data = data;

	return memfile;
}

typedef struct {
	FILE* file;
	//char buffer[BUFSIZ];
} ce_bstdfile;

static int ce_bstdfile_close(ce_memfile* memfile)
{
	ce_bstdfile* bstdfile = (ce_bstdfile*)memfile->impl;
	fclose(bstdfile->file);
	return 0;
}

static size_t ce_bstdfile_read(ce_memfile* memfile, void* data, size_t size, size_t n)
{
	ce_bstdfile* bstdfile = (ce_bstdfile*)memfile->impl;
	return fread(data, size, n, bstdfile->file);
}

static int ce_bstdfile_seek(ce_memfile* memfile, long int offset, int whence)
{
	ce_bstdfile* bstdfile = (ce_bstdfile*)memfile->impl;
	return fseek(bstdfile->file, offset, whence);
}

static long int ce_bstdfile_tell(ce_memfile* memfile)
{
	ce_bstdfile* bstdfile = (ce_bstdfile*)memfile->impl;
	return ftell(bstdfile->file);
}

static int ce_bstdfile_eof(ce_memfile* memfile)
{
	ce_bstdfile* bstdfile = (ce_bstdfile*)memfile->impl;
	return feof(bstdfile->file);
}

static int ce_bstdfile_error(ce_memfile* memfile)
{
	ce_bstdfile* bstdfile = (ce_bstdfile*)memfile->impl;
	return ferror(bstdfile->file);
}

ce_memfile* ce_memfile_open_path(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL == file) {
		ce_error_report_c_last("memfile");
		ce_logging_error("memfile: could not open file: '%s'", path);
		return NULL;
	}

	ce_memfile* memfile = ce_memfile_open((ce_memfile_vtable)
		{sizeof(ce_bstdfile), ce_bstdfile_close,
		ce_bstdfile_read, ce_bstdfile_seek, ce_bstdfile_tell,
		ce_bstdfile_eof, ce_bstdfile_error});

	ce_bstdfile* bstdfile = (ce_bstdfile*)memfile->impl;
	bstdfile->file = file;

	return memfile;
}
