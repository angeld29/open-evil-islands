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
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "cememfile.h"

const int CE_MEMFILE_SEEK_CUR = SEEK_CUR;
const int CE_MEMFILE_SEEK_END = SEEK_END;
const int CE_MEMFILE_SEEK_SET = SEEK_SET;

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

static size_t ce_datafile_read(ce_memfile* memfile, void* restrict ptr, size_t size, size_t n)
{
	ce_datafile* datafile = (ce_datafile*)memfile->impl;
	n = ce_smin(n, (datafile->size - datafile->pos) / size);
	size *= n;
	memcpy(ptr, datafile->data + datafile->pos, size);
	datafile->pos += size;
	return n;
}

static int ce_datafile_seek(ce_memfile* memfile, long int offset, int whence)
{
	ce_datafile* datafile = (ce_datafile*)memfile->impl;

	if (CE_MEMFILE_SEEK_SET == whence) {
		if (offset >= 0) {
			size_t pos = offset;
			if (pos <= datafile->size) {
				datafile->pos = pos;
				return 0;
			}
		}
		errno = ERANGE;
		return -1;
	}

	if (CE_MEMFILE_SEEK_CUR == whence) {
		if (offset >= 0) {
			size_t n = offset;
			if (n <= datafile->size - datafile->pos) {
				datafile->pos += n;
				return 0;
			}
		} else {
			size_t n = -offset;
			if (n <= datafile->pos) {
				datafile->pos -= n;
				return 0;
			}
		}
		errno = ERANGE;
		return -1;
	}

	if (CE_MEMFILE_SEEK_END == whence) {
		if (offset <= 0) {
			size_t n = -offset;
			if (n <= datafile->size) {
				datafile->pos = datafile->size - n;
				return 0;
			}
		}
		errno = ERANGE;
		return -1;
	}

	errno = EINVAL;
	return -1;
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

ce_memfile* ce_memfile_open_data(void* restrict data, size_t size)
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

/*
 *  fread(3) is part of the C library, and provides buffered reads.
 *  It is usually implemented by calling read(2) in order to fill its buffer.
 *  So we will not try to implement own buffers here.
 *  See also ISO/IEC 9899:1999 remarks below.
*/
typedef struct {
	FILE* file;
} ce_bstdfile;

static inline void ce_bstdfile_detect_eof(ce_bstdfile* bstdfile)
{
	// Function getc is implemented as a macro,
	// so the argument should never be an expression with side effects.
	int c = getc(bstdfile->file);
	// One character of pushback is guaranteed.
	// If the value of c equals that of the macro EOF,
	// the operation fails and the input stream is unchanged.
	ungetc(c, bstdfile->file);
}

static int ce_bstdfile_close(ce_memfile* memfile)
{
	ce_bstdfile* bstdfile = (ce_bstdfile*)memfile->impl;
	fclose(bstdfile->file);
	return 0;
}

static size_t ce_bstdfile_read(ce_memfile* memfile, void* restrict ptr, size_t size, size_t n)
{
	ce_bstdfile* bstdfile = (ce_bstdfile*)memfile->impl;
	n = fread(ptr, size, n, bstdfile->file);
	ce_bstdfile_detect_eof(bstdfile);
	return n;
}

static int ce_bstdfile_seek(ce_memfile* memfile, long int offset, int whence)
{
	ce_bstdfile* bstdfile = (ce_bstdfile*)memfile->impl;
	// A successful call to the fseek function undoes any
	// effects of the ungetc function on the stream and
	// clears the end-of-ﬁle indicator for the stream.
	int result = fseek(bstdfile->file, offset, whence);
	ce_bstdfile_detect_eof(bstdfile);
	return result;
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
		return NULL;
	}

	ce_memfile* memfile = ce_memfile_open((ce_memfile_vtable)
		{sizeof(ce_bstdfile), ce_bstdfile_close,
		ce_bstdfile_read, ce_bstdfile_seek, ce_bstdfile_tell,
		ce_bstdfile_eof, ce_bstdfile_error});

	ce_bstdfile* bstdfile = (ce_bstdfile*)memfile->impl;
	bstdfile->file = file;

	ce_bstdfile_detect_eof(bstdfile);

	return memfile;
}
