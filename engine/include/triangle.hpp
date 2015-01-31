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

#ifndef CE_TRIANGLE_HPP
#define CE_TRIANGLE_HPP

#include <stdbool.h>

#include "vec3.hpp"

typedef struct {
    ce_vec3 a, b, c;
} ce_triangle;

static inline ce_triangle* ce_triangle_init(ce_triangle* triangle, const ce_vec3* a, const ce_vec3* b, const ce_vec3* c)
{
    ce_vec3_copy(&triangle->a, a);
    ce_vec3_copy(&triangle->b, b);
    ce_vec3_copy(&triangle->c, c);
    return triangle;
}

static inline ce_vec3* ce_triangle_calc_normal(const ce_triangle* triangle, ce_vec3* normal)
{
    ce_vec3 edge1, edge2;
    ce_vec3_sub(&edge1, &triangle->b, &triangle->a);
    ce_vec3_sub(&edge2, &triangle->c, &triangle->a);
    return ce_vec3_norm(normal, ce_vec3_cross(normal, &edge1, &edge2));
}

extern bool ce_triangle_test(const ce_triangle* triangle, const ce_vec3* point);

#endif /* CE_TRIANGLE_HPP */
