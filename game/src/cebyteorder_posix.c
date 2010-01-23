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

#if __GNUC__ == 4 && __GNUC_MINOR__ == 2
#include <assert.h>
// Hack for gcc 4.2
typedef unsigned long long __le64;
typedef unsigned long long __be64;
#endif
#include <asm/byteorder.h>

#include "cebyteorder.h"

uint16_t cecpu2le16(uint16_t v)
{
	return __cpu_to_le16(v);
}

uint32_t cecpu2le32(uint32_t v)
{
	return __cpu_to_le32(v);
}

uint64_t cecpu2le64(uint64_t v)
{
	return __cpu_to_le64(v);
}

uint16_t cele2cpu16(uint16_t v)
{
	return __le16_to_cpu(v);
}

uint32_t cele2cpu32(uint32_t v)
{
	return __le32_to_cpu(v);
}

uint64_t cele2cpu64(uint64_t v)
{
	return __le64_to_cpu(v);
}

uint16_t cecpu2be16(uint16_t v)
{
	return __cpu_to_be16(v);
}

uint32_t cecpu2be32(uint32_t v)
{
	return __cpu_to_be32(v);
}

uint64_t cecpu2be64(uint64_t v)
{
	return __cpu_to_be64(v);
}

uint16_t cebe2cpu16(uint16_t v)
{
	return __be16_to_cpu(v);
}

uint32_t cebe2cpu32(uint32_t v)
{
	return __be32_to_cpu(v);
}

uint64_t cebe2cpu64(uint64_t v)
{
	return __be64_to_cpu(v);
}

void cecpu2le16s(uint16_t* v)
{
	__cpu_to_le16s(v);
}

void cecpu2le32s(uint32_t* v)
{
	__cpu_to_le32s(v);
}

void cecpu2le64s(uint64_t* v)
{
	__cpu_to_le64s(v);
}

void cele2cpu16s(uint16_t* v)
{
	__le16_to_cpus(v);
}

void cele2cpu32s(uint32_t* v)
{
	__le32_to_cpus(v);
}

void cele2cpu64s(uint64_t* v)
{
	__le64_to_cpus(v);
}

void cecpu2be16s(uint16_t* v)
{
	__cpu_to_be16s(v);
}

void cecpu2be32s(uint32_t* v)
{
	__cpu_to_be32s(v);
}

void cecpu2be64s(uint64_t* v)
{
#if __GNUC__ == 4 && __GNUC_MINOR__ == 2
	assert(0);
#else
	__cpu_to_be64s(v);
#endif
}

void cebe2cpu16s(uint16_t* v)
{
	__be16_to_cpus(v);
}

void cebe2cpu32s(uint32_t* v)
{
	__be32_to_cpus(v);
}

void cebe2cpu64s(uint64_t* v)
{
#if __GNUC__ == 4 && __GNUC_MINOR__ == 2
	assert(0);
#else
	__be64_to_cpus(v);
#endif
}
