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

#include <cmath>
#include <limits>
#include <array>
#include <algorithm>
#include <iterator>

#include "bbox.hpp"

namespace cursedearth
{
    namespace
    {
        void compute_corners(const bbox_t& bbox, std::array<vec3_t, 8>& corners)
        {
            vec3_t axis_x, axis_y, axis_z;
            ce_quat_to_axes(&bbox.axis, &axis_x, &axis_y, &axis_z);

            axis_x *= bbox.aabb.extents.x;
            axis_y *= bbox.aabb.extents.y;
            axis_z *= bbox.aabb.extents.z;

            std::fill(begin(corners), end(corners), bbox.aabb.origin);

            corners[0] -= axis_x; corners[1] += axis_x; corners[2] += axis_x; corners[3] -= axis_x;
            corners[0] -= axis_y; corners[1] -= axis_y; corners[2] += axis_y; corners[3] += axis_y;
            corners[0] -= axis_z; corners[1] -= axis_z; corners[2] -= axis_z; corners[3] -= axis_z;

            corners[4] -= axis_x; corners[5] += axis_x; corners[6] += axis_x; corners[7] -= axis_x;
            corners[4] -= axis_y; corners[5] -= axis_y; corners[6] += axis_y; corners[7] += axis_y;
            corners[4] += axis_z; corners[5] += axis_z; corners[6] += axis_z; corners[7] += axis_z;
        }

        void find_min_max(const vec3_t& center, const vec3_t& axis_x, const vec3_t& axis_y, const vec3_t& axis_z, std::array<vec3_t, 8>& corners, vec3_t& min, vec3_t& max)
        {
            float d;

            for (vec3_t& corner: corners) {
                corner -= center;

                d = dot(corner, axis_x);
                if (d > max.x) {
                    max.x = d;
                } else if (d < min.x) {
                    min.x = d;
                }

                d = dot(corner, axis_y);
                if (d > max.y) {
                    max.y = d;
                } else if (d < min.y) {
                    min.y = d;
                }

                d = dot(corner, axis_z);
                if (d > max.z) {
                    max.z = d;
                } else if (d < min.z) {
                    min.z = d;
                }
            }
        }
    }

    void bbox_t::merge(const bbox_t& other)
    {
        vec3_t axis_x, axis_y, axis_z;
        ce_quat_to_axes(&other.axis, &axis_x, &axis_y, &axis_z);

        axis_x *= other.aabb.extents.x;
        axis_y *= other.aabb.extents.y;
        axis_z *= other.aabb.extents.z;

        aabb_t that;
        that.origin = other.aabb.origin;

        // calculate the aabb extents in local coord space
        // from the other extents and axes
        that.extents.x = std::abs(axis_x.x) + std::abs(axis_y.x) + std::abs(axis_z.x);
        that.extents.y = std::abs(axis_x.y) + std::abs(axis_y.y) + std::abs(axis_z.y);
        that.extents.z = std::abs(axis_x.z) + std::abs(axis_y.z) + std::abs(axis_z.z);

        aabb.merge(that);
        aabb.update_radius();
        axis = CE_QUAT_IDENTITY;
    }

    void bbox_t::merge2(const bbox_t& other)
    {
        if (-std::numeric_limits<float>::max() == aabb.extents.x) {
            *this = other;
            return;
        }

        ce_quat new_axis;
        if (ce_quat_dot(&axis, &other.axis) < 0.0f) {
            ce_quat_neg(&new_axis, &other.axis);
        } else {
            ce_quat_copy(&new_axis, &other.axis);
        }

        ce_quat_add(&new_axis, &new_axis, &axis);
        ce_quat_norm(&new_axis, &new_axis);

        vec3_t axis_x, axis_y, axis_z;
        ce_quat_to_axes(&new_axis, &axis_x, &axis_y, &axis_z);

        vec3_t min, max, center = midpoint(aabb.origin, other.aabb.origin);
        std::array<vec3_t, 8> corners;

        compute_corners(*this, corners);
        ce_quat_to_axes(&axis, &axis_x, &axis_y, &axis_z);
        find_min_max(center, axis_x, axis_y, axis_z, corners, min, max);

        compute_corners(other, corners);
        ce_quat_to_axes(&other.axis, &axis_x, &axis_y, &axis_z);
        find_min_max(center, axis_x, axis_y, axis_z, corners, min, max);

        ce_quat_to_axes(&new_axis, &axis_x, &axis_y, &axis_z);

        axis_x *= 0.5f * (min.x + max.x);
        axis_y *= 0.5f * (min.y + max.y);
        axis_z *= 0.5f * (min.z + max.z);

        center += axis_x;
        center += axis_y;
        center += axis_z;

        aabb.extents = 0.5f * (max - min);
        aabb.origin = center;
        aabb.update_radius();
        axis = new_axis;
    }
}
