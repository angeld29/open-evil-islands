#include <math.h>

#include "cemath.h"

/**
 *  Based on http://www.c-faq.com/fp/fpequal.html.
*/
static float freldiff(float a, float b)
{
	float c = fmaxf(fabsf(a), fabsf(b));
	return 0.0f == c ? 0.0f : fabsf(a - b) / c;
}

bool fisequalf(float a, float b, float tolerance)
{
	return a == b || fabsf(a - b) <= tolerance || freldiff(a, b) <= tolerance;
}
