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

#ifndef CE_RINGBUFFER_H
#define CE_RINGBUFFER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	size_t size;
	size_t read_pos, write_pos;
	size_t write_avail;
	char* data;
} ce_ringbuffer;

extern ce_ringbuffer* ce_ringbuffer_new(size_t size);
extern void ce_ringbuffer_del(ce_ringbuffer* ringbuffer);

extern void ce_ringbuffer_clear(ce_ringbuffer* ringbuffer);

static inline size_t ce_ringbuffer_get_read_avail(const ce_ringbuffer* ringbuffer)
{
	return ringbuffer->size - ringbuffer->write_avail;
}

extern size_t ce_ringbuffer_read(ce_ringbuffer* ringbuffer,
									void* buffer, size_t size);

extern size_t ce_ringbuffer_write(ce_ringbuffer* ringbuffer,
									const void* buffer, size_t size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_RINGBUFFER_H */
