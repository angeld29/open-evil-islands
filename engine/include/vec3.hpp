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

#ifndef CE_VEC3_HPP
#define CE_VEC3_HPP

#include "math.hpp"

namespace cursedearth
{
    struct ce_quat;

    struct vec3_t
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;

        vec3_t() {}
        vec3_t(float x, float y, float z): x(x), y(y), z(z) {}
        explicit vec3_t(float scalar): x(scalar), y(scalar), z(scalar) {}
        explicit vec3_t(const float array[3]): x(array[0]), y(array[1]), z(array[2]) {}

        float length() const;
        void rotate(const ce_quat&);

        vec3_t& operator -()
        {
            x = -x;
            y = -y;
            z = -z;
            return *this;
        }

        vec3_t& operator +=(const vec3_t& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }

        vec3_t& operator -=(const vec3_t& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        vec3_t& operator *=(const vec3_t& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            return *this;
        }

        vec3_t& operator /=(const vec3_t& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            return *this;
        }

        vec3_t& operator *=(float rhs)
        {
            x *= rhs;
            y *= rhs;
            z *= rhs;
            return *this;
        }

        float square_length() const
        {
            return x * x + y * y + z * z;
        }

        void normalize()
        {
            *this *= 1.0f / length();
        }

        static vec3_t make_unit_x() { return { 1.0f, 0.0f, 0.0f }; }
        static vec3_t make_unit_y() { return { 0.0f, 1.0f, 0.0f }; }
        static vec3_t make_unit_z() { return { 0.0f, 0.0f, 1.0f }; }
        static vec3_t make_unit_scale() { return { 1.0f, 1.0f, 1.0f }; }
        static vec3_t make_neg_unit_x() { return { -1.0f, 0.0f, 0.0f }; }
        static vec3_t make_neg_unit_y() { return { 0.0f, -1.0f, 0.0f }; }
        static vec3_t make_neg_unit_z() { return { 0.0f, 0.0f, -1.0f }; }
        static vec3_t make_neg_unit_scale() { return { -1.0f, -1.0f, -1.0f }; }
    };

    float distance(const vec3_t& lhs, const vec3_t& rhs);
    float absolute_dot(const vec3_t& lhs, const vec3_t& rhs);

    vec3_t floor(const vec3_t& lhs, const vec3_t& rhs);
    vec3_t ceil(const vec3_t& lhs, const vec3_t& rhs);

    inline vec3_t operator +(const vec3_t& lhs, const vec3_t& rhs) { return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z }; }
    inline vec3_t operator -(const vec3_t& lhs, const vec3_t& rhs) { return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z }; }
    inline vec3_t operator *(const vec3_t& lhs, const vec3_t& rhs) { return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z }; }
    inline vec3_t operator /(const vec3_t& lhs, const vec3_t& rhs) { return { lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z }; }

    inline vec3_t operator *(const vec3_t& lhs, float rhs) { return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs }; }
    inline vec3_t operator *(float lhs, const vec3_t& rhs) { return { lhs * rhs.x, lhs * rhs.y, lhs * rhs.z }; }

    vec3_t normalize(const vec3_t& vec3)
    {
        return vec3 * (1.0f / vec3.length());
    }

    inline float square_distance(const vec3_t& lhs, const vec3_t& rhs)
    {
        return (rhs - lhs).square_length();
    }

    inline float dot(const vec3_t& lhs, const vec3_t& rhs)
    {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
    }

    inline vec3_t cross(const vec3_t& lhs, const vec3_t& rhs)
    {
        return { lhs.y * rhs.z - lhs.z * rhs.y,
                 lhs.z * rhs.x - lhs.x * rhs.z,
                 lhs.x * rhs.y - lhs.y * rhs.x };
    }

    inline vec3_t midpoint(const vec3_t& lhs, const vec3_t& rhs)
    {
        return 0.5f * (lhs + rhs);
    }

    inline vec3_t lerp(const vec3_t& lhs, const vec3_t& rhs, float u)
    {
        return { lerp(lhs.x, rhs.x, u),
                 lerp(lhs.y, rhs.y, u),
                 lerp(lhs.z, rhs.z, u) };
    }
}

#endif /* CE_VEC3_HPP */
