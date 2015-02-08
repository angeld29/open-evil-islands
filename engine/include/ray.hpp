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

#ifndef CE_RAY_HPP
#define CE_RAY_HPP

#include "vector3.hpp"

namespace cursedearth
{
    struct ray_t
    {
        vector3_t origin;
        vector3_t direction;
    };

    inline ray_t* ce_ray_init(ray_t* ray, const vector3_t* origin, const vector3_t* direction)
    {
        ce_vec3_copy(&ray->origin, origin);
        ce_vec3_copy(&ray->direction, direction);
        return ray;
    }

    inline ray_t* ce_ray_init_segment(ray_t* ray, const vector3_t* start, const vector3_t* end)
    {
        ce_vec3_copy(&ray->origin, start);
        ce_vec3_norm(&ray->direction, ce_vec3_sub(&ray->direction, end, start));
        return ray;
    }
}

#endif
