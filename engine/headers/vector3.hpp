/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#ifndef CE_VECTOR3_HPP
#define CE_VECTOR3_HPP

namespace cursedearth
{
    struct quaternion_t;

    struct vector3_t
    {
        float x, y, z;
    };

    extern const vector3_t CE_VEC3_ZERO;
    extern const vector3_t CE_VEC3_UNIT_X;
    extern const vector3_t CE_VEC3_UNIT_Y;
    extern const vector3_t CE_VEC3_UNIT_Z;
    extern const vector3_t CE_VEC3_UNIT_SCALE;
    extern const vector3_t CE_VEC3_NEG_UNIT_X;
    extern const vector3_t CE_VEC3_NEG_UNIT_Y;
    extern const vector3_t CE_VEC3_NEG_UNIT_Z;
    extern const vector3_t CE_VEC3_NEG_UNIT_SCALE;

    inline vector3_t* ce_vec3_init(vector3_t* vec, float x, float y, float z)
    {
        vec->x = x;
        vec->y = y;
        vec->z = z;
        return vec;
    }

    inline vector3_t* ce_vec3_init_scalar(vector3_t* vec, float s)
    {
        vec->x = s;
        vec->y = s;
        vec->z = s;
        return vec;
    }

    inline vector3_t* ce_vec3_init_array(vector3_t* vec, const float* array)
    {
        vec->x = array[0];
        vec->y = array[1];
        vec->z = array[2];
        return vec;
    }

    inline vector3_t* ce_vec3_init_zero(vector3_t* vec)
    {
        vec->x = 0.0f;
        vec->y = 0.0f;
        vec->z = 0.0f;
        return vec;
    }

    inline vector3_t* ce_vec3_init_unit_x(vector3_t* vec)
    {
        vec->x = 1.0f;
        vec->y = 0.0f;
        vec->z = 0.0f;
        return vec;
    }

    inline vector3_t* ce_vec3_init_unit_y(vector3_t* vec)
    {
        vec->x = 0.0f;
        vec->y = 1.0f;
        vec->z = 0.0f;
        return vec;
    }

    inline vector3_t* ce_vec3_init_unit_z(vector3_t* vec)
    {
        vec->x = 0.0f;
        vec->y = 0.0f;
        vec->z = 1.0f;
        return vec;
    }

    inline vector3_t* ce_vec3_init_unit_scale(vector3_t* vec)
    {
        vec->x = 1.0f;
        vec->y = 1.0f;
        vec->z = 1.0f;
        return vec;
    }

    inline vector3_t* ce_vec3_init_neg_unit_x(vector3_t* vec)
    {
        vec->x = -1.0f;
        vec->y = 0.0f;
        vec->z = 0.0f;
        return vec;
    }

    inline vector3_t* ce_vec3_init_neg_unit_y(vector3_t* vec)
    {
        vec->x = 0.0f;
        vec->y = -1.0f;
        vec->z = 0.0f;
        return vec;
    }

    inline vector3_t* ce_vec3_init_neg_unit_z(vector3_t* vec)
    {
        vec->x = 0.0f;
        vec->y = 0.0f;
        vec->z = -1.0f;
        return vec;
    }

    inline vector3_t* ce_vec3_init_neg_unit_scale(vector3_t* vec)
    {
        vec->x = -1.0f;
        vec->y = -1.0f;
        vec->z = -1.0f;
        return vec;
    }

    inline vector3_t* ce_vec3_copy(vector3_t* vec, const vector3_t* other)
    {
        vec->x = other->x;
        vec->y = other->y;
        vec->z = other->z;
        return vec;
    }

    inline vector3_t* ce_vec3_neg(vector3_t* vec, const vector3_t* other)
    {
        vec->x = -other->x;
        vec->y = -other->y;
        vec->z = -other->z;
        return vec;
    }

    inline vector3_t* ce_vec3_add(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs)
    {
        vec->x = lhs->x + rhs->x;
        vec->y = lhs->y + rhs->y;
        vec->z = lhs->z + rhs->z;
        return vec;
    }

    inline vector3_t* ce_vec3_sub(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs)
    {
        vec->x = lhs->x - rhs->x;
        vec->y = lhs->y - rhs->y;
        vec->z = lhs->z - rhs->z;
        return vec;
    }

    inline vector3_t* ce_vec3_mul(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs)
    {
        vec->x = lhs->x * rhs->x;
        vec->y = lhs->y * rhs->y;
        vec->z = lhs->z * rhs->z;
        return vec;
    }

    inline vector3_t* ce_vec3_div(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs)
    {
        vec->x = lhs->x / rhs->x;
        vec->y = lhs->y / rhs->y;
        vec->z = lhs->z / rhs->z;
        return vec;
    }

    inline vector3_t* ce_vec3_scale(vector3_t* vec, float s, const vector3_t* other)
    {
        vec->x = s * other->x;
        vec->y = s * other->y;
        vec->z = s * other->z;
        return vec;
    }

    float ce_vec3_len(const vector3_t* vec);
    float ce_vec3_len2(const vector3_t* vec);

    float ce_vec3_dist(const vector3_t* lhs, const vector3_t* rhs);
    float ce_vec3_dist2(const vector3_t* lhs, const vector3_t* rhs);

    vector3_t* ce_vec3_norm(vector3_t* vec, const vector3_t* other);

    float ce_vec3_dot(const vector3_t* lhs, const vector3_t* rhs);
    float ce_vec3_absdot(const vector3_t* lhs, const vector3_t* rhs);

    vector3_t* ce_vec3_cross(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs);

    vector3_t* ce_vec3_mid(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs);
    vector3_t* ce_vec3_rot(vector3_t* vec, const vector3_t* other, const struct quaternion_t* quat);

    vector3_t* ce_vec3_lerp(vector3_t* vec, float u, const vector3_t* lhs, const vector3_t* rhs);

    vector3_t* ce_vec3_floor(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs);
    vector3_t* ce_vec3_ceil(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs);
}

#endif
