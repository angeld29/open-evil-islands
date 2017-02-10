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

#include <cmath>

#include "utility.hpp"
#include "quaternion.hpp"
#include "vector3.hpp"

namespace cursedearth
{
    const vector3_t CE_VEC3_ZERO = { 0.0f, 0.0f, 0.0f };
    const vector3_t CE_VEC3_UNIT_X = { 1.0f, 0.0f, 0.0f };
    const vector3_t CE_VEC3_UNIT_Y = { 0.0f, 1.0f, 0.0f };
    const vector3_t CE_VEC3_UNIT_Z = { 0.0f, 0.0f, 1.0f };
    const vector3_t CE_VEC3_UNIT_SCALE = { 1.0f, 1.0f, 1.0f };
    const vector3_t CE_VEC3_NEG_UNIT_X = { -1.0f, 0.0f, 0.0f };
    const vector3_t CE_VEC3_NEG_UNIT_Y = { 0.0f, -1.0f, 0.0f };
    const vector3_t CE_VEC3_NEG_UNIT_Z = { 0.0f, 0.0f, -1.0f };
    const vector3_t CE_VEC3_NEG_UNIT_SCALE = { -1.0f, -1.0f, -1.0f };

    float ce_vec3_len(const vector3_t* vec)
    {
        return sqrtf(ce_vec3_len2(vec));
    }

    float ce_vec3_len2(const vector3_t* vec)
    {
        return vec->x * vec->x + vec->y * vec->y + vec->z * vec->z;
    }

    float ce_vec3_dist(const vector3_t* lhs, const vector3_t* rhs)
    {
        return sqrtf(ce_vec3_dist2(lhs, rhs));
    }

    float ce_vec3_dist2(const vector3_t* lhs, const vector3_t* rhs)
    {
        vector3_t tmp;
        return ce_vec3_len2(ce_vec3_sub(&tmp, rhs, lhs));
    }

    vector3_t* ce_vec3_norm(vector3_t* vec, const vector3_t* other)
    {
        return ce_vec3_scale(vec, 1.0f / ce_vec3_len(other), other);
    }

    float ce_vec3_dot(const vector3_t* lhs, const vector3_t* rhs)
    {
        return lhs->x * rhs->x + lhs->y * rhs->y + lhs->z * rhs->z;
    }

    float ce_vec3_absdot(const vector3_t* lhs, const vector3_t* rhs)
    {
        return fabsf(lhs->x * rhs->x) + fabsf(lhs->y * rhs->y) + fabsf(lhs->z * rhs->z);
    }

    vector3_t* ce_vec3_cross(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs)
    {
        vec->x = lhs->y * rhs->z - lhs->z * rhs->y;
        vec->y = lhs->z * rhs->x - lhs->x * rhs->z;
        vec->z = lhs->x * rhs->y - lhs->y * rhs->x;
        return vec;
    }

    vector3_t* ce_vec3_mid(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs)
    {
        return ce_vec3_scale(vec, 0.5f, ce_vec3_add(vec, lhs, rhs));
    }

    vector3_t* ce_vec3_rot(vector3_t* vec, const vector3_t* other, const quaternion_t* quat)
    {
        vector3_t qv = { quat->x, quat->y, quat->z }, uv, uuv;
        ce_vec3_cross(&uv, &qv, other);
        ce_vec3_cross(&uuv, &qv, &uv);
        ce_vec3_scale(&uv, 2.0f * quat->w, &uv);
        ce_vec3_scale(&uuv, 2.0f, &uuv);
        return ce_vec3_add(vec, ce_vec3_add(vec, other, &uv), &uuv);
    }

    vector3_t* ce_vec3_lerp(vector3_t* vec, float u, const vector3_t* lhs, const vector3_t* rhs)
    {
        vec->x = lerp(u, lhs->x, rhs->x);
        vec->y = lerp(u, lhs->y, rhs->y);
        vec->z = lerp(u, lhs->z, rhs->z);
        return vec;
    }

    vector3_t* ce_vec3_floor(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs)
    {
        vec->x = fminf(lhs->x, rhs->x);
        vec->y = fminf(lhs->y, rhs->y);
        vec->z = fminf(lhs->z, rhs->z);
        return vec;
    }

    vector3_t* ce_vec3_ceil(vector3_t* vec, const vector3_t* lhs, const vector3_t* rhs)
    {
        vec->x = fmaxf(lhs->x, rhs->x);
        vec->y = fmaxf(lhs->y, rhs->y);
        vec->z = fmaxf(lhs->z, rhs->z);
        return vec;
    }
}
