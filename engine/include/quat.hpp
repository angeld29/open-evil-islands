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

#ifndef CE_QUAT_HPP
#define CE_QUAT_HPP

#include <math.h>

struct ce_vec3;

typedef struct ce_quat {
    float w, x, y, z;
} ce_quat;

extern const ce_quat CE_QUAT_ZERO;
extern const ce_quat CE_QUAT_IDENTITY;

extern ce_quat* ce_quat_init_polar(ce_quat* quat, float angle, const struct ce_vec3* axis);

extern float ce_quat_to_polar(const ce_quat* quat, struct ce_vec3* axis);
extern void ce_quat_to_axes(const ce_quat* quat, struct ce_vec3* xaxis,
                                                struct ce_vec3* yaxis,
                                                struct ce_vec3* zaxis);

// spherical linear interpolation
extern ce_quat* ce_quat_slerp(ce_quat* quat, float u, const ce_quat* lhs,
                                                        const ce_quat* rhs);

static inline ce_quat* ce_quat_init(ce_quat* quat, float w, float x, float y, float z)
{
    quat->w = w;
    quat->x = x;
    quat->y = y;
    quat->z = z;
    return quat;
}

static inline ce_quat* ce_quat_init_array(ce_quat* quat, const float* array)
{
    quat->w = array[0];
    quat->x = array[1];
    quat->y = array[2];
    quat->z = array[3];
    return quat;
}

static inline ce_quat* ce_quat_init_zero(ce_quat* quat)
{
    quat->w = 0.0f;
    quat->x = 0.0f;
    quat->y = 0.0f;
    quat->z = 0.0f;
    return quat;
}

static inline ce_quat* ce_quat_init_identity(ce_quat* quat)
{
    quat->w = 1.0f;
    quat->x = 0.0f;
    quat->y = 0.0f;
    quat->z = 0.0f;
    return quat;
}

static inline ce_quat* ce_quat_copy(ce_quat* quat, const ce_quat* other)
{
    quat->w = other->w;
    quat->x = other->x;
    quat->y = other->y;
    quat->z = other->z;
    return quat;
}

static inline ce_quat* ce_quat_neg(ce_quat* quat, const ce_quat* other)
{
    quat->w = -other->w;
    quat->x = -other->x;
    quat->y = -other->y;
    quat->z = -other->z;
    return quat;
}

static inline ce_quat* ce_quat_conj(ce_quat* quat, const ce_quat* other)
{
    quat->w = other->w;
    quat->x = -other->x;
    quat->y = -other->y;
    quat->z = -other->z;
    return quat;
}

static inline ce_quat* ce_quat_add(ce_quat* quat, const ce_quat* lhs, const ce_quat* rhs)
{
    quat->w = lhs->w + rhs->w;
    quat->x = lhs->x + rhs->x;
    quat->y = lhs->y + rhs->y;
    quat->z = lhs->z + rhs->z;
    return quat;
}

static inline ce_quat* ce_quat_sub(ce_quat* quat, const ce_quat* lhs, const ce_quat* rhs)
{
    quat->w = lhs->w - rhs->w;
    quat->x = lhs->x - rhs->x;
    quat->y = lhs->y - rhs->y;
    quat->z = lhs->z - rhs->z;
    return quat;
}

static inline ce_quat* ce_quat_mul(ce_quat* restrict quat, const ce_quat* lhs,
                                                            const ce_quat* rhs)
{
    quat->w = lhs->w * rhs->w - lhs->x * rhs->x -
                lhs->y * rhs->y - lhs->z * rhs->z;
    quat->x = lhs->w * rhs->x + lhs->x * rhs->w +
                lhs->y * rhs->z - lhs->z * rhs->y;
    quat->y = lhs->w * rhs->y - lhs->x * rhs->z +
                lhs->y * rhs->w + lhs->z * rhs->x;
    quat->z = lhs->w * rhs->z + lhs->x * rhs->y -
                lhs->y * rhs->x + lhs->z * rhs->w;
    return quat;
}

static inline ce_quat* ce_quat_scale(ce_quat* quat, float s, const ce_quat* other)
{
    quat->w = s * other->w;
    quat->x = s * other->x;
    quat->y = s * other->y;
    quat->z = s * other->z;
    return quat;
}

static inline float ce_quat_len2(const ce_quat* quat)
{
    return quat->w * quat->w + quat->x * quat->x +
            quat->y * quat->y + quat->z * quat->z;
}

static inline float ce_quat_len(const ce_quat* quat)
{
    return sqrtf(ce_quat_len2(quat));
}

static inline float ce_quat_arg(const ce_quat* quat)
{
    float s = ce_quat_len(quat);
    return 0.0f == s ? 0.0f : acosf(quat->w / s);
}

static inline ce_quat* ce_quat_norm(ce_quat* quat, const ce_quat* other)
{
    return ce_quat_scale(quat, 1.0f / ce_quat_len(other), other);
}

static inline ce_quat* ce_quat_inv(ce_quat* quat, const ce_quat* other)
{
    return ce_quat_conj(quat,
            ce_quat_scale(quat, 1.0f / ce_quat_len2(other), other));
}

static inline float ce_quat_dot(const ce_quat* lhs, const ce_quat* rhs)
{
    return lhs->w * rhs->w + lhs->x * rhs->x +
            lhs->y * rhs->y + lhs->z * rhs->z;
}

#endif /* CE_QUAT_HPP */
