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

#ifndef CE_PLANE_HPP
#define CE_PLANE_HPP

#include "vector3.hpp"
#include "ray.hpp"
#include "triangle.hpp"

namespace cursedearth
{
    struct plane_t
    {
        vector3_t n;
        float d;
    };

    inline plane_t* ce_plane_init(plane_t* plane, float a, float b, float c, float d)
    {
        ce_vec3_init(&plane->n, a, b, c);
        plane->d = d;
        return plane;
    }

    inline plane_t* ce_plane_init_array(plane_t* plane, const float* array)
    {
        ce_vec3_init_array(&plane->n, array);
        plane->d = array[3];
        return plane;
    }

    inline plane_t* ce_plane_init_triangle(plane_t* plane, const triangle_t* triangle)
    {
        ce_triangle_calc_normal(triangle, &plane->n);
        plane->d = -ce_vec3_dot(&plane->n, &triangle->a);
        return plane;
    }

    inline plane_t* ce_plane_init_tri(plane_t* plane, const vector3_t* a, const vector3_t* b, const vector3_t* c)
    {
        triangle_t triangle;
        ce_triangle_init(&triangle, a, b, c);
        return ce_plane_init_triangle(plane, &triangle);
    }

    inline plane_t* ce_plane_normalise(plane_t* plane, const plane_t* other)
    {
        const float s = 1.0f / ce_vec3_len(&other->n);
        ce_vec3_scale(&plane->n, s, &other->n);
        plane->d = s * other->d;
        return plane;
    }

    inline float ce_plane_dist(const plane_t* plane, const vector3_t* point)
    {
        return ce_vec3_dot(&plane->n, point) + plane->d;
    }

    inline float ce_plane_dist_ray(const plane_t* plane, const ray_t* ray)
    {
        return (-plane->d - ce_vec3_dot(&plane->n, &ray->origin)) / ce_vec3_dot(&plane->n, &ray->direction);
    }

    bool ce_plane_isect_ray(const plane_t* plane, const ray_t* ray, vector3_t* point);
}

#endif
