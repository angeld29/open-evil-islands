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

#ifndef CE_BITARRAY_H
#define CE_BITARRAY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
	size_t capacity, size;
	size_t index, pos;
	uint8_t array[];
} ce_bitarray;

extern ce_bitarray* ce_bitarray_new(size_t capacity);
extern void ce_bitarray_del(ce_bitarray* bitarray);

static inline void ce_bitarray_reset(ce_bitarray* bitarray, size_t size)
{
	bitarray->size = size;
	bitarray->index = 0;
	bitarray->pos = 0;
}

static inline size_t ce_bitarray_count(ce_bitarray* bitarray)
{
	return 8 * bitarray->index + bitarray->pos;
}

static inline void ce_bitarray_skip_bits(ce_bitarray* bitarray, size_t n)
{
	bitarray->index += n / 8;
	bitarray->pos += n % 8;
	if (bitarray->pos >= 8) {
		++bitarray->index;
		bitarray->pos -= 8;
	}
}

extern uint32_t ce_bitarray_get_bit(ce_bitarray* bitarray);
extern uint32_t ce_bitarray_get_bits(ce_bitarray* bitarray, size_t n);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_BITARRAY_H */
