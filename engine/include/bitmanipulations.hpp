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

#ifndef CE_BITWISEOPERATIONS_HPP
#define CE_BITWISEOPERATIONS_HPP

#include <cstddef>
#include <cstdint>

#define ce_bitset(T, v, p) ce_bitset_##T(v, p)
#define ce_bitclr(T, v, p) ce_bitclr_##T(v, p)
#define ce_bitflp(T, v, p) ce_bitflp_##T(v, p)
#define ce_bittst(T, v, p) ce_bittst_##T(v, p)

#define CE_BITOP_DEF_BITSET(T) \
inline T ce_bitset_##T(T v, size_t p) \
{ \
    return v | ((T)0x1 << p); \
}

#define CE_BITOP_DEF_BITCLR(T) \
inline T ce_bitclr_##T(T v, size_t p) \
{ \
    return v & ~((T)0x1 << p); \
}

#define CE_BITOP_DEF_BITFLP(T) \
inline T ce_bitflp_##T(T v, size_t p) \
{ \
    return v ^ ((T)0x1 << p); \
}

#define CE_BITOP_DEF_BITTST(T) \
inline bool ce_bittst_##T(T v, size_t p) \
{ \
    return v & ((T)0x1 << p); \
}

#define CE_BITOP_DEF_ALL(T) \
CE_BITOP_DEF_BITSET(T) \
CE_BITOP_DEF_BITCLR(T) \
CE_BITOP_DEF_BITFLP(T) \
CE_BITOP_DEF_BITTST(T)

namespace cursedearth
{
    CE_BITOP_DEF_ALL(uint8_t)
    CE_BITOP_DEF_ALL(uint16_t)
    CE_BITOP_DEF_ALL(uint32_t)
}

#undef CE_BITOP_DEF_ALL
#undef CE_BITOP_DEF_BITTST
#undef CE_BITOP_DEF_BITFLP
#undef CE_BITOP_DEF_BITCLR
#undef CE_BITOP_DEF_BITSET

#endif
