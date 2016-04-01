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

#ifndef CE_QUATERNION_HPP
#define CE_QUATERNION_HPP

namespace cursedearth
{
    struct vector3_t;

    struct quaternion_t
    {
        float w, x, y, z;
    };

    extern const quaternion_t CE_QUAT_ZERO;
    extern const quaternion_t CE_QUAT_IDENTITY;

    quaternion_t* ce_quat_init_polar(quaternion_t* quat, float angle, const struct vector3_t* axis);

    float ce_quat_to_polar(const quaternion_t* quat, struct vector3_t* axis);
    void ce_quat_to_axes(const quaternion_t* quat, struct vector3_t* xaxis, struct vector3_t* yaxis, struct vector3_t* zaxis);

    float ce_quat_len(const quaternion_t* quat);
    float ce_quat_arg(const quaternion_t* quat);

    // spherical linear interpolation
    quaternion_t* ce_quat_slerp(quaternion_t* quat, float u, const quaternion_t* lhs, const quaternion_t* rhs);

    inline quaternion_t* ce_quat_init(quaternion_t* quat, float w, float x, float y, float z)
    {
        quat->w = w;
        quat->x = x;
        quat->y = y;
        quat->z = z;
        return quat;
    }

    inline quaternion_t* ce_quat_init_array(quaternion_t* quat, const float* array)
    {
        quat->w = array[0];
        quat->x = array[1];
        quat->y = array[2];
        quat->z = array[3];
        return quat;
    }

    inline quaternion_t* ce_quat_init_zero(quaternion_t* quat)
    {
        quat->w = 0.0f;
        quat->x = 0.0f;
        quat->y = 0.0f;
        quat->z = 0.0f;
        return quat;
    }

    inline quaternion_t* ce_quat_init_identity(quaternion_t* quat)
    {
        quat->w = 1.0f;
        quat->x = 0.0f;
        quat->y = 0.0f;
        quat->z = 0.0f;
        return quat;
    }

    inline quaternion_t* ce_quat_copy(quaternion_t* quat, const quaternion_t* other)
    {
        quat->w = other->w;
        quat->x = other->x;
        quat->y = other->y;
        quat->z = other->z;
        return quat;
    }

    inline quaternion_t* ce_quat_neg(quaternion_t* quat, const quaternion_t* other)
    {
        quat->w = -other->w;
        quat->x = -other->x;
        quat->y = -other->y;
        quat->z = -other->z;
        return quat;
    }

    inline quaternion_t* ce_quat_conj(quaternion_t* quat, const quaternion_t* other)
    {
        quat->w = other->w;
        quat->x = -other->x;
        quat->y = -other->y;
        quat->z = -other->z;
        return quat;
    }

    inline quaternion_t* ce_quat_add(quaternion_t* quat, const quaternion_t* lhs, const quaternion_t* rhs)
    {
        quat->w = lhs->w + rhs->w;
        quat->x = lhs->x + rhs->x;
        quat->y = lhs->y + rhs->y;
        quat->z = lhs->z + rhs->z;
        return quat;
    }

    inline quaternion_t* ce_quat_sub(quaternion_t* quat, const quaternion_t* lhs, const quaternion_t* rhs)
    {
        quat->w = lhs->w - rhs->w;
        quat->x = lhs->x - rhs->x;
        quat->y = lhs->y - rhs->y;
        quat->z = lhs->z - rhs->z;
        return quat;
    }

    inline quaternion_t* ce_quat_mul(quaternion_t* quat, const quaternion_t* lhs, const quaternion_t* rhs)
    {
        quat->w = lhs->w * rhs->w - lhs->x * rhs->x - lhs->y * rhs->y - lhs->z * rhs->z;
        quat->x = lhs->w * rhs->x + lhs->x * rhs->w + lhs->y * rhs->z - lhs->z * rhs->y;
        quat->y = lhs->w * rhs->y - lhs->x * rhs->z + lhs->y * rhs->w + lhs->z * rhs->x;
        quat->z = lhs->w * rhs->z + lhs->x * rhs->y - lhs->y * rhs->x + lhs->z * rhs->w;
        return quat;
    }

    inline quaternion_t* ce_quat_scale(quaternion_t* quat, float s, const quaternion_t* other)
    {
        quat->w = s * other->w;
        quat->x = s * other->x;
        quat->y = s * other->y;
        quat->z = s * other->z;
        return quat;
    }

    inline float ce_quat_len2(const quaternion_t* quat)
    {
        return quat->w * quat->w + quat->x * quat->x + quat->y * quat->y + quat->z * quat->z;
    }

    inline quaternion_t* ce_quat_norm(quaternion_t* quat, const quaternion_t* other)
    {
        return ce_quat_scale(quat, 1.0f / ce_quat_len(other), other);
    }

    inline quaternion_t* ce_quat_inv(quaternion_t* quat, const quaternion_t* other)
    {
        return ce_quat_conj(quat, ce_quat_scale(quat, 1.0f / ce_quat_len2(other), other));
    }

    inline float ce_quat_dot(const quaternion_t* lhs, const quaternion_t* rhs)
    {
        return lhs->w * rhs->w + lhs->x * rhs->x + lhs->y * rhs->y + lhs->z * rhs->z;
    }
}

#endif
