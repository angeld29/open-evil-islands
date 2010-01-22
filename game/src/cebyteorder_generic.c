#include <stdbool.h>
#include <assert.h>

#include "cebyteorder.h"

static bool is_big_endian(void)
{
	static uint16_t pattern = 0xbabe;
	return 0xba == 0[(volatile unsigned char*)&pattern];
}

static void* swap_endian(void* p, int n)
{
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

uint16_t cecpu2le16(uint16_t v)
{
	return is_big_endian() ? swap_uint16(v) : v;
}

uint32_t cecpu2le32(uint32_t v)
{
	return is_big_endian() ? swap_uint32(v) : v;
}

uint64_t cecpu2le64(uint64_t v)
{
	return is_big_endian() ? swap_uint64(v) : v;
}

uint16_t cele2cpu16(uint16_t v)
{
	return is_big_endian() ? swap_uint16(v) : v;
}

uint32_t cele2cpu32(uint32_t v)
{
	return is_big_endian() ? swap_uint32(v) : v;
}

uint64_t cele2cpu64(uint64_t v)
{
	return is_big_endian() ? swap_uint64(v) : v;
}

uint16_t cecpu2be16(uint16_t v)
{
	return is_big_endian() ? v : swap_uint16(v);
}

uint32_t cecpu2be32(uint32_t v)
{
	return is_big_endian() ? v : swap_uint32(v);
}

uint64_t cecpu2be64(uint64_t v)
{
	return is_big_endian() ? v : swap_uint64(v);
}

uint16_t cebe2cpu16(uint16_t v)
{
	return is_big_endian() ? v : swap_uint16(v);
}

uint32_t cebe2cpu32(uint32_t v)
{
	return is_big_endian() ? v : swap_uint32(v);
}

uint64_t cebe2cpu64(uint64_t v)
{
	return is_big_endian() ? v : swap_uint64(v);
}

void cecpu2le16s(uint16_t* v)
{
	if (is_big_endian()) {
		swap_uint16s(v);
	}
}

void cecpu2le32s(uint32_t* v)
{
	if (is_big_endian()) {
		swap_uint32s(v);
	}
}

void cecpu2le64s(uint64_t* v)
{
	if (is_big_endian()) {
		swap_uint64s(v);
	}
}

void cele2cpu16s(uint16_t* v)
{
	if (is_big_endian()) {
		swap_uint16s(v);
	}
}

void cele2cpu32s(uint32_t* v)
{
	if (is_big_endian()) {
		swap_uint32s(v);
	}
}

void cele2cpu64s(uint64_t* v)
{
	if (is_big_endian()) {
		swap_uint64s(v);
	}
}

void cecpu2be16s(uint16_t* v)
{
	if (!is_big_endian()) {
		swap_uint16s(v);
	}
}

void cecpu2be32s(uint32_t* v)
{
	if (!is_big_endian()) {
		swap_uint32s(v);
	}
}

void cecpu2be64s(uint64_t* v)
{
	if (!is_big_endian()) {
		swap_uint64s(v);
	}
}

void cebe2cpu16s(uint16_t* v)
{
	if (!is_big_endian()) {
		swap_uint16s(v);
	}
}

void cebe2cpu32s(uint32_t* v)
{
	if (!is_big_endian()) {
		swap_uint32s(v);
	}
}

void cebe2cpu64s(uint64_t* v)
{
	if (!is_big_endian()) {
		swap_uint64s(v);
	}
}
