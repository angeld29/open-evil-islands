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
#include <assert.h>

#include "cealloc.h"
#include "cebitarray.h"

ce_bitarray* ce_bitarray_new(size_t capacity)
{
	ce_bitarray* bitarray = ce_alloc(sizeof(ce_bitarray) + capacity);
	bitarray->capacity = capacity;
	bitarray->size = 0;
	bitarray->index = 0;
	bitarray->pos = 0;
	return bitarray;
}

void ce_bitarray_del(ce_bitarray* bitarray)
{
	if (NULL != bitarray) {
		ce_free(bitarray, sizeof(ce_bitarray) + bitarray->capacity);
	}
}

uint8_t ce_bitarray_get_bit(ce_bitarray* bitarray)
{
	uint8_t value = bitarray->array[bitarray->index] >> (7 - bitarray->pos);
	if (8 == ++bitarray->pos) {
		++bitarray->index;
		bitarray->pos = 0;
	}
	return value & 0x1;
}

uint8_t ce_bitarray_get_byte(ce_bitarray* bitarray)
{
	uint8_t value = bitarray->array[bitarray->index++];
	return (value << bitarray->pos) |
			(bitarray->array[bitarray->index] >> (8 - bitarray->pos));
}

void ce_bitarray_get_bits(ce_bitarray* bitarray, void* arg, size_t n)
{
	uint8_t* ptr = arg;
	size_t index = 0;

	// read whole bytes
	while (n >= 8) {
		ptr[index++] = ce_bitarray_get_byte(bitarray);
		n -= 8;
	}

	if (n > 0) {
		// read remaining bits
		size_t shift = 8 - n;

		while (n-- > 0) {
			ptr[index] <<= 1;
			ptr[index] |= ce_bitarray_get_bit(bitarray);
		}

		// shift last bits into position
		ptr[index] <<= shift;
	}
}
