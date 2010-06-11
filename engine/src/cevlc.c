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

/*
 *  Based on:
 *  1. FFmpeg (C) Michael Niedermayer
 *            (C) 2000, 2001 Fabrice Bellard
 *            (C) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *            (C) 2010 Loren Merritt
*/

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "cebyteorder.h"
#include "cevlc.h"

typedef struct {
	uint8_t bits;
	uint16_t symbol;
	// codeword, with the first bit-to-be-read in the msb
	// (even if intended for a little-endian bitstream reader)
	uint32_t code;
} ce_vlccode;

static int ce_vlccode_comp(const void* a, const void* b)
{
	const ce_vlccode *ca = a, *cb = b;
	return (ca->code >> 1) - (cb->code >> 1);
}

static uint8_t ce_vlc_get_data(const uint8_t* table, int i, int wrap, int size)
{
	const void* ptr = table + i * wrap;
	uint8_t v;
	switch (size) {
	case 1:
		v = *(const uint8_t*)ptr;
		break;
	case 2:
		v = *(const uint16_t*)ptr;
		break;
	default:
		v = *(const uint32_t*)ptr;
		break;
    }
	return v;
}

static int ce_vlc_copy(ce_vlccode* buffer, int nb_bits, int nb_codes,
	const void* bits, int bits_wrap, int bits_size,
	const void* codes, int codes_wrap, int codes_size,
	const void* symbols, int symbols_wrap, int symbols_size,
	ce_vlctable_flags flags, bool flag, int index)
{
	for (int i = 0; i < nb_codes; ++i) {
		buffer[index].bits = ce_vlc_get_data(bits, i, bits_wrap, bits_size);

		if (flag) {
			if (!(buffer[index].bits > nb_bits)) {
				continue;
			}
		} else {
			if (!(buffer[index].bits && buffer[index].bits <= nb_bits)) {
				continue;
			}
		}

		buffer[index].code = ce_vlc_get_data(codes, i, codes_wrap, codes_size);

		if (CE_VLCTABLE_LE & flags) {
			buffer[index].code = ce_swap32(buffer[index].code);
		} else {
			buffer[index].code <<= 32 - buffer[index].bits;
		}

		if (NULL != symbols) {
			buffer[index].symbol = ce_vlc_get_data(symbols, i, symbols_wrap, symbols_size);
		} else {
			buffer[index].symbol = i;
		}

		++index;
	}
	return index;
}

static int ce_vlctable_alloc(ce_vlctable* vlctable, int size)
{
	int index = vlctable->table_size;
	vlctable->table_size += size;
	assert(vlctable->table_size <= vlctable->table_allocated && "too little memory");
	return index;
}

static int ce_vlctable_build(ce_vlctable* vlctable, ce_vlccode* vlccodes,
	int nb_bits, int nb_codes, ce_vlctable_flags flags)
{
	int table_size, table_index, index, code_prefix, symbol, subtable_bits;
	int i, j, k, n, nb, inc;
	uint32_t code;
	int16_t (*table)[2];

	table_size = 1 << nb_bits;
	table_index = ce_vlctable_alloc(vlctable, table_size);

	table = &vlctable->table[table_index];

	for (i = 0; i < table_size; ++i) {
		table[i][1] = 0; // bits
		table[i][0] = -1; // codes
	}

	for (i = 0; i < nb_codes; ++i) {
		// map codes and compute auxillary table sizes
        n = vlccodes[i].bits;
        code = vlccodes[i].code;
        symbol = vlccodes[i].symbol;
        if (n <= nb_bits) {
            // no need to add another table
            j = code >> (32 - nb_bits);
            nb = 1 << (nb_bits - n);
            inc = 1;
            if (CE_VLCTABLE_LE & flags) {
                j = ce_swap32(code);
                inc = 1 << n;
            }
            for (k = 0; k < nb; k++) {
				assert(0 == table[j][1] /* bits */ && "incorrect codes");
                table[j][1] = n; // bits
                table[j][0] = symbol;
                j += inc;
            }
        } else {
            // fill auxiliary table recursively
            n -= nb_bits;
            code_prefix = code >> (32 - nb_bits);
            subtable_bits = n;
            vlccodes[i].bits = n;
            vlccodes[i].code = code << nb_bits;
            for (k = i+1; k < nb_codes; k++) {
                n = vlccodes[k].bits - nb_bits;
                if (n <= 0)
                    break;
                code = vlccodes[k].code;
                if (code >> (32 - nb_bits) != code_prefix)
                    break;
                vlccodes[k].bits = n;
                vlccodes[k].code = code << nb_bits;
                subtable_bits = ce_max(subtable_bits, n);
            }
            subtable_bits = ce_min(subtable_bits, nb_bits);
            j = (CE_VLCTABLE_LE & flags) ? ce_swap32(code_prefix) >> (32 - nb_bits) : code_prefix;
            table[j][1] = -subtable_bits;
            index = ce_vlctable_build(vlctable, vlccodes + i, subtable_bits, k-i, flags);
            // note: realloc has been done, so reload tables
            table = &vlctable->table[table_index];
            table[j][0] = index; // code
            i = k-1;
        }
    }

	return table_index;
}

void ce_vlctable_init(ce_vlctable* vlctable, int nb_bits, int nb_codes,
	const void* bits, int bits_wrap, int bits_size,
	const void* codes, int codes_wrap, int codes_size,
	const void* symbols, int symbols_wrap, int symbols_size,
	ce_vlctable_flags flags)
{
	assert(symbols_size <= 2 || NULL == symbols);
	vlctable->bits = nb_bits;

	ce_vlccode vlccodes[nb_codes + 1];

	int index = ce_vlc_copy(vlccodes, nb_bits, nb_codes,
		bits, bits_wrap, bits_size,
		codes, codes_wrap, codes_size,
		symbols, symbols_wrap, symbols_size,
		flags, true, 0);

	qsort(vlccodes, index, sizeof(ce_vlccode), ce_vlccode_comp);

	nb_codes = ce_vlc_copy(vlccodes, nb_bits, nb_codes,
		bits, bits_wrap, bits_size,
		codes, codes_wrap, codes_size,
		symbols, symbols_wrap, symbols_size,
		flags, false, index);

	ce_vlctable_build(vlctable, vlccodes, nb_bits, nb_codes, flags);
}

void ce_vlctable_clean(ce_vlctable* vlctable)
{
	if (NULL != vlctable) {
	}
}
