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

#ifndef CE_VLC_H
#define CE_VLC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 *  Variable Length Coding
 *
 *  Based on:
 *  1. FFmpeg (C) Michael Niedermayer
 *            (C) 2000, 2001 Fabrice Bellard
 *            (C) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *            (C) 2010 Loren Merritt
*/

typedef enum {
	CE_VLCTABLE_LE = 1,
} ce_vlctable_flags;

typedef struct {
	int bits;
	int16_t (*table)[2]; // code, bits
	int table_size, table_allocated;
} ce_vlctable;

extern void ce_vlctable_init(ce_vlctable* vlctable, int nb_bits, int nb_codes,
	const void* bits, int bits_wrap, int bits_size,
	const void* codes, int codes_wrap, int codes_size,
	const void* symbols, int symbols_wrap, int symbols_size,
	ce_vlctable_flags flags);

extern void ce_vlctable_clean(ce_vlctable* vlctable);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VLC_H */
