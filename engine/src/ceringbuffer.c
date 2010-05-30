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

#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "ceatomic.h"
#include "ceringbuffer.h"

ce_ringbuffer* ce_ringbuffer_new(size_t size)
{
	ce_ringbuffer* ringbuffer = ce_alloc(sizeof(ce_ringbuffer));
	ringbuffer->size = size;
	ringbuffer->data = ce_alloc(size);
	ce_ringbuffer_clear(ringbuffer);
	return ringbuffer;
}

void ce_ringbuffer_del(ce_ringbuffer* ringbuffer)
{
	if (NULL != ringbuffer) {
		ce_free(ringbuffer->data, ringbuffer->size);
		ce_free(ringbuffer, sizeof(ce_ringbuffer));
	}
}

void ce_ringbuffer_clear(ce_ringbuffer* ringbuffer)
{
	ringbuffer->read_pos = 0;
	ringbuffer->write_pos = 0;
	ringbuffer->write_avail = ringbuffer->size;
}

size_t ce_ringbuffer_get_read_avail(ce_ringbuffer* ringbuffer)
{
	return ringbuffer->size - ce_ringbuffer_get_write_avail(ringbuffer);
}

size_t ce_ringbuffer_get_write_avail(ce_ringbuffer* ringbuffer)
{
	return ce_atomic_fetch_size_t(&ringbuffer->write_avail);
}

size_t ce_ringbuffer_read(ce_ringbuffer* ringbuffer, void* buffer, size_t size)
{
	size_t write_avail = ce_ringbuffer_get_write_avail(ringbuffer);

	if (write_avail == ringbuffer->size) {
		return 0;
	}

	char* data = buffer;
	size = ce_smin(size, ce_ringbuffer_get_read_avail(ringbuffer));

	if (size > ringbuffer->size - ringbuffer->read_pos) {
		size_t length = ringbuffer->size - ringbuffer->read_pos;
		memcpy(data, ringbuffer->data + ringbuffer->read_pos, length);
		memcpy(data + length, ringbuffer->data, size - length);
	} else {
		memcpy(data, ringbuffer->data + ringbuffer->read_pos, size);
	}

	ringbuffer->read_pos = (ringbuffer->read_pos + size) % ringbuffer->size;
	ce_atomic_add_and_fetch_size_t(&ringbuffer->write_avail, size);

	return size;
}

size_t ce_ringbuffer_write(ce_ringbuffer* ringbuffer, const void* buffer, size_t size)
{
	size_t write_avail = ce_ringbuffer_get_write_avail(ringbuffer);

	if (0 == write_avail) {
		return 0;
	}

	const char* data = buffer;
	size = ce_smin(size, write_avail);

	if (size > ringbuffer->size - ringbuffer->write_pos) {
		size_t length = ringbuffer->size - ringbuffer->write_pos;
		memcpy(ringbuffer->data + ringbuffer->write_pos, data, length);
		memcpy(ringbuffer->data, data + length, size - length);
	} else {
		memcpy(ringbuffer->data + ringbuffer->write_pos, data, size);
	}

	ringbuffer->write_pos = (ringbuffer->write_pos + size) % ringbuffer->size;
	ce_atomic_sub_and_fetch_size_t(&ringbuffer->write_avail, size);

	return size;
}
