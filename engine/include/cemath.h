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

#ifndef CE_MATH_H
#define CE_MATH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern const float CE_PI;
extern const float CE_PI2;
extern const float CE_PI_DIV_2;
extern const float CE_PI_DIV_4;
extern const float CE_PI_INV;

extern const float CE_EPS_E3;
extern const float CE_EPS_E4;
extern const float CE_EPS_E5;
extern const float CE_EPS_E6;

extern bool ce_fisequal(float a, float b, float tolerance);
extern bool ce_fiszero(float a, float tolerance);

extern void ce_fswap(float* a, float* b);
extern float ce_fclamp(float v, float a, float b);

extern float ce_lerp(float u, float a, float b);

extern float ce_deg2rad(float angle);
extern float ce_rad2deg(float angle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MATH_H */
