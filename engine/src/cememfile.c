/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

const int CE_MEM_FILE_SEEK_CUR = SEEK_CUR;
const int CE_MEM_FILE_SEEK_END = SEEK_END;
const int CE_MEM_FILE_SEEK_SET = SEEK_SET;

ce_mem_file* ce_mem_file_new(ce_mem_file_vtable vtable)
{
	ce_mem_file* mem_file = ce_alloc_zero(sizeof(ce_mem_file) + vtable.size);
	mem_file->vtable = vtable;
	return mem_file;
}

void ce_mem_file_del(ce_mem_file* mem_file)
{
	if (NULL != mem_file) {
		if (NULL != mem_file->vtable.close) {
			(mem_file->vtable.close)(mem_file);
		}
		ce_free(mem_file, sizeof(ce_mem_file) + mem_file->vtable.size);
	}
}

typedef struct {
	size_t size, pos;
	char* data;
} ce_data_file;

static int ce_data_file_close(ce_mem_file* mem_file)
{
	ce_data_file* data_file = (ce_data_file*)mem_file->impl;
	ce_free(data_file->data, data_file->size);
	return 0;
}

static size_t ce_data_file_read(ce_mem_file* mem_file, void* restrict ptr, size_t size, size_t n)
{
	ce_data_file* data_file = (ce_data_file*)mem_file->impl;
	n = ce_min(size_t, n, (data_file->size - data_file->pos) / size);
	size *= n;
	memcpy(ptr, data_file->data + data_file->pos, size);
	data_file->pos += size;
	return n;
}

static int ce_data_file_seek(ce_mem_file* mem_file, long int offset, int whence)
{
	ce_data_file* data_file = (ce_data_file*)mem_file->impl;

	if (CE_MEM_FILE_SEEK_SET == whence) {
		if (offset >= 0) {
			size_t pos = offset;
			if (pos <= data_file->size) {
				data_file->pos = pos;
				return 0;
			}
		}
		errno = ERANGE;
		return -1;
	}

	if (CE_MEM_FILE_SEEK_CUR == whence) {
		if (offset >= 0) {
			size_t n = offset;
			if (n <= data_file->size - data_file->pos) {
				data_file->pos += n;
				return 0;
			}
		} else {
			size_t n = -offset;
			if (n <= data_file->pos) {
				data_file->pos -= n;
				return 0;
			}
		}
		errno = ERANGE;
		return -1;
	}

	if (CE_MEM_FILE_SEEK_END == whence) {
		if (offset <= 0) {
			size_t n = -offset;
			if (n <= data_file->size) {
				data_file->pos = data_file->size - n;
				return 0;
			}
		}
		errno = ERANGE;
		return -1;
	}

	errno = EINVAL;
	return -1;
}

static long int ce_data_file_tell(ce_mem_file* mem_file)
{
	ce_data_file* data_file = (ce_data_file*)mem_file->impl;
	return data_file->pos;
}

static int ce_data_file_eof(ce_mem_file* mem_file)
{
	ce_data_file* data_file = (ce_data_file*)mem_file->impl;
	return data_file->pos == data_file->size;
}

static int ce_data_file_error(ce_mem_file* mem_file)
{
	// always successful
	ce_unused(mem_file);
	return 0;
}

ce_mem_file* ce_mem_file_new_data(void* restrict data, size_t size)
{
	ce_mem_file* mem_file = ce_mem_file_new((ce_mem_file_vtable)
		{sizeof(ce_data_file), ce_data_file_close,
		ce_data_file_read, ce_data_file_seek, ce_data_file_tell,
		ce_data_file_eof, ce_data_file_error});

	ce_data_file* data_file = (ce_data_file*)mem_file->impl;
	data_file->size = size;
	data_file->data = data;

	return mem_file;
}

/*
 *  fread(3) is part of the C library, and provides buffered reads.
 *  It is usually implemented by calling read(2) in order to fill its buffer.
 *  So we will not try to implement own buffers here.
 *  See also ISO/IEC 9899:1999 remarks below.
*/
typedef struct {
	FILE* file;
} ce_bstd_file;

static inline void ce_bstd_file_detect_eof(ce_bstd_file* bstd_file)
{
	// Function getc is implemented as a macro,
	// so the argument should never be an expression with side effects.
	int c = getc(bstd_file->file);
	// One character of pushback is guaranteed.
	// If the value of c equals that of the macro EOF,
	// the operation fails and the input stream is unchanged.
	ungetc(c, bstd_file->file);
}

static int ce_bstd_file_close(ce_mem_file* mem_file)
{
	ce_bstd_file* bstd_file = (ce_bstd_file*)mem_file->impl;
	fclose(bstd_file->file);
	return 0;
}

static size_t ce_bstd_file_read(ce_mem_file* mem_file, void* restrict ptr, size_t size, size_t n)
{
	ce_bstd_file* bstd_file = (ce_bstd_file*)mem_file->impl;
	n = fread(ptr, size, n, bstd_file->file);
	ce_bstd_file_detect_eof(bstd_file);
	return n;
}

static int ce_bstd_file_seek(ce_mem_file* mem_file, long int offset, int whence)
{
	ce_bstd_file* bstd_file = (ce_bstd_file*)mem_file->impl;
	// A successful call to the fseek function undoes any
	// effects of the ungetc function on the stream and
	// clears the end-of-ﬁle indicator for the stream.
	int result = fseek(bstd_file->file, offset, whence);
	ce_bstd_file_detect_eof(bstd_file);
	return result;
}

static long int ce_bstd_file_tell(ce_mem_file* mem_file)
{
	ce_bstd_file* bstd_file = (ce_bstd_file*)mem_file->impl;
	return ftell(bstd_file->file);
}

static int ce_bstd_file_eof(ce_mem_file* mem_file)
{
	ce_bstd_file* bstd_file = (ce_bstd_file*)mem_file->impl;
	return feof(bstd_file->file);
}

static int ce_bstd_file_error(ce_mem_file* mem_file)
{
	ce_bstd_file* bstd_file = (ce_bstd_file*)mem_file->impl;
	return ferror(bstd_file->file);
}

ce_mem_file* ce_mem_file_new_path(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL == file) {
		return NULL;
	}

	ce_mem_file* mem_file = ce_mem_file_new((ce_mem_file_vtable)
		{sizeof(ce_bstd_file), ce_bstd_file_close,
		ce_bstd_file_read, ce_bstd_file_seek, ce_bstd_file_tell,
		ce_bstd_file_eof, ce_bstd_file_error});

	ce_bstd_file* bstd_file = (ce_bstd_file*)mem_file->impl;
	bstd_file->file = file;

	ce_bstd_file_detect_eof(bstd_file);

	return mem_file;
}
