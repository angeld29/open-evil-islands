#include <math.h>

#include "cemath.h"

/**
 *  Based on http://www.c-faq.com/fp/fpequal.html.
*/
static float freldif(float a, float b)
{
	float c = fmax(fabs(a), fabs(b));
	return 0.0f == c ? 0.0f : fabs(a - b) / c;
}

bool fisequal(float a, float b, float tolerance)
{
	if (a == b || fabs(a - b) <= tolerance) {
		return true;
	}
	return freldif(a, b) <= tolerance;
}
