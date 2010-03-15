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

extern ce_vec3* ce_vec3_init(ce_vec3* vec, float x, float y, float z);
extern ce_vec3* ce_vec3_init_array(ce_vec3* vec, const float* array);

extern ce_vec3* ce_vec3_init_zero(ce_vec3* vec);
extern ce_vec3* ce_vec3_init_unit_x(ce_vec3* vec);
extern ce_vec3* ce_vec3_init_unit_y(ce_vec3* vec);
extern ce_vec3* ce_vec3_init_unit_z(ce_vec3* vec);
extern ce_vec3* ce_vec3_init_unit_scale(ce_vec3* vec);
extern ce_vec3* ce_vec3_init_neg_unit_x(ce_vec3* vec);
extern ce_vec3* ce_vec3_init_neg_unit_y(ce_vec3* vec);
extern ce_vec3* ce_vec3_init_neg_unit_z(ce_vec3* vec);
extern ce_vec3* ce_vec3_init_neg_unit_scale(ce_vec3* vec);

extern ce_vec3* ce_vec3_copy(ce_vec3* vec, const ce_vec3* other);

extern ce_vec3* ce_vec3_neg(ce_vec3* vec, const ce_vec3* other);

extern ce_vec3* ce_vec3_add(ce_vec3* vec, const ce_vec3* lhs,
											const ce_vec3* rhs);
extern ce_vec3* ce_vec3_sub(ce_vec3* vec, const ce_vec3* lhs,
											const ce_vec3* rhs);
extern ce_vec3* ce_vec3_mul(ce_vec3* vec, const ce_vec3* lhs,
											const ce_vec3* rhs);
extern ce_vec3* ce_vec3_div(ce_vec3* vec, const ce_vec3* lhs,
											const ce_vec3* rhs);

extern ce_vec3* ce_vec3_scale(ce_vec3* vec, float s, const ce_vec3* other);

extern float ce_vec3_len(const ce_vec3* vec);
extern float ce_vec3_len2(const ce_vec3* vec);

extern float ce_vec3_dist(const ce_vec3* lhs, const ce_vec3* rhs);
extern float ce_vec3_dist2(const ce_vec3* lhs, const ce_vec3* rhs);

extern ce_vec3* ce_vec3_normalise(ce_vec3* vec, const ce_vec3* other);

extern float ce_vec3_dot(const ce_vec3* lhs, const ce_vec3* rhs);
extern float ce_vec3_absdot(const ce_vec3* lhs, const ce_vec3* rhs);

extern ce_vec3* ce_vec3_cross(ce_vec3* restrict vec, const ce_vec3* lhs,
													const ce_vec3* rhs);

extern ce_vec3* ce_vec3_mid(ce_vec3* vec, const ce_vec3* lhs,
											const ce_vec3* rhs);
extern ce_vec3* ce_vec3_rot(ce_vec3* vec, const ce_vec3* other,
											const ce_quat* quat);

extern ce_vec3* ce_vec3_lerp(ce_vec3* vec, float u, const ce_vec3* lhs,
													const ce_vec3* rhs);

extern ce_vec3* ce_vec3_floor(ce_vec3* vec, const ce_vec3* lhs,
											const ce_vec3* rhs);
extern ce_vec3* ce_vec3_ceil(ce_vec3* vec, const ce_vec3* lhs,
											const ce_vec3* rhs);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VEC3_H */
