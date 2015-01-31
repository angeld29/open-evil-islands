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

#include <stdbool.h>

#include "vec3.hpp"
#include "ray.hpp"
#include "triangle.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ce_vec3 n;
    float d;
} ce_plane;

static inline ce_plane* ce_plane_init(ce_plane* plane, float a, float b,
                                                        float c, float d)
{
    ce_vec3_init(&plane->n, a, b, c);
    plane->d = d;
    return plane;
}

static inline ce_plane* ce_plane_init_array(ce_plane* plane, const float* array)
{
    ce_vec3_init_array(&plane->n, array);
    plane->d = array[3];
    return plane;
}

static inline ce_plane* ce_plane_init_triangle(ce_plane* plane,
                                                const ce_triangle* triangle)
{
    ce_triangle_calc_normal(triangle, &plane->n);
    plane->d = -ce_vec3_dot(&plane->n, &triangle->a);
    return plane;
}

static inline ce_plane* ce_plane_init_tri(ce_plane* plane, const ce_vec3* a,
                                                            const ce_vec3* b,
                                                            const ce_vec3* c)
{
    ce_triangle triangle;
    ce_triangle_init(&triangle, a, b, c);
    return ce_plane_init_triangle(plane, &triangle);
}

static inline ce_plane* ce_plane_normalise(ce_plane* plane, const ce_plane* other)
{
    const float s = 1.0f / ce_vec3_len(&other->n);
    ce_vec3_scale(&plane->n, s, &other->n);
    plane->d = s * other->d;
    return plane;
}

static inline float ce_plane_dist(const ce_plane* plane, const ce_vec3* point)
{
    return ce_vec3_dot(&plane->n, point) + plane->d;
}

static inline float ce_plane_dist_ray(const ce_plane* plane, const ce_ray* ray)
{
    return (-plane->d - ce_vec3_dot(&plane->n, &ray->origin)) /
            ce_vec3_dot(&plane->n, &ray->direction);
}

extern bool ce_plane_isect_ray(const ce_plane* plane, const ce_ray* ray, ce_vec3* point);

#ifdef __cplusplus
}
#endif

#endif /* CE_PLANE_HPP */
