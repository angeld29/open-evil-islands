#include "cebitop.h"

uint8_t cebitset(uint8_t v, size_t p)
{
	return v | (1 << p);
}

uint8_t cebitclr(uint8_t v, size_t p)
{
	return v & ~(1 << p);
}

uint8_t cebitflp(uint8_t v, size_t p)
{
	return v ^ (1 << p);
}

bool cebittst(uint8_t v, size_t p)
{
	return v & (1 << p);
}
