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
#include "ceringbuffer.h"

ce_ringbuffer* ce_ringbuffer_new(size_t capacity)
{
	ce_ringbuffer* ringbuffer = ce_alloc(sizeof(ce_ringbuffer));
	ringbuffer->capacity = capacity;
	ringbuffer->data = ce_alloc(capacity);
	ce_ringbuffer_clear(ringbuffer);
	return ringbuffer;
}

void ce_ringbuffer_del(ce_ringbuffer* ringbuffer)
{
	if (NULL != ringbuffer) {
		ce_free(ringbuffer->data, ringbuffer->capacity);
		ce_free(ringbuffer, sizeof(ce_ringbuffer));
	}
}

void ce_ringbuffer_clear(ce_ringbuffer* ringbuffer)
{
	ringbuffer->size = 0;
	ringbuffer->start = 0;
	ringbuffer->end = 0;
}

size_t ce_ringbuffer_read(ce_ringbuffer* ringbuffer, void* buffer, size_t size)
{
	char* data = buffer;
	size = ce_smin(size, ce_ringbuffer_size(ringbuffer));

	if (size > ringbuffer->capacity - ringbuffer->start) {
		size_t length = ringbuffer->capacity - ringbuffer->start;
		memcpy(data, ringbuffer->data + ringbuffer->start, length);
		memcpy(data + length, ringbuffer->data, size - length);
	} else {
		memcpy(data, ringbuffer->data + ringbuffer->start, size);
	}

	ringbuffer->start = (ringbuffer->start + size) % ringbuffer->capacity;
	ce_atomic_sub(size_t, &ringbuffer->size, size);

	return size;
}

size_t ce_ringbuffer_write(ce_ringbuffer* ringbuffer, const void* buffer, size_t size)
{
	const char* data = buffer;
	size = ce_smin(size, ce_ringbuffer_free_space(ringbuffer));

	if (size > ringbuffer->capacity - ringbuffer->end) {
		size_t length = ringbuffer->capacity - ringbuffer->end;
		memcpy(ringbuffer->data + ringbuffer->end, data, length);
		memcpy(ringbuffer->data, data + length, size - length);
	} else {
		memcpy(ringbuffer->data + ringbuffer->end, data, size);
	}

	ringbuffer->end = (ringbuffer->end + size) % ringbuffer->capacity;
	ce_atomic_add(size_t, &ringbuffer->size, size);

	return size;
}
