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

#include "aabb.hpp"

namespace cursedearth
{
    namespace
    {
        void merge_pass(float& origin, float& extent, float other_origin, float other_extent)
        {
            float displacement = other_origin - origin;
            float difference = other_extent - extent;
            if (extent < 0.0f || difference >= std::abs(displacement)) {
                // 2nd contains 1st
                extent = other_extent;
                origin = other_origin;
            } else if (other_extent < 0.0f || -difference >= std::abs(displacement)) {
                // 1st contains 2nd, do nothing
            } else {
                // not contained
                float min, max;
                if (displacement > 0.0f) {
                    min = origin - extent;
                    max = other_origin + other_extent;
                } else {
                    min = other_origin - other_extent;
                    max = origin + extent;
                }
                origin = 0.5f * (min + max);
                extent = max - origin;
            }
        }

        void merge_pass(float& origin, float& extent, float point)
        {
            float displacement = point - origin;
            if (std::abs(displacement) > extent) {
                float min, max;
                if (extent < 0.0f) { // degenerate
                    min = max = point;
                } else if (displacement > 0.0f) {
                    min = origin - extent;
                    max = origin + displacement;
                } else {
                    max = origin + extent;
                    min = origin + displacement;
                }
                origin = 0.5f * (min + max);
                extent = max - origin;
            }
        }
    }

    void aabb_t::clear()
    {
        origin = vec3_t::zero();
        extents = vec3_t(-std::numeric_limits<float>::max());
        radius = 0.0f;
    }

    void aabb_t::merge(const aabb_t& other)
    {
        merge_pass(origin.x, extents.x, other.origin.x, other.extents.x);
        merge_pass(origin.y, extents.y, other.origin.y, other.extents.y);
        merge_pass(origin.z, extents.z, other.origin.z, other.extents.z);
    }

    void aabb_t::merge(const vec3_t& point)
    {
        merge_pass(origin.x, extents.x, point.x);
        merge_pass(origin.y, extents.y, point.y);
        merge_pass(origin.z, extents.z, point.z);
    }

    void aabb_t::merge(const float array[3])
    {
        merge_pass(origin.x, extents.x, array[0]);
        merge_pass(origin.y, extents.y, array[1]);
        merge_pass(origin.z, extents.z, array[2]);
    }
}
