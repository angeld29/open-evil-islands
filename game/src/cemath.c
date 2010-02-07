/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <math.h>

#include "cemath.h"

const float CE_PI = 3.14159265f;
const float CE_PI2 = 6.28318531f;
const float CE_PI_DIV_2 = 1.57079633f;
const float CE_PI_DIV_4 = 0.78539816f;
const float CE_PI_INV = 0.31830989f;

const float CE_EPS_E4 = 1e-4f;
const float CE_EPS_E5 = 1e-5f;
const float CE_EPS_E6 = 1e-6f;

static const float CE_DEG2RAD = 0.01745329f;
static const float CE_RAD2DEG = 57.2957795f;

/*
 *  Based on http://www.c-faq.com/fp/fpequal.html.
*/
static float reldif(float a, float b)
{
	float c = fmaxf(fabsf(a), fabsf(b));
	return 0.0f == c ? 0.0f : fabsf(a - b) / c;
}

bool ce_fisequal(float a, float b, float tolerance)
{
	return a == b || fabsf(a - b) <= tolerance || reldif(a, b) <= tolerance;
}

bool ce_fiszero(float a, float tolerance)
{
	return ce_fisequal(a, 0.0f, tolerance);
}

void ce_fswap(float* a, float* b)
{
	float t = *a;
	*a = *b;
	*b = t;
}

float ce_fclamp(float v, float a, float b)
{
	return v < a ? a : (v > b ? b : v);
}

float ce_lerp(float u, float a, float b)
{
	return a + u * (b - a);
}

float ce_deg2rad(float angle)
{
	return CE_DEG2RAD * angle;
}

float ce_rad2deg(float angle)
{
	return CE_RAD2DEG * angle;
}
