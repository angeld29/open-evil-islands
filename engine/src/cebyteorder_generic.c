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

#include "cebyteorder.h"

static uint16_t ce_swap_uint16(uint16_t v)
{
	return v << 8 | v >> 8;
}

static uint32_t ce_swap_uint32(uint32_t v)
{
	return v << 24 | v >> 24 |
		(v & (uint32_t)0x0000ff00UL) << 8 |
		(v & (uint32_t)0x00ff0000UL) >> 8;
}

static uint64_t ce_swap_uint64(uint64_t v)
{
	return v << 56 | v >> 56 |
		(v & (uint64_t)0x000000000000ff00ULL) << 40 |
		(v & (uint64_t)0x0000000000ff0000ULL) << 24 |
		(v & (uint64_t)0x00000000ff000000ULL) << 8  |
		(v & (uint64_t)0x000000ff00000000ULL) >> 8  |
		(v & (uint64_t)0x0000ff0000000000ULL) >> 24 |
		(v & (uint64_t)0x00ff000000000000ULL) >> 40;
}

uint16_t ce_cpu2le16(uint16_t v)
{
	return ce_is_big_endian() ? ce_swap_uint16(v) : v;
}

uint32_t ce_cpu2le32(uint32_t v)
{
	return ce_is_big_endian() ? ce_swap_uint32(v) : v;
}

uint64_t ce_cpu2le64(uint64_t v)
{
	return ce_is_big_endian() ? ce_swap_uint64(v) : v;
}

uint16_t ce_le2cpu16(uint16_t v)
{
	return ce_is_big_endian() ? ce_swap_uint16(v) : v;
}

uint32_t ce_le2cpu32(uint32_t v)
{
	return ce_is_big_endian() ? ce_swap_uint32(v) : v;
}

uint64_t ce_le2cpu64(uint64_t v)
{
	return ce_is_big_endian() ? ce_swap_uint64(v) : v;
}

uint16_t ce_cpu2be16(uint16_t v)
{
	return ce_is_big_endian() ? v : ce_swap_uint16(v);
}

uint32_t ce_cpu2be32(uint32_t v)
{
	return ce_is_big_endian() ? v : ce_swap_uint32(v);
}

uint64_t ce_cpu2be64(uint64_t v)
{
	return ce_is_big_endian() ? v : ce_swap_uint64(v);
}

uint16_t ce_be2cpu16(uint16_t v)
{
	return ce_is_big_endian() ? v : ce_swap_uint16(v);
}

uint32_t ce_be2cpu32(uint32_t v)
{
	return ce_is_big_endian() ? v : ce_swap_uint32(v);
}

uint64_t ce_be2cpu64(uint64_t v)
{
	return ce_is_big_endian() ? v : ce_swap_uint64(v);
}

void ce_cpu2le16s(uint16_t* v)
{
	if (ce_is_big_endian()) {
		*v = ce_swap_uint16(*v);
	}
}

void ce_cpu2le32s(uint32_t* v)
{
	if (ce_is_big_endian()) {
		*v = ce_swap_uint32(*v);
	}
}

void ce_cpu2le64s(uint64_t* v)
{
	if (ce_is_big_endian()) {
		*v = ce_swap_uint64(*v);
	}
}

void ce_le2cpu16s(uint16_t* v)
{
	if (ce_is_big_endian()) {
		*v = ce_swap_uint16(*v);
	}
}

void ce_le2cpu32s(uint32_t* v)
{
	if (ce_is_big_endian()) {
		*v = ce_swap_uint32(*v);
	}
}

void ce_le2cpu64s(uint64_t* v)
{
	if (ce_is_big_endian()) {
		*v = ce_swap_uint64(*v);
	}
}

void ce_cpu2be16s(uint16_t* v)
{
	if (!ce_is_big_endian()) {
		*v = ce_swap_uint16(*v);
	}
}

void ce_cpu2be32s(uint32_t* v)
{
	if (!ce_is_big_endian()) {
		*v = ce_swap_uint32(*v);
	}
}

void ce_cpu2be64s(uint64_t* v)
{
	if (!ce_is_big_endian()) {
		*v = ce_swap_uint64(*v);
	}
}

void ce_be2cpu16s(uint16_t* v)
{
	if (!ce_is_big_endian()) {
		*v = ce_swap_uint16(*v);
	}
}

void ce_be2cpu32s(uint32_t* v)
{
	if (!ce_is_big_endian()) {
		*v = ce_swap_uint32(*v);
	}
}

void ce_be2cpu64s(uint64_t* v)
{
	if (!ce_is_big_endian()) {
		*v = ce_swap_uint64(*v);
	}
}
