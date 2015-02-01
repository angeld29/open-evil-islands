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

#ifndef CE_AABB_HPP
#define CE_AABB_HPP

#include "vec3.hpp"

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
        vec3_t origin, extents;
        float radius = 0.0f;

        void clear();

        void merge(const aabb_t& other);
        void merge(const vec3_t& point);
        void merge(const float array[3]);

        void update_radius()
        {
            radius = extents.length();
        }
    };
}

#endif
