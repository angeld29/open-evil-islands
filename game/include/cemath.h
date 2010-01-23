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

extern const float CEPI;
extern const float CEPI2;
extern const float CEPI_DIV_2;
extern const float CEPI_DIV_4;
extern const float CEPI_INV;

extern const float CEEPS_E4;
extern const float CEEPS_E5;
extern const float CEEPS_E6;

extern bool cefisequal(float a, float b, float tolerance);
extern bool cefiszero(float a, float tolerance);

extern void cefswap(float* a, float* b);
extern float cefclamp(float v, float a, float b);

extern float celerp(float u, float a, float b);

extern float cedeg2rad(float d);
extern float cerad2deg(float r);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MATH_H */
