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

#ifndef CE_VEC3_H
#define CE_VEC3_H

#include "quatfwd.h"
#include "vec3fwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

struct vec3 {
	float x, y, z;
};

extern const vec3 VEC3_ZERO;
extern const vec3 VEC3_UNIT_X;
extern const vec3 VEC3_UNIT_Y;
extern const vec3 VEC3_UNIT_Z;
extern const vec3 VEC3_UNIT_SCALE;
extern const vec3 VEC3_NEG_UNIT_X;
extern const vec3 VEC3_NEG_UNIT_Y;
extern const vec3 VEC3_NEG_UNIT_Z;
extern const vec3 VEC3_NEG_UNIT_SCALE;

extern vec3* vec3_zero(vec3* r);
extern vec3* vec3_unit_x(vec3* r);
extern vec3* vec3_unit_y(vec3* r);
extern vec3* vec3_unit_z(vec3* r);
extern vec3* vec3_unit_scale(vec3* r);
extern vec3* vec3_neg_unit_x(vec3* r);
extern vec3* vec3_neg_unit_y(vec3* r);
extern vec3* vec3_neg_unit_z(vec3* r);
extern vec3* vec3_neg_unit_scale(vec3* r);

extern vec3* vec3_init(float x, float y, float z, vec3* r);
extern vec3* vec3_copy(const vec3* a, vec3* r);

extern vec3* vec3_neg(const vec3* a, vec3* r);

extern vec3* vec3_add(const vec3* a, const vec3* b, vec3* r);
extern vec3* vec3_sub(const vec3* a, const vec3* b, vec3* r);
extern vec3* vec3_mul(const vec3* a, const vec3* b, vec3* r);
extern vec3* vec3_div(const vec3* a, const vec3* b, vec3* r);

extern vec3* vec3_scale(const vec3* a, float s, vec3* r);

extern float vec3_abs(const vec3* a);
extern float vec3_abs2(const vec3* a);

extern float vec3_dist(const vec3* a, const vec3* b);
extern float vec3_dist2(const vec3* a, const vec3* b);

extern vec3* vec3_normalise(const vec3* a, vec3* r);

extern float vec3_dot(const vec3* a, const vec3* b);
extern float vec3_absdot(const vec3* a, const vec3* b);

extern vec3* vec3_cross(const vec3* a, const vec3* b, vec3* restrict r);

extern vec3* vec3_mid(const vec3* a, const vec3* b, vec3* r);
extern vec3* vec3_rot(const vec3* a, const quat* b, vec3* r);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VEC3_H */
