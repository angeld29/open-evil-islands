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
    void endian_swap(uint16_t& value)
    {
        value = value << 8 | value >> 8;
    }

    void endian_swap(uint32_t& value)
    {
        value = value << 24 | value >> 24 |
            (value & static_cast<uint32_t>(0x0000ff00UL)) << 8 |
            (value & static_cast<uint32_t>(0x00ff0000UL)) >> 8;
    }

    void endian_swap(uint64_t& value)
    {
        value = value << 56 | value >> 56 |
            (value & static_cast<uint64_t>(0x000000000000ff00ULL)) << 40 |
            (value & static_cast<uint64_t>(0x0000000000ff0000ULL)) << 24 |
            (value & static_cast<uint64_t>(0x00000000ff000000ULL)) << 8  |
            (value & static_cast<uint64_t>(0x000000ff00000000ULL)) >> 8  |
            (value & static_cast<uint64_t>(0x0000ff0000000000ULL)) >> 24 |
            (value & static_cast<uint64_t>(0x00ff000000000000ULL)) >> 40;
    }

    void do_cpu2le(uint16_t& value)
    {
        if (endian_t::BIG == host_order()) {
            endian_swap(value);
        }
    }

    void do_cpu2le(uint32_t& value)
    {
        if (endian_t::BIG == host_order()) {
            endian_swap(value);
        }
    }

    void do_cpu2le(uint64_t& value)
    {
        if (endian_t::BIG == host_order()) {
            endian_swap(value);
        }
    }

    void do_le2cpu(uint16_t& value)
    {
        if (endian_t::BIG == host_order()) {
            endian_swap(value);
        }
    }

    void do_le2cpu(uint32_t& value)
    {
        if (endian_t::BIG == host_order()) {
            endian_swap(value);
        }
    }

    void do_le2cpu(uint64_t& value)
    {
        if (endian_t::BIG == host_order()) {
            endian_swap(value);
        }
    }

    void do_cpu2be(uint16_t& value)
    {
        if (endian_t::LITTLE == host_order()) {
            endian_swap(value);
        }
    }

    void do_cpu2be(uint32_t& value)
    {
        if (endian_t::LITTLE == host_order()) {
            endian_swap(value);
        }
    }

    void do_cpu2be(uint64_t& value)
    {
        if (endian_t::LITTLE == host_order()) {
            endian_swap(value);
        }
    }

    void do_be2cpu(uint16_t& value)
    {
        if (endian_t::LITTLE == host_order()) {
            endian_swap(value);
        }
    }

    void do_be2cpu(uint32_t& value)
    {
        if (endian_t::LITTLE == host_order()) {
            endian_swap(value);
        }
    }

    void do_be2cpu(uint64_t& value)
    {
        if (endian_t::LITTLE == host_order()) {
            endian_swap(value);
        }
    }
}
