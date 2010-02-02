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

#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "memfile.h"

typedef struct {
	char* data;
	size_t size;
	size_t pos;
} cookie;

static int cookie_close(void* client_data)
{
	ce_free(client_data, sizeof(cookie));
	return 0;
}

static size_t cookie_read(void* client_data, void* data, size_t size, size_t n)
{
	cookie* c = client_data;

	if (c->pos == c->size) {
		return 0;
	}

	size_t avail_n = ce_smin((c->size - c->pos) / size, n);
	size_t avail_size = size * avail_n;

	memcpy(data, c->data + c->pos, avail_size);
	c->pos += avail_size;

	return avail_n;
}

static size_t cookie_write(void* client_data, const void* data,
										size_t size, size_t n)
{
	assert(false && "Not implemented");
	ce_unused(data);
	ce_unused(size);
	ce_unused(n);
	ce_unused(client_data);
	return 0;
}

static int cookie_seek(void* client_data, long int offset, int whence)
{
	assert(false && "Not implemented");
	ce_unused(offset);
	ce_unused(whence);
	ce_unused(client_data);
	return 0;
}

static long int cookie_tell(void* client_data)
{
	assert(false && "Not implemented");
	ce_unused(client_data);
	return 0;
}

static const ce_io_callbacks cookie_callbacks = {
	cookie_close, cookie_read, cookie_write, cookie_seek, cookie_tell
};

memfile* memfile_open_data(void* data, size_t size, const char* mode)
{
	ce_unused(mode);

	cookie* c = ce_alloc(sizeof(cookie));
	if (NULL == c) {
		return NULL;
	}

	c->data = data;
	c->size = size;
	c->pos = 0;

	memfile* mem = memfile_open_callbacks(cookie_callbacks, c);
	if (NULL == mem) {
		ce_free(c, sizeof(cookie));
		return NULL;
	}

	return mem;
}
