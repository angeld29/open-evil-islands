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

#include "logging.hpp"
#include "triangle.hpp"

bool ce_triangle_test(const ce_triangle* triangle, const ce_vec3* point)
{
    ce_vec3 normal;
    ce_triangle_calc_normal(triangle, &normal);

    ce_vec3 l1, l2, l3;
    ce_vec3_sub(&l1, &triangle->a, point);
    ce_vec3_sub(&l2, &triangle->b, point);
    ce_vec3_sub(&l3, &triangle->c, point);

    ce_vec3 t1, t2, t3;
    ce_vec3_cross(&t1, &l1, &l2);
    ce_vec3_cross(&t2, &l2, &l3);
    ce_vec3_cross(&t3, &l3, &l1);

    float s1 = ce_vec3_dot(&t1, &normal);
    float s2 = ce_vec3_dot(&t2, &normal);
    float s3 = ce_vec3_dot(&t3, &normal);

    return (s1 <= 0.0f && s2 <= 0.0f && s3 <= 0.0f) ||
            (s1 >= 0.0f && s2 >= 0.0f && s3 >= 0.0f);
}
