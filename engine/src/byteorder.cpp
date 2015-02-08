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

#include <stdexcept>

#include "byteorder.hpp"

namespace cursedearth
{
    const uint32_t g_little_endian_pattern = 0x03020100ul;
    const uint32_t g_big_endian_pattern = 0x00010203ul;
    const uint32_t g_middle_endian_pattern = 0x01000302ul;

    const union { uint8_t bytes[4]; uint32_t value; } g_host_order = { { 0u, 1u, 2u, 3u } };

    endian_t host_order()
    {
        switch (g_host_order.value) {
        case g_little_endian_pattern: return endian_t::little;
        case g_big_endian_pattern: return endian_t::big;
        case g_middle_endian_pattern: return endian_t::middle;
        }
        throw std::runtime_error("unknown-endian system");
    }
}
