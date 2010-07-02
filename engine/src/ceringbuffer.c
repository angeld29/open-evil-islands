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

#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "ceringbuffer.h"

ce_ring_buffer* ce_ring_buffer_new(size_t capacity)
{
	ce_ring_buffer* ring_buffer = ce_alloc_zero(sizeof(ce_ring_buffer) + capacity);
	ring_buffer->capacity = capacity;
	ring_buffer->prepared_data = ce_semaphore_new(0);
	ring_buffer->unprepared_data = ce_semaphore_new(capacity);
	return ring_buffer;
}

void ce_ring_buffer_del(ce_ring_buffer* ring_buffer)
{
	if (NULL != ring_buffer) {
		ce_semaphore_del(ring_buffer->unprepared_data);
		ce_semaphore_del(ring_buffer->prepared_data);
		ce_free(ring_buffer, sizeof(ce_ring_buffer) + ring_buffer->capacity);
	}
}

size_t ce_ring_buffer_read(ce_ring_buffer* ring_buffer, void* buffer, size_t size)
{
	char* data = buffer;
	ce_semaphore_acquire(ring_buffer->prepared_data, size);

	if (size > ring_buffer->capacity - ring_buffer->start) {
		size_t length = ring_buffer->capacity - ring_buffer->start;
		memcpy(data, ring_buffer->data + ring_buffer->start, length);
		memcpy(data + length, ring_buffer->data, size - length);
	} else {
		memcpy(data, ring_buffer->data + ring_buffer->start, size);
	}

	ring_buffer->start = (ring_buffer->start + size) % ring_buffer->capacity;
	ce_semaphore_release(ring_buffer->unprepared_data, size);

	return size;
}

size_t ce_ring_buffer_write(ce_ring_buffer* ring_buffer, const void* buffer, size_t size)
{
	const char* data = buffer;
	ce_semaphore_acquire(ring_buffer->unprepared_data, size);

	if (size > ring_buffer->capacity - ring_buffer->end) {
		size_t length = ring_buffer->capacity - ring_buffer->end;
		memcpy(ring_buffer->data + ring_buffer->end, data, length);
		memcpy(ring_buffer->data, data + length, size - length);
	} else {
		memcpy(ring_buffer->data + ring_buffer->end, data, size);
	}

	ring_buffer->end = (ring_buffer->end + size) % ring_buffer->capacity;
	ce_semaphore_release(ring_buffer->prepared_data, size);

	return size;
}
