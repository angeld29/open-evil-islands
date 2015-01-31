/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#define CE_LIB_DEF_MIN(T) \
static inline T ce_min_##T(T a, T b) \
{ \
    return a < b ? a : b; \
}

#define CE_LIB_DEF_MAX(T) \
static inline T ce_max_##T(T a, T b) \
{ \
    return a > b ? a : b; \
}

#define CE_LIB_DEF_CLAMP(T) \
static inline T ce_clamp_##T(T v, T a, T b) \
{ \
    return v < a ? a : (v > b ? b : v); \
}

#define CE_LIB_DEF_SWAP(T) \
static inline void ce_swap_##T(T* a, T* b) \
{ \
    *a ^= *b; /* a' = (a ^ b)           */ \
    *b ^= *a; /* b' = (b ^ (a ^ b)) = a */ \
    *a ^= *b; /* a' = (a ^ b) ^ a = b   */ \
}

#define CE_LIB_DEF_SWAP_TEMP(T) \
static inline void ce_swap_temp_##T(T* a, T* b) \
{ \
    T t = *a; \
    *a = *b; \
    *b = t; \
}

#define CE_LIB_DEF_ALL(T) \
CE_LIB_DEF_MIN(T) \
CE_LIB_DEF_MAX(T) \
CE_LIB_DEF_CLAMP(T) \
CE_LIB_DEF_SWAP_TEMP(T)

#define ce_min(T, a, b) ce_min_##T(a, b)
#define ce_max(T, a, b) ce_max_##T(a, b)
#define ce_clamp(T, v, a, b) ce_clamp_##T(v, a, b)
#define ce_swap(T, a, b) ce_swap_##T(a, b)
#define ce_swap_temp(T, a, b) ce_swap_temp_##T(a, b)

#ifdef __cplusplus
extern "C" {
#endif

CE_LIB_DEF_ALL(int)
CE_LIB_DEF_ALL(int16_t)
CE_LIB_DEF_ALL(int32_t)
CE_LIB_DEF_ALL(size_t)
CE_LIB_DEF_ALL(float)

// only for integers
CE_LIB_DEF_SWAP(int)
CE_LIB_DEF_SWAP(int16_t)
CE_LIB_DEF_SWAP(int32_t)
CE_LIB_DEF_SWAP(size_t)

// only for pointers
static inline void ce_swap_pointer(void* a, void* b)
{
    void **aa = a, **bb = b, *t = *aa;
    *aa = *bb;
    *bb = t;
}

// is power of two (using 2's complement arithmetic)
static inline bool ce_ispot(size_t v)
{
    return 0 == (v & (v - 1));
}

// next largest power of two (using SWAR algorithm)
static inline size_t ce_nlpot(size_t v)
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

static inline void ce_pass(void) {}

#ifdef __cplusplus
}
#endif

#undef CE_LIB_DEF_ALL
#undef CE_LIB_DEF_SWAP_TEMP
#undef CE_LIB_DEF_SWAP
#undef CE_LIB_DEF_CLAMP
#undef CE_LIB_DEF_MAX
#undef CE_LIB_DEF_MIN

#endif /* CE_LIB_H */
