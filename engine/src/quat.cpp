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

#include "math.hpp"
#include "vec3.hpp"
#include "quat.hpp"

const ce_quat CE_QUAT_ZERO = {.w = 0.0f, .x = 0.0f, .y = 0.0f, .z = 0.0f};
const ce_quat CE_QUAT_IDENTITY = {.w = 1.0f, .x = 0.0f, .y = 0.0f, .z = 0.0f};

ce_quat* ce_quat_init_polar(ce_quat* quat, float angle, const ce_vec3* axis)
{
    float t = 0.5f * angle;
    float s = sinf(t);
    quat->w = cosf(t);
    quat->x = s * axis->x;
    quat->y = s * axis->y;
    quat->z = s * axis->z;
    return quat;
}

float ce_quat_to_polar(const ce_quat* quat, ce_vec3* axis)
{
    float sqr_length = quat->x * quat->x +
                        quat->y * quat->y +
                        quat->z * quat->z;

    if (sqr_length > 0.0f) {
        float inv_length = 1.0f / sqrtf(sqr_length);
        axis->x = quat->x * inv_length;
        axis->y = quat->y * inv_length;
        axis->z = quat->z * inv_length;
        return 2.0f * acosf(quat->w);
    }

    *axis = CE_VEC3_UNIT_X;
    return 0.0f;
}

void ce_quat_to_axes(const ce_quat* quat, ce_vec3* xaxis,
                                        ce_vec3* yaxis,
                                        ce_vec3* zaxis)
{
    float tx = 2.0f * quat->x;
    float ty = 2.0f * quat->y;
    float tz = 2.0f * quat->z;
    float twx = tx * quat->w;
    float twy = ty * quat->w;
    float twz = tz * quat->w;
    float txx = tx * quat->x;
    float txy = ty * quat->x;
    float txz = tz * quat->x;
    float tyy = ty * quat->y;
    float tyz = tz * quat->y;
    float tzz = tz * quat->z;

    xaxis->x = 1.0f - (tyy + tzz);
    xaxis->y = txy + twz;
    xaxis->z = txz - twy;

    yaxis->x = txy - twz;
    yaxis->y = 1.0f - (txx + tzz);
    yaxis->z = tyz + twx;

    zaxis->x = txz + twy;
    zaxis->y = tyz - twx;
    zaxis->z = 1.0f - (txx + tyy);
}

ce_quat* ce_quat_slerp(ce_quat* quat, float u, const ce_quat* lhs,
                                                const ce_quat* rhs)
{
    ce_quat ta, tb = *rhs;
    float cosom = ce_quat_dot(lhs, rhs);

    if (cosom < 0.0f) {
        // invert rotation
        cosom = -cosom;
        ce_quat_neg(&tb, rhs);
    }

    if (cosom < 1.0f - CE_EPS_E3) {
        // standard case
        float angle = acosf(cosom);
        float inv_sinom = 1.0f / sinf(angle);
        return ce_quat_add(quat,
            ce_quat_scale(&ta, sinf((1.0f - u) * angle) * inv_sinom, lhs),
            ce_quat_scale(&tb, sinf(u * angle) * inv_sinom, &tb));
    }

    // quaternions are very close
    // linear interpolation
    // taking the complement requires renormalisation
    return ce_quat_norm(quat,
        ce_quat_add(quat,
            ce_quat_scale(&ta, 1.0f - u, lhs),
            ce_quat_scale(&tb, u, &tb)));
}
