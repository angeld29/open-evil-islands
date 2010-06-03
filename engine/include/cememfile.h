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

#ifndef CE_MEMFILE_H
#define CE_MEMFILE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*
 *  Abstraction layer for read-only binary files based on FILE interface.
 *  All implementations signal an EOF of file condition synchronously
 *  with the transmission of the last bytes of a file.
*/
typedef struct ce_memfile ce_memfile;

typedef struct {
	size_t size;
	int (*close)(ce_memfile* memfile);
	size_t (*read)(ce_memfile* memfile, void* restrict ptr, size_t size, size_t n);
	int (*seek)(ce_memfile* memfile, long int offset, int whence);
	long int (*tell)(ce_memfile* memfile);
	int (*eof)(ce_memfile* memfile);
	int (*error)(ce_memfile* memfile);
} ce_memfile_vtable;

struct ce_memfile {
	ce_memfile_vtable vtable;
	char impl[];
};

extern const int CE_MEMFILE_SEEK_CUR;
extern const int CE_MEMFILE_SEEK_END;
extern const int CE_MEMFILE_SEEK_SET;

/*
 *  You may to instruct memfile to either automatically close or not to close
 *  the resource in memfile_close. Automatic closure is disabled by passing
 *  NULL as the close callback.
*/
extern ce_memfile* ce_memfile_open(ce_memfile_vtable vtable);
extern void ce_memfile_close(ce_memfile* memfile);

static inline size_t ce_memfile_read(ce_memfile* memfile, void* restrict ptr, size_t size, size_t n)
{
	return (memfile->vtable.read)(memfile, ptr, size, n);
}

static inline int ce_memfile_seek(ce_memfile* memfile, long int offset, int whence)
{
	return (memfile->vtable.seek)(memfile, offset, whence);
}

static inline long int ce_memfile_tell(ce_memfile* memfile)
{
	return (memfile->vtable.tell)(memfile);
}

static inline void ce_memfile_rewind(ce_memfile* memfile)
{
	ce_memfile_seek(memfile, 0L, CE_MEMFILE_SEEK_SET);
}

static inline int ce_memfile_eof(ce_memfile* memfile)
{
	return (memfile->vtable.eof)(memfile);
}

static inline int ce_memfile_error(ce_memfile* memfile)
{
	return (memfile->vtable.error)(memfile);
}

static inline long int ce_memfile_size(ce_memfile* memfile)
{
	long int pos = ce_memfile_tell(memfile);
	ce_memfile_seek(memfile, 0, CE_MEMFILE_SEEK_END);
	long int size = ce_memfile_tell(memfile);
	ce_memfile_seek(memfile, pos, CE_MEMFILE_SEEK_SET);
	return size;
}

/*
 *  Implements in-memory files.
 *  NOTE: memfile takes ownership of the data.
*/
extern ce_memfile* ce_memfile_open_data(void* restrict data, size_t size);

/*
 *  Implements a buffered interface for the FILE standard functions.
*/
extern ce_memfile* ce_memfile_open_path(const char* path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MEMFILE_H */
