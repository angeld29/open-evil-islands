#include "bitop.h"

uint8_t bitset(uint8_t v, size_t p)
{
	return v | (1 << p);
}

uint8_t bitclr(uint8_t v, size_t p)
{
	return v & ~(1 << p);
}

uint8_t bitflp(uint8_t v, size_t p)
{
	return v ^ (1 << p);
}

bool bittst(uint8_t v, size_t p)
{
	return v & (1 << p);
}
