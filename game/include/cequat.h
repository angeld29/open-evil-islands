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

#ifndef CE_QUAT_H
#define CE_QUAT_H

#include "cevec3fwd.h"
#include "cequatfwd.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

struct ce_quat {
	float w, x, y, z;
};

extern const ce_quat CE_QUAT_ZERO;
extern const ce_quat CE_QUAT_IDENTITY;

extern ce_quat* ce_quat_zero(ce_quat* r);
extern ce_quat* ce_quat_identity(ce_quat* r);

extern ce_quat* ce_quat_init(ce_quat* r, float w, float x, float y, float z);
extern ce_quat* ce_quat_init_vector(ce_quat* r, float* v);
extern ce_quat* ce_quat_init_polar(ce_quat* r, float theta, const ce_vec3* axis);
extern ce_quat* ce_quat_copy(ce_quat* r, const ce_quat* a);

extern ce_quat* ce_quat_neg(ce_quat* r, const ce_quat* a);
extern ce_quat* ce_quat_conj(ce_quat* r, const ce_quat* a);

extern ce_quat* ce_quat_add(ce_quat* r, const ce_quat* a, const ce_quat* b);
extern ce_quat* ce_quat_sub(ce_quat* r, const ce_quat* a, const ce_quat* b);
extern ce_quat* ce_quat_mul(ce_quat* restrict r, const ce_quat* a,
												const ce_quat* b);

extern float ce_quat_abs(const ce_quat* a);
extern float ce_quat_abs2(const ce_quat* a);

extern float ce_quat_arg(const ce_quat* a);

extern ce_quat* ce_quat_normalise(ce_quat* r, const ce_quat* a);
extern ce_quat* ce_quat_inverse(ce_quat* r, const ce_quat* a);

extern float ce_quat_dot(const ce_quat* a, const ce_quat* b);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_QUAT_H */
