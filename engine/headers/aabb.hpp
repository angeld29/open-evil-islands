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

#ifndef CE_AABB_HPP
#define CE_AABB_HPP

#include "vector3.hpp"

namespace cursedearth
{
    /**
     * @brief Axis-Aligned Bounding Box
     *        origin: centre of bounding box
     *        extents: +/- extents of box from origin
     *        radius: cached length of extents vector
     */
    struct aabb_t
    {
        vector3_t origin, extents;
        float radius;
    };

    aabb_t* ce_aabb_init(aabb_t* aabb, const vector3_t* origin, const vector3_t* extents, float radius);
    aabb_t* ce_aabb_init_zero(aabb_t* aabb);
    aabb_t* ce_aabb_copy(aabb_t* aabb, const aabb_t* other);

    aabb_t* ce_aabb_clear(aabb_t* aabb);
    aabb_t* ce_aabb_update_radius(aabb_t* aabb);

    aabb_t* ce_aabb_merge_aabb(aabb_t* aabb, const aabb_t* other);
    aabb_t* ce_aabb_merge_point(aabb_t* aabb, const vector3_t* point);
    aabb_t* ce_aabb_merge_point_array(aabb_t* aabb, const float* point);
}

#endif
