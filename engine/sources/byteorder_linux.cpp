/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <endian.h>

#include "byteorder.hpp"

namespace cursedearth
{
    void do_cpu2le(uint16_t& value)
    {
        value = htole16(value);
    }

    void do_cpu2le(uint32_t& value)
    {
        value = htole32(value);
    }

    void do_cpu2le(uint64_t& value)
    {
        value = htole64(value);
    }

    void do_le2cpu(uint16_t& value)
    {
        value = le16toh(value);
    }

    void do_le2cpu(uint32_t& value)
    {
        value = le32toh(value);
    }

    void do_le2cpu(uint64_t& value)
    {
        value = le64toh(value);
    }

    void do_cpu2be(uint16_t& value)
    {
        value = htobe16(value);
    }

    void do_cpu2be(uint32_t& value)
    {
        value = htobe32(value);
    }

    void do_cpu2be(uint64_t& value)
    {
        value = htobe64(value);
    }

    void do_be2cpu(uint16_t& value)
    {
        value = be16toh(value);
    }

    void do_be2cpu(uint32_t& value)
    {
        value = be32toh(value);
    }

    void do_be2cpu(uint64_t& value)
    {
        value = be64toh(value);
    }
}
