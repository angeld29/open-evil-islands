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

static void* ce_swap_endian(char* restrict s, void* restrict p, int n)
{
	char* t = p;
	for (int i = 0, j = n - 1; i < n; ++i, --j) {
		s[i] = t[j];
	}
	return s;
}

#define CE_MAKE_SWAP(type) \
static type ## _t ce_swap_ ## type(type ## _t v) \
{ \
	char s[sizeof(type ## _t)]; \
	return *(type ## _t*)ce_swap_endian(s, &v, sizeof(type ## _t)); \
}

CE_MAKE_SWAP(uint16)
CE_MAKE_SWAP(uint32)
CE_MAKE_SWAP(uint64)

#define CE_MAKE_SWAP_S(type) \
static void ce_swap_ ## type ## s(type ## _t* v) \
{ \
	char s[sizeof(type ## _t)]; \
	*v = *(type ## _t*)ce_swap_endian(s, v, sizeof(type ## _t)); \
}

CE_MAKE_SWAP_S(uint16)
CE_MAKE_SWAP_S(uint32)
CE_MAKE_SWAP_S(uint64)

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
		ce_swap_uint16s(v);
	}
}

void ce_cpu2le32s(uint32_t* v)
{
	if (ce_is_big_endian()) {
		ce_swap_uint32s(v);
	}
}

void ce_cpu2le64s(uint64_t* v)
{
	if (ce_is_big_endian()) {
		ce_swap_uint64s(v);
	}
}

void ce_le2cpu16s(uint16_t* v)
{
	if (ce_is_big_endian()) {
		ce_swap_uint16s(v);
	}
}

void ce_le2cpu32s(uint32_t* v)
{
	if (ce_is_big_endian()) {
		ce_swap_uint32s(v);
	}
}

void ce_le2cpu64s(uint64_t* v)
{
	if (ce_is_big_endian()) {
		ce_swap_uint64s(v);
	}
}

void ce_cpu2be16s(uint16_t* v)
{
	if (!ce_is_big_endian()) {
		ce_swap_uint16s(v);
	}
}

void ce_cpu2be32s(uint32_t* v)
{
	if (!ce_is_big_endian()) {
		ce_swap_uint32s(v);
	}
}

void ce_cpu2be64s(uint64_t* v)
{
	if (!ce_is_big_endian()) {
		ce_swap_uint64s(v);
	}
}

void ce_be2cpu16s(uint16_t* v)
{
	if (!ce_is_big_endian()) {
		ce_swap_uint16s(v);
	}
}

void ce_be2cpu32s(uint32_t* v)
{
	if (!ce_is_big_endian()) {
		ce_swap_uint32s(v);
	}
}

void ce_be2cpu64s(uint64_t* v)
{
	if (!ce_is_big_endian()) {
		ce_swap_uint64s(v);
	}
}
