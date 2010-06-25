/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#ifndef CE_QUAT_H
#define CE_QUAT_H

#ifdef __cplusplus
extern "C" {
#endif

struct ce_vec3;

typedef struct ce_quat {
	float w, x, y, z;
} ce_quat;

extern const ce_quat CE_QUAT_ZERO;
extern const ce_quat CE_QUAT_IDENTITY;

extern ce_quat* ce_quat_init(ce_quat* quat, float w, float x, float y, float z);
extern ce_quat* ce_quat_init_array(ce_quat* quat, const float* array);
extern ce_quat* ce_quat_init_polar(ce_quat* quat, float angle,
									const struct ce_vec3* axis);

extern ce_quat* ce_quat_init_zero(ce_quat* quat);
extern ce_quat* ce_quat_init_identity(ce_quat* quat);

extern ce_quat* ce_quat_copy(ce_quat* quat, const ce_quat* other);

extern ce_quat* ce_quat_neg(ce_quat* quat, const ce_quat* other);
extern ce_quat* ce_quat_conj(ce_quat* quat, const ce_quat* other);

extern ce_quat* ce_quat_add(ce_quat* quat, const ce_quat* lhs,
											const ce_quat* rhs);
extern ce_quat* ce_quat_sub(ce_quat* quat, const ce_quat* lhs,
											const ce_quat* rhs);
extern ce_quat* ce_quat_mul(ce_quat* restrict quat, const ce_quat* lhs,
													const ce_quat* rhs);

extern ce_quat* ce_quat_scale(ce_quat* quat, float s, const ce_quat* other);

extern float ce_quat_len(const ce_quat* quat);
extern float ce_quat_len2(const ce_quat* quat);

extern float ce_quat_arg(const ce_quat* quat);

extern ce_quat* ce_quat_norm(ce_quat* quat, const ce_quat* other);
extern ce_quat* ce_quat_inv(ce_quat* quat, const ce_quat* other);

extern float ce_quat_dot(const ce_quat* lhs, const ce_quat* rhs);

extern float ce_quat_to_polar(const ce_quat* quat, struct ce_vec3* axis);
extern void ce_quat_to_axes(const ce_quat* quat, struct ce_vec3* xaxis,
												struct ce_vec3* yaxis,
												struct ce_vec3* zaxis);

// spherical linear interpolation
extern ce_quat* ce_quat_slerp(ce_quat* quat, float u, const ce_quat* lhs,
														const ce_quat* rhs);

#ifdef __cplusplus
}
#endif

#endif /* CE_QUAT_H */
