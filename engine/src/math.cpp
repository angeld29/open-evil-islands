/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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
#include <assert.h>

#include "math.hpp"

const float CE_PI = 3.14159265f;
const float CE_PI2 = 6.28318531f;
const float CE_PI_DIV_2 = 1.57079633f;
const float CE_PI_DIV_4 = 0.78539816f;
const float CE_PI_INV = 0.31830989f;

const float CE_EPS_E3 = 1e-3f;
const float CE_EPS_E4 = 1e-4f;
const float CE_EPS_E5 = 1e-5f;
const float CE_EPS_E6 = 1e-6f;

const float CE_DEG2RAD = 0.01745329f;
const float CE_RAD2DEG = 57.2957795f;

// based on http://www.c-faq.com/fp/fpequal.html
static inline float ce_reldif(float a, float b)
{
    float t = fmaxf(fabsf(a), fabsf(b));
    return 0.0f == t ? 0.0f : fabsf(a - b) / t;
}

bool ce_fisequal(float a, float b, float tolerance)
{
    return a == b || fabsf(a - b) <= tolerance || ce_reldif(a, b) <= tolerance;
}
