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

#if __GNUC__ == 4 && __GNUC_MINOR__ == 2
#include <linux/types.h>
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;
#endif

#include <asm/byteorder.h>

#include "byteorder.hpp"

uint16_t ce_cpu2le16(uint16_t v)
{
    return __cpu_to_le16(v);
}

uint32_t ce_cpu2le32(uint32_t v)
{
    return __cpu_to_le32(v);
}

uint64_t ce_cpu2le64(uint64_t v)
{
    return __cpu_to_le64(v);
}

uint16_t ce_le2cpu16(uint16_t v)
{
    return __le16_to_cpu(v);
}

uint32_t ce_le2cpu32(uint32_t v)
{
    return __le32_to_cpu(v);
}

uint64_t ce_le2cpu64(uint64_t v)
{
    return __le64_to_cpu(v);
}

uint16_t ce_cpu2be16(uint16_t v)
{
    return __cpu_to_be16(v);
}

uint32_t ce_cpu2be32(uint32_t v)
{
    return __cpu_to_be32(v);
}

uint64_t ce_cpu2be64(uint64_t v)
{
    return __cpu_to_be64(v);
}

uint16_t ce_be2cpu16(uint16_t v)
{
    return __be16_to_cpu(v);
}

uint32_t ce_be2cpu32(uint32_t v)
{
    return __be32_to_cpu(v);
}

uint64_t ce_be2cpu64(uint64_t v)
{
    return __be64_to_cpu(v);
}

void ce_cpu2le16s(uint16_t* v)
{
    __cpu_to_le16s((__u16*)v);
}

void ce_cpu2le32s(uint32_t* v)
{
    __cpu_to_le32s((__u32*)v);
}

void ce_cpu2le64s(uint64_t* v)
{
    __cpu_to_le64s((__u64*)v);
}

void ce_le2cpu16s(uint16_t* v)
{
    __le16_to_cpus((__u16*)v);
}

void ce_le2cpu32s(uint32_t* v)
{
    __le32_to_cpus((__u32*)v);
}

void ce_le2cpu64s(uint64_t* v)
{
    __le64_to_cpus((__u64*)v);
}

void ce_cpu2be16s(uint16_t* v)
{
    __cpu_to_be16s((__u16*)v);
}

void ce_cpu2be32s(uint32_t* v)
{
    __cpu_to_be32s((__u32*)v);
}

void ce_cpu2be64s(uint64_t* v)
{
    __cpu_to_be64s((__u64*)v);
}

void ce_be2cpu16s(uint16_t* v)
{
    __be16_to_cpus((__u16*)v);
}

void ce_be2cpu32s(uint32_t* v)
{
    __be32_to_cpus((__u32*)v);
}

void ce_be2cpu64s(uint64_t* v)
{
    __be64_to_cpus((__u64*)v);
}
