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

#include "vec3fwd.h"
#include "quatdef.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern const quat QUAT_ZERO;
extern const quat QUAT_IDENTITY;

extern quat* quat_zero(quat* r);
extern quat* quat_identity(quat* r);

extern quat* quat_init(float w, float x, float y, float z, quat* r);
extern quat* quat_init_polar(float theta, const vec3* axis, quat* r);
extern quat* quat_copy(const quat* a, quat* r);

extern quat* quat_neg(const quat* a, quat* r);
extern quat* quat_conj(const quat* a, quat* r);

extern quat* quat_add(const quat* a, const quat* b, quat* r);
extern quat* quat_sub(const quat* a, const quat* b, quat* r);
extern quat* quat_mul(const quat* a, const quat* b, quat* restrict r);

extern float quat_abs(const quat* a);
extern float quat_abs2(const quat* a);

extern float quat_arg(const quat* a);

extern quat* quat_normalise(const quat* a, quat* r);
extern quat* quat_inverse(const quat* a, quat* r);

extern float quat_dot(const quat* a, const quat* b);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_QUAT_H */
