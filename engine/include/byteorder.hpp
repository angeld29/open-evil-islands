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

#ifndef CE_BYTEORDER_HPP
#define CE_BYTEORDER_HPP

#include <cstdint>

namespace cursedearth
{
    extern bool ce_is_big_endian(void);

    extern uint16_t ce_swap16(uint16_t v);
    extern uint32_t ce_swap32(uint32_t v);
    extern uint64_t ce_swap64(uint64_t v);

    extern uint16_t ce_cpu2le16(uint16_t v);
    extern uint32_t ce_cpu2le32(uint32_t v);
    extern uint64_t ce_cpu2le64(uint64_t v);

    extern uint16_t ce_le2cpu16(uint16_t v);
    extern uint32_t ce_le2cpu32(uint32_t v);
    extern uint64_t ce_le2cpu64(uint64_t v);

    extern uint16_t ce_cpu2be16(uint16_t v);
    extern uint32_t ce_cpu2be32(uint32_t v);
    extern uint64_t ce_cpu2be64(uint64_t v);

    extern uint16_t ce_be2cpu16(uint16_t v);
    extern uint32_t ce_be2cpu32(uint32_t v);
    extern uint64_t ce_be2cpu64(uint64_t v);

    extern void ce_cpu2le16s(uint16_t* v);
    extern void ce_cpu2le32s(uint32_t* v);
    extern void ce_cpu2le64s(uint64_t* v);

    extern void ce_le2cpu16s(uint16_t* v);
    extern void ce_le2cpu32s(uint32_t* v);
    extern void ce_le2cpu64s(uint64_t* v);

    extern void ce_cpu2be16s(uint16_t* v);
    extern void ce_cpu2be32s(uint32_t* v);
    extern void ce_cpu2be64s(uint64_t* v);

    extern void ce_be2cpu16s(uint16_t* v);
    extern void ce_be2cpu32s(uint32_t* v);
    extern void ce_be2cpu64s(uint64_t* v);
}

#endif
