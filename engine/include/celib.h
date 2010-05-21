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

#ifndef CE_LIB_H
#define CE_LIB_H

#include <stddef.h>
#include <stdbool.h>

#define ce_unused(var) (void)(var)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int ce_min(int a, int b);
extern int ce_max(int a, int b);
extern size_t ce_smin(size_t a, size_t b);
extern size_t ce_smax(size_t a, size_t b);

extern void ce_swap(int* a, int* b);

extern int ce_clamp(int v, int a, int b);
extern size_t ce_sclamp(size_t v, size_t a, size_t b);

// is power of two (using 2's complement arithmetic)
static inline bool ce_ispot(size_t x)
{
	return 0 == (x & (x - 1));
}

// next largest power of two (using SWAR algorithm)
static inline size_t ce_nlpot(size_t x)
{
	// FIXME: only for 32 bit value
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return x + 1;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_LIB_H */
