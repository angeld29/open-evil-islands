#include "celib.h"

#define CE_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CE_MAX(a, b) ((a) > (b) ? (a) : (b))
#define CE_CLAMP(v, a, b) ((v) < (a) ? (a) : ((v) > (b) ? (b) : (v)))

int cemin(int a, int b)
{
	return CE_MIN(a, b);
}

int cemax(int a, int b)
{
	return CE_MAX(a, b);
}

size_t cesmin(size_t a, size_t b)
{
	return CE_MIN(a, b);
}

size_t cesmax(size_t a, size_t b)
{
	return CE_MAX(a, b);
}

void ceswap(int* a, int* b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

int ceclamp(int v, int a, int b)
{
	return CE_CLAMP(v, a, b);
}

size_t cesclamp(size_t v, size_t a, size_t b)
{
	return CE_CLAMP(v, a, b);
}
