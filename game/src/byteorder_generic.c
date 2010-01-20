#include <stdbool.h>
#include <assert.h>

#include "byteorder.h"

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

uint16_t cpu2le16(uint16_t v)
{
	return is_big_endian() ? swap_uint16(v) : v;
}

uint32_t cpu2le32(uint32_t v)
{
	return is_big_endian() ? swap_uint32(v) : v;
}

uint64_t cpu2le64(uint64_t v)
{
	return is_big_endian() ? swap_uint64(v) : v;
}

uint16_t le2cpu16(uint16_t v)
{
	return is_big_endian() ? swap_uint16(v) : v;
}

uint32_t le2cpu32(uint32_t v)
{
	return is_big_endian() ? swap_uint32(v) : v;
}

uint64_t le2cpu64(uint64_t v)
{
	return is_big_endian() ? swap_uint64(v) : v;
}

uint16_t cpu2be16(uint16_t v)
{
	return is_big_endian() ? v : swap_uint16(v);
}

uint32_t cpu2be32(uint32_t v)
{
	return is_big_endian() ? v : swap_uint32(v);
}

uint64_t cpu2be64(uint64_t v)
{
	return is_big_endian() ? v : swap_uint64(v);
}

uint16_t be2cpu16(uint16_t v)
{
	return is_big_endian() ? v : swap_uint16(v);
}

uint32_t be2cpu32(uint32_t v)
{
	return is_big_endian() ? v : swap_uint32(v);
}

uint64_t be2cpu64(uint64_t v)
{
	return is_big_endian() ? v : swap_uint64(v);
}

void cpu2le16s(uint16_t* v)
{
	if (is_big_endian()) {
		swap_uint16s(v);
	}
}

void cpu2le32s(uint32_t* v)
{
	if (is_big_endian()) {
		swap_uint32s(v);
	}
}

void cpu2le64s(uint64_t* v)
{
	if (is_big_endian()) {
		swap_uint64s(v);
	}
}

void le2cpu16s(uint16_t* v)
{
	if (is_big_endian()) {
		swap_uint16s(v);
	}
}

void le2cpu32s(uint32_t* v)
{
	if (is_big_endian()) {
		swap_uint32s(v);
	}
}

void le2cpu64s(uint64_t* v)
{
	if (is_big_endian()) {
		swap_uint64s(v);
	}
}

void cpu2be16s(uint16_t* v)
{
	if (!is_big_endian()) {
		swap_uint16s(v);
	}
}

void cpu2be32s(uint32_t* v)
{
	if (!is_big_endian()) {
		swap_uint32s(v);
	}
}

void cpu2be64s(uint64_t* v)
{
	if (!is_big_endian()) {
		swap_uint64s(v);
	}
}

void be2cpu16s(uint16_t* v)
{
	if (!is_big_endian()) {
		swap_uint16s(v);
	}
}

void be2cpu32s(uint32_t* v)
{
	if (!is_big_endian()) {
		swap_uint32s(v);
	}
}

void be2cpu64s(uint64_t* v)
{
	if (!is_big_endian()) {
		swap_uint64s(v);
	}
}
