#include <math.h>

#include "cemath.h"

const float CEPI = 3.14159265f;
const float CEPI2 = 6.28318531f;
const float CEPI_DIV_2 = 1.57079633f;
const float CEPI_DIV_4 = 0.78539816f;
const float CEPI_INV = 0.31830989f;

const float CEEPS_E4 = 1e-4f;
const float CEEPS_E5 = 1e-5f;
const float CEEPS_E6 = 1e-6f;

static const float DEG2RAD = 0.01745329f;
static const float RAD2DEG = 57.2957795f;

/*
 *  Based on http://www.c-faq.com/fp/fpequal.html.
*/
static float reldif(float a, float b)
{
	float c = fmaxf(fabsf(a), fabsf(b));
	return 0.0f == c ? 0.0f : fabsf(a - b) / c;
}

bool cefisequal(float a, float b, float tolerance)
{
	return a == b || fabsf(a - b) <= tolerance || reldif(a, b) <= tolerance;
}

bool cefiszero(float a, float tolerance)
{
	return cefisequal(a, 0.0f, tolerance);
}

void cefswap(float* a, float* b)
{
	float t = *a;
	*a = *b;
	*b = t;
}

float cefclamp(float v, float a, float b)
{
	return v < a ? a : (v > b ? b : v);
}

float celerp(float u, float a, float b)
{
	return a + u * (b - a);
}

float cedeg2rad(float d)
{
	return DEG2RAD * d;
}

float cerad2deg(float r)
{
	return RAD2DEG * r;
}
