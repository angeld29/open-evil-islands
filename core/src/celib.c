#include "celib.h"

#define CE_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CE_MAX(a, b) ((a) > (b) ? (a) : (b))

int min(int a, int b)
{
	return CE_MIN(a, b);
}

int max(int a, int b)
{
	return CE_MAX(a, b);
}

size_t smin(size_t a, size_t b)
{
	return CE_MIN(a, b);
}

size_t smax(size_t a, size_t b)
{
	return CE_MAX(a, b);
}

void swap(int* a, int* b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

int clamp(int v, int a, int b)
{
	return v < a ? a : (v > b ? b : v);
}
