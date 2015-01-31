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

#ifndef CE_MATH_HPP
#define CE_MATH_HPP

#include <stdbool.h>

extern const float CE_PI;
extern const float CE_PI2;
extern const float CE_PI_DIV_2;
extern const float CE_PI_DIV_4;
extern const float CE_PI_INV;

extern const float CE_EPS_E3;
extern const float CE_EPS_E4;
extern const float CE_EPS_E5;
extern const float CE_EPS_E6;

extern const float CE_DEG2RAD;
extern const float CE_RAD2DEG;

extern bool ce_fisequal(float a, float b, float tolerance);

static inline bool ce_fiszero(float a, float tolerance)
{
    return ce_fisequal(a, 0.0f, tolerance);
}

static inline float ce_lerp(float u, float a, float b)
{
    return a + u * (b - a);
}

static inline float ce_deg2rad(float angle)
{
    return CE_DEG2RAD * angle;
}

static inline float ce_rad2deg(float angle)
{
    return CE_RAD2DEG * angle;
}

#endif /* CE_MATH_HPP */
