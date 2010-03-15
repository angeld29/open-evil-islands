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

typedef struct {
	char* data;
	size_t size;
	FILE* file;
} ce_memfile_cookie;

static int ce_memfile_cookie_close(void* client_data)
{
	ce_memfile_cookie* cookie = client_data;
	fclose(cookie->file);
	ce_free(cookie->data, cookie->size);
	ce_free(cookie, sizeof(ce_memfile_cookie));
	return 0;
}

static size_t
ce_memfile_cookie_read(void* client_data, void* data, size_t size, size_t n)
{
	ce_memfile_cookie* cookie = client_data;
	return fread(data, size, n, cookie->file);
}

static size_t ce_memfile_cookie_write(void* client_data,
										const void* data,
										size_t size, size_t n)
{
	ce_memfile_cookie* cookie = client_data;
	return fwrite(data, size, n, cookie->file);
}

static int
ce_memfile_cookie_seek(void* client_data, long int offset, int whence)
{
	ce_memfile_cookie* cookie = client_data;
	return fseek(cookie->file, offset, whence);
}

static long int ce_memfile_cookie_tell(void* client_data)
{
	ce_memfile_cookie* cookie = client_data;
	return ftell(cookie->file);
}

static const ce_io_callbacks ce_memfile_cookie_callbacks = {
	ce_memfile_cookie_close,
	ce_memfile_cookie_read, ce_memfile_cookie_write,
	ce_memfile_cookie_seek, ce_memfile_cookie_tell
};

ce_memfile* ce_memfile_open_data(void* data, size_t size, const char* mode)
{
	// TODO: Invalid read of size 1: need NULL terminated data???
	//       strlen (mc_replace_strmem.c:275)
	//       fmemopen (fmemopen.c:246)
	ce_memfile_cookie* cookie = ce_alloc(sizeof(ce_memfile_cookie));
	cookie->data = data;
	cookie->size = size;
	cookie->file = fmemopen(data, size, mode);
	return ce_memfile_open_callbacks(ce_memfile_cookie_callbacks, cookie);
}
