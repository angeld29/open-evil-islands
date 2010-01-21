#include <math.h>

#include "cemath.h"

const float PI = 3.14159265f;
const float PI2 = 6.28318531f;
const float PI_DIV_2 = 1.57079633f;
const float PI_DIV_4 = 0.78539816f;
const float PI_INV = 0.31830989f;

const float EPS_E4 = 1e-4f;
const float EPS_E5 = 1e-5f;
const float EPS_E6 = 1e-6f;

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

bool fisequal(float a, float b, float tolerance)
{
	return a == b || fabsf(a - b) <= tolerance || reldif(a, b) <= tolerance;
}

bool fiszero(float a, float tolerance)
{
	return fisequal(a, 0.0f, tolerance);
}

void fswap(float* a, float* b)
{
	float t = *a;
	*a = *b;
	*b = t;
}

float fclamp(float v, float a, float b)
{
	return v < a ? a : (v > b ? b : v);
}

float flerp(float u, float a, float b)
{
	return a + u * (b - a);
}

float deg2rad(float d)
{
	return DEG2RAD * d;
}

float rad2deg(float r)
{
	return RAD2DEG * r;
}
