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
#include <stdint.h>

#define ce_pass() (void)(0)
#define ce_unused(var) (void)(var)

#define CE_DEF_MIN(name, type) \
static inline type ce_##name(type a, type b) \
{ \
	return a < b ? a : b; \
}

#define CE_DEF_MAX(name, type) \
static inline type ce_##name(type a, type b) \
{ \
	return a > b ? a : b; \
}

#define CE_DEF_CLAMP(name, type) \
static inline type ce_##name(type v, type a, type b) \
{ \
	return v < a ? a : (v > b ? b : v); \
}

#define CE_DEF_SWAP(name, type) \
static inline void ce_##name(type* a, type* b) \
{ \
	*a ^= *b; /* a' = (a ^ b)           */ \
	*b ^= *a; /* b' = (b ^ (a ^ b)) = a */ \
	*a ^= *b; /* a' = (a ^ b) ^ a = b   */ \
}

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

CE_DEF_MIN(min, int)
CE_DEF_MIN(smin, size_t)

CE_DEF_MAX(max, int)
CE_DEF_MAX(smax, size_t)

CE_DEF_CLAMP(clamp, int)
CE_DEF_CLAMP(sclamp, size_t)

CE_DEF_SWAP(swap, int)
CE_DEF_SWAP(sswap, size_t)

// is power of two (using 2's complement arithmetic)
static inline bool ce_sispot(size_t v)
{
	return 0 == (v & (v - 1));
}

// next largest power of two (using SWAR algorithm)
static inline size_t ce_snlpot(size_t v)
{
	v |= (v >> 1);
	v |= (v >> 2);
	v |= (v >> 4);
	v |= (v >> 8);
	v |= (v >> 16);
#if CE_SIZEOF_SIZE_T > 4
	v |= (v >> 32);
#endif
#if CE_SIZEOF_SIZE_T > 8
	v |= (v >> 64);
#endif
	return v + 1;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#undef CE_DEF_MIN
#undef CE_DEF_MAX
#undef CE_DEF_CLAMP
#undef CE_DEF_SWAP

#endif /* CE_LIB_H */
