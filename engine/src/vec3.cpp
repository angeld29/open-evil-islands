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

#include <cmath>
#include <algorithm>

#include "quat.hpp"
#include "vec3.hpp"

namespace cursedearth
{
    float vec3_t::length() const
    {
        return std::sqrt(square_length());
    }

    void vec3_t::rotate(const ce_quat& quat)
    {
        vec3_t qv = { quat.x, quat.y, quat.z }, uv, uuv;
        uv = cross(qv, *this);
        uuv = cross(qv, uv);
        uv *= 2.0f * quat.w;
        uuv *= 2.0f;
        *this += uv + uuv;
    }

    float distance(const vec3_t& lhs, const vec3_t& rhs)
    {
        return std::sqrt(square_distance(lhs, rhs));
    }

    float absolute_dot(const vec3_t& lhs, const vec3_t& rhs)
    {
        return std::abs(lhs.x * rhs.x) + std::abs(lhs.y * rhs.y) + std::abs(lhs.z * rhs.z);
    }

    vec3_t floor(const vec3_t& lhs, const vec3_t& rhs)
    {
        vec.x = std::min(lhs.x, rhs.x);
        vec.y = std::min(lhs.y, rhs.y);
        vec.z = std::min(lhs.z, rhs.z);
        return vec;
    }

    vec3_t ceil(const vec3_t& lhs, const vec3_t& rhs)
    {
        vec.x = std::max(lhs.x, rhs.x);
        vec.y = std::max(lhs.y, rhs.y);
        vec.z = std::max(lhs.z, rhs.z);
        return vec;
    }
}
