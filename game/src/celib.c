#include "celib.h"

#define CE_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CE_MAX(a, b) ((a) > (b) ? (a) : (b))
#define CE_CLAMP(t, a, b) ((t) < (a) ? (a) : ((t) > (b) ? (b) : (t)))

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

int clamp(int t, int a, int b)
{
	return CE_CLAMP(t, a, b);
}

size_t sclamp(size_t t, size_t a, size_t b)
{
	return CE_CLAMP(t, a, b);
}
