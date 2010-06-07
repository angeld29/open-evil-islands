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

#ifndef CE_BITOP_H
#define CE_BITOP_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define CE_DEF_BITSET(type, num) \
static inline type ce_bitset##num(type v, size_t p) \
{ \
	return v | (0x1 << p); \
}

#define CE_DEF_BITCLR(type, num) \
static inline type ce_bitclr##num(type v, size_t p) \
{ \
	return v & ~(0x1 << p); \
}

#define CE_DEF_BITFLP(type, num) \
static inline type ce_bitflp##num(type v, size_t p) \
{ \
	return v ^ (0x1 << p); \
}

#define CE_DEF_BITTST(type, num) \
static inline bool ce_bittst##num(type v, size_t p) \
{ \
	return v & (0x1 << p); \
}

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CE_DEF_BITSET(uint8_t, 8)
CE_DEF_BITCLR(uint8_t, 8)
CE_DEF_BITFLP(uint8_t, 8)
CE_DEF_BITTST(uint8_t, 8)

CE_DEF_BITSET(uint16_t, 16)
CE_DEF_BITCLR(uint16_t, 16)
CE_DEF_BITFLP(uint16_t, 16)
CE_DEF_BITTST(uint16_t, 16)

CE_DEF_BITSET(uint32_t, 32)
CE_DEF_BITCLR(uint32_t, 32)
CE_DEF_BITFLP(uint32_t, 32)
CE_DEF_BITTST(uint32_t, 32)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#undef CE_DEF_BITTST
#undef CE_DEF_BITFLP
#undef CE_DEF_BITCLR
#undef CE_DEF_BITSET

#endif /* CE_BITOP_H */
