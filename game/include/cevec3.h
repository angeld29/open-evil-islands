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

#include "cequatfwd.h"
#include "cevec3fwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

struct ce_vec3 {
	float x, y, z;
};

extern const ce_vec3 CE_VEC3_ZERO;
extern const ce_vec3 CE_VEC3_UNIT_X;
extern const ce_vec3 CE_VEC3_UNIT_Y;
extern const ce_vec3 CE_VEC3_UNIT_Z;
extern const ce_vec3 CE_VEC3_UNIT_SCALE;
extern const ce_vec3 CE_VEC3_NEG_UNIT_X;
extern const ce_vec3 CE_VEC3_NEG_UNIT_Y;
extern const ce_vec3 CE_VEC3_NEG_UNIT_Z;
extern const ce_vec3 CE_VEC3_NEG_UNIT_SCALE;

extern ce_vec3* ce_vec3_zero(ce_vec3* r);
extern ce_vec3* ce_vec3_unit_x(ce_vec3* r);
extern ce_vec3* ce_vec3_unit_y(ce_vec3* r);
extern ce_vec3* ce_vec3_unit_z(ce_vec3* r);
extern ce_vec3* ce_vec3_unit_scale(ce_vec3* r);
extern ce_vec3* ce_vec3_neg_unit_x(ce_vec3* r);
extern ce_vec3* ce_vec3_neg_unit_y(ce_vec3* r);
extern ce_vec3* ce_vec3_neg_unit_z(ce_vec3* r);
extern ce_vec3* ce_vec3_neg_unit_scale(ce_vec3* r);

extern ce_vec3* ce_vec3_init(ce_vec3* r, float x, float y, float z);
extern ce_vec3* ce_vec3_init_array(ce_vec3* r, float* v);

extern ce_vec3* ce_vec3_copy(ce_vec3* r, const ce_vec3* a);

extern ce_vec3* ce_vec3_neg(ce_vec3* r, const ce_vec3* a);

extern ce_vec3* ce_vec3_add(ce_vec3* r, const ce_vec3* a, const ce_vec3* b);
extern ce_vec3* ce_vec3_sub(ce_vec3* r, const ce_vec3* a, const ce_vec3* b);
extern ce_vec3* ce_vec3_mul(ce_vec3* r, const ce_vec3* a, const ce_vec3* b);
extern ce_vec3* ce_vec3_div(ce_vec3* r, const ce_vec3* a, const ce_vec3* b);

extern ce_vec3* ce_vec3_scale(ce_vec3* r, const ce_vec3* a, float s);

extern float ce_vec3_abs(const ce_vec3* a);
extern float ce_vec3_abs2(const ce_vec3* a);

extern float ce_vec3_dist(const ce_vec3* a, const ce_vec3* b);
extern float ce_vec3_dist2(const ce_vec3* a, const ce_vec3* b);

extern ce_vec3* ce_vec3_normalise(ce_vec3* r, const ce_vec3* a);

extern float ce_vec3_dot(const ce_vec3* a, const ce_vec3* b);
extern float ce_vec3_absdot(const ce_vec3* a, const ce_vec3* b);

extern ce_vec3* ce_vec3_cross(ce_vec3* restrict r, const ce_vec3* a,
													const ce_vec3* b);

extern ce_vec3* ce_vec3_mid(ce_vec3* r, const ce_vec3* a, const ce_vec3* b);
extern ce_vec3* ce_vec3_rot(ce_vec3* r, const ce_vec3* a, const ce_quat* b);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VEC3_H */
