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

#ifndef CE_FRUSTUM_HPP
#define CE_FRUSTUM_HPP

#include "vector3.hpp"
#include "quaternion.hpp"
#include "sphere.hpp"
#include "aabb.hpp"
#include "bbox.hpp"
#include "plane.hpp"

namespace cursedearth
{
    enum frustum_plane_t {
        CE_FRUSTUM_PLANE_TOP,
        CE_FRUSTUM_PLANE_BOTTOM,
        CE_FRUSTUM_PLANE_LEFT,
        CE_FRUSTUM_PLANE_RIGHT,
        CE_FRUSTUM_PLANE_NEAR,
        CE_FRUSTUM_PLANE_FAR,
        CE_FRUSTUM_PLANE_COUNT
    };

    struct frustum_t
    {
        plane_t planes[CE_FRUSTUM_PLANE_COUNT];
    };

    frustum_t* ce_frustum_init(frustum_t* frustum, float fov, float aspect, float near, float far, const vector3_t* position, const vector3_t* forward, const vector3_t* right, const vector3_t* up);

    bool ce_frustum_test_point(const frustum_t* frustum, const vector3_t* point);
    bool ce_frustum_test_sphere(const frustum_t* frustum, const sphere_t* sphere);
    bool ce_frustum_test_aabb(const frustum_t* frustum, const aabb_t* aabb);
    bool ce_frustum_test_bbox(const frustum_t* frustum, const bbox_t* bbox);
}

#endif
