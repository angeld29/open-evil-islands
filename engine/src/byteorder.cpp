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

#include "byteorder.hpp"

namespace cursedearth
{
static const uint16_t pattern = 0xceca;

bool ce_is_big_endian(void)
{
    return 0xce == 0[(uint8_t*)&pattern];
}

uint16_t ce_swap16(uint16_t v)
{
    return v << 8 | v >> 8;
}

uint32_t ce_swap32(uint32_t v)
{
    return v << 24 | v >> 24 |
        (v & (uint32_t)0x0000ff00UL) << 8 |
        (v & (uint32_t)0x00ff0000UL) >> 8;
}

uint64_t ce_swap64(uint64_t v)
{
    return v << 56 | v >> 56 |
        (v & (uint64_t)0x000000000000ff00ULL) << 40 |
        (v & (uint64_t)0x0000000000ff0000ULL) << 24 |
        (v & (uint64_t)0x00000000ff000000ULL) << 8  |
        (v & (uint64_t)0x000000ff00000000ULL) >> 8  |
        (v & (uint64_t)0x0000ff0000000000ULL) >> 24 |
        (v & (uint64_t)0x00ff000000000000ULL) >> 40;
}
}
