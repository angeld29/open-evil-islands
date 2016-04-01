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

#include "vector3.hpp"

namespace cursedearth
{
    struct triangle_t
    {
        vector3_t a, b, c;
    };

    inline triangle_t* ce_triangle_init(triangle_t* triangle, const vector3_t* a, const vector3_t* b, const vector3_t* c)
    {
        ce_vec3_copy(&triangle->a, a);
        ce_vec3_copy(&triangle->b, b);
        ce_vec3_copy(&triangle->c, c);
        return triangle;
    }

    inline vector3_t* ce_triangle_calc_normal(const triangle_t* triangle, vector3_t* normal)
    {
        vector3_t edge1, edge2;
        ce_vec3_sub(&edge1, &triangle->b, &triangle->a);
        ce_vec3_sub(&edge2, &triangle->c, &triangle->a);
        return ce_vec3_norm(normal, ce_vec3_cross(normal, &edge1, &edge2));
    }

    bool ce_triangle_test(const triangle_t* triangle, const vector3_t* point);
}

#endif
