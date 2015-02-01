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

#ifndef CE_PLANE_HPP
#define CE_PLANE_HPP

#include "vec3.hpp"
#include "ray.hpp"
#include "triangle.hpp"

namespace cursedearth
{
    struct plane_t
    {
        vec3_t n;
        float d = 0.0f;

        plane_t() {}
        plane_t(float a, float b, float c, float d): n(a, b, c), d(d) {}
        explicit plane_t(const float array[4]): n(array), d(array[3]) {}
        explicit plane_t(const triangle_t& triangle): n(triangle.get_normal()), d(-dot(n, triangle.a)) {}

        bool intersect(const ray_t& ray, vec3_t& point) const;

        void normalise()
        {
            const float s = 1.0f / n.length();
            n *= s;
            d *= s;
        }
    };

    inline float distance(const plane_t& plane, const vec3_t& point)
    {
        return dot(plane.n, point) + plane.d;
    }

    inline float distance(const plane_t& plane, const ray_t& ray)
    {
        return (-plane.d - dot(plane.n, ray.origin)) / dot(plane.n, ray.direction);
    }

    inline float distance(const vec3_t& point, const plane_t& plane)
    {
        return distance(plane, point);
    }

    inline float distance(const ray_t& ray, const plane_t& plane)
    {
        return distance(plane, ray);
    }
}

#endif
