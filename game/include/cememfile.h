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

#include "ceio.h"
#include "cememfilefwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 *  You may to instruct memfile to either automatically close or not to close
 *  the resource in memfile_close. Automatic closure is disabled by passing
 *  NULL as the close callback. The client is responsible for closing
 *  a resource when a call to memfile_open_callbacks is unsuccessful.
*/
extern ce_memfile* ce_memfile_open_callbacks(ce_io_callbacks callbacks,
													void* client_data);
extern ce_memfile* ce_memfile_open_data(void* data, size_t size,
												const char* mode);
extern ce_memfile* ce_memfile_open_path(const char* path, const char* mode);
extern void ce_memfile_close(ce_memfile* mem);

extern size_t ce_memfile_read(ce_memfile* mem, void* data,
									size_t size, size_t n);
extern size_t ce_memfile_write(ce_memfile* mem, const void* data,
									size_t size, size_t n);

extern int ce_memfile_seek(ce_memfile* mem, long int offset, int whence);
extern long int ce_memfile_tell(ce_memfile* mem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MEMFILE_H */
