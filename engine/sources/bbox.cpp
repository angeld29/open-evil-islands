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

#include <cfloat>
#include <cmath>

#include "bbox.hpp"

namespace cursedearth
{
    bbox_t* ce_bbox_clear(bbox_t* bbox)
    {
        ce_aabb_clear(&bbox->aabb);
        bbox->axis = CE_QUAT_IDENTITY;
        return bbox;
    }

    bbox_t* ce_bbox_merge(bbox_t* bbox, const bbox_t* other)
    {
        vector3_t axis_x, axis_y, axis_z;
        ce_quat_to_axes(&other->axis, &axis_x, &axis_y, &axis_z);

        ce_vec3_scale(&axis_x, other->aabb.extents.x, &axis_x);
        ce_vec3_scale(&axis_y, other->aabb.extents.y, &axis_y);
        ce_vec3_scale(&axis_z, other->aabb.extents.z, &axis_z);

        aabb_t aabb;
        aabb.origin = other->aabb.origin;

        // calculate the aabb extents in local coord space
        // from the other extents and axes
        aabb.extents.x = fabsf(axis_x.x) + fabsf(axis_y.x) + fabsf(axis_z.x);
        aabb.extents.y = fabsf(axis_x.y) + fabsf(axis_y.y) + fabsf(axis_z.y);
        aabb.extents.z = fabsf(axis_x.z) + fabsf(axis_y.z) + fabsf(axis_z.z);

        ce_aabb_merge_aabb(&bbox->aabb, &aabb);
        ce_aabb_update_radius(&bbox->aabb);

        bbox->axis = CE_QUAT_IDENTITY;

        return bbox;
    }
}
