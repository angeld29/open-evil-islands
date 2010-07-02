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

#ifndef CE_RINGBUFFER_H
#define CE_RINGBUFFER_H

#include <stddef.h>

#include "cethread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	size_t capacity, start, end;
	ce_semaphore* prepared_data;
	ce_semaphore* unprepared_data;
	char data[];
} ce_ring_buffer;

extern ce_ring_buffer* ce_ring_buffer_new(size_t capacity);
extern void ce_ring_buffer_del(ce_ring_buffer* ring_buffer);

extern size_t ce_ring_buffer_read(ce_ring_buffer* ring_buffer,
									void* buffer, size_t size);

extern size_t ce_ring_buffer_write(ce_ring_buffer* ring_buffer,
									const void* buffer, size_t size);

static inline size_t ce_ring_buffer_size_read(ce_ring_buffer* ring_buffer)
{
	return ce_semaphore_available(ring_buffer->prepared_data);
}

static inline size_t ce_ring_buffer_size_write(ce_ring_buffer* ring_buffer)
{
	return ce_semaphore_available(ring_buffer->unprepared_data);
}

#ifdef __cplusplus
}
#endif

#endif /* CE_RINGBUFFER_H */
