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

#include "sphere.hpp"

namespace cursedearth
{
    sphere_t* ce_sphere_init(sphere_t* sphere, const vector3_t* origin, float radius)
    {
        sphere->origin = *origin;
        sphere->radius = radius;
        return sphere;
    }

    sphere_t* ce_sphere_init_array(sphere_t* sphere, const float* array)
    {
        sphere->origin.x = array[0];
        sphere->origin.y = array[1];
        sphere->origin.z = array[2];
        sphere->radius = array[3];
        return sphere;
    }

    sphere_t* ce_sphere_init_zero(sphere_t* sphere)
    {
        sphere->origin = CE_VEC3_ZERO;
        sphere->radius = 0.0f;
        return sphere;
    }

    sphere_t* ce_sphere_copy(sphere_t* sphere, const sphere_t* other)
    {
        sphere->origin = other->origin;
        sphere->radius = other->radius;
        return sphere;
    }
}
