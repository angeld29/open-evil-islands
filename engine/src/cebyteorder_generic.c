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

#include <assert.h>

#include "cebyteorder.h"

static void* swap_endian(void* p, int n)
{
	// FIXME: threads?
	static char s[8];
	char* t = p;
	switch (n) {
	case 2:
		s[0] = t[1];
		s[1] = t[0];
		break;
	case 4:
		s[0] = t[3];
		s[1] = t[2];
		s[2] = t[1];
		s[3] = t[0];
		break;
	case 8:
		s[0] = t[7];
		s[1] = t[6];
		s[2] = t[5];
		s[3] = t[4];
		s[4] = t[3];
		s[5] = t[2];
		s[6] = t[1];
		s[7] = t[0];
		break;
	default:
		assert(false);
	}
	return s;
}

static uint16_t swap_uint16(uint16_t v)
{
	return *(uint16_t*)swap_endian(&v, sizeof(uint16_t));
}

static uint32_t swap_uint32(uint32_t v)
{
	return *(uint32_t*)swap_endian(&v, sizeof(uint32_t));
}

static uint64_t swap_uint64(uint64_t v)
{
	return *(uint64_t*)swap_endian(&v, sizeof(uint64_t));
}

static void swap_uint16s(uint16_t* v)
{
	*v = *(uint16_t*)swap_endian(v, sizeof(uint16_t));
}

static void swap_uint32s(uint32_t* v)
{
	*v = *(uint32_t*)swap_endian(v, sizeof(uint32_t));
}

static void swap_uint64s(uint64_t* v)
{
	*v = *(uint64_t*)swap_endian(v, sizeof(uint64_t));
}

uint16_t ce_cpu2le16(uint16_t v)
{
	return ce_is_big_endian() ? swap_uint16(v) : v;
}

uint32_t ce_cpu2le32(uint32_t v)
{
	return ce_is_big_endian() ? swap_uint32(v) : v;
}

uint64_t ce_cpu2le64(uint64_t v)
{
	return ce_is_big_endian() ? swap_uint64(v) : v;
}

uint16_t ce_le2cpu16(uint16_t v)
{
	return ce_is_big_endian() ? swap_uint16(v) : v;
}

uint32_t ce_le2cpu32(uint32_t v)
{
	return ce_is_big_endian() ? swap_uint32(v) : v;
}

uint64_t ce_le2cpu64(uint64_t v)
{
	return ce_is_big_endian() ? swap_uint64(v) : v;
}

uint16_t ce_cpu2be16(uint16_t v)
{
	return ce_is_big_endian() ? v : swap_uint16(v);
}

uint32_t ce_cpu2be32(uint32_t v)
{
	return ce_is_big_endian() ? v : swap_uint32(v);
}

uint64_t ce_cpu2be64(uint64_t v)
{
	return ce_is_big_endian() ? v : swap_uint64(v);
}

uint16_t ce_be2cpu16(uint16_t v)
{
	return ce_is_big_endian() ? v : swap_uint16(v);
}

uint32_t ce_be2cpu32(uint32_t v)
{
	return ce_is_big_endian() ? v : swap_uint32(v);
}

uint64_t ce_be2cpu64(uint64_t v)
{
	return ce_is_big_endian() ? v : swap_uint64(v);
}

void ce_cpu2le16s(uint16_t* v)
{
	if (ce_is_big_endian()) {
		swap_uint16s(v);
	}
}

void ce_cpu2le32s(uint32_t* v)
{
	if (ce_is_big_endian()) {
		swap_uint32s(v);
	}
}

void ce_cpu2le64s(uint64_t* v)
{
	if (ce_is_big_endian()) {
		swap_uint64s(v);
	}
}

void ce_le2cpu16s(uint16_t* v)
{
	if (ce_is_big_endian()) {
		swap_uint16s(v);
	}
}

void ce_le2cpu32s(uint32_t* v)
{
	if (ce_is_big_endian()) {
		swap_uint32s(v);
	}
}

void ce_le2cpu64s(uint64_t* v)
{
	if (ce_is_big_endian()) {
		swap_uint64s(v);
	}
}

void ce_cpu2be16s(uint16_t* v)
{
	if (!ce_is_big_endian()) {
		swap_uint16s(v);
	}
}

void ce_cpu2be32s(uint32_t* v)
{
	if (!ce_is_big_endian()) {
		swap_uint32s(v);
	}
}

void ce_cpu2be64s(uint64_t* v)
{
	if (!ce_is_big_endian()) {
		swap_uint64s(v);
	}
}

void ce_be2cpu16s(uint16_t* v)
{
	if (!ce_is_big_endian()) {
		swap_uint16s(v);
	}
}

void ce_be2cpu32s(uint32_t* v)
{
	if (!ce_is_big_endian()) {
		swap_uint32s(v);
	}
}

void ce_be2cpu64s(uint64_t* v)
{
	if (!ce_is_big_endian()) {
		swap_uint64s(v);
	}
}
