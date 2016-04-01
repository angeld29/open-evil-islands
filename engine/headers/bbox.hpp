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

#ifndef CE_BBOX_HPP
#define CE_BBOX_HPP

#include "aabb.hpp"
#include "quaternion.hpp"

namespace cursedearth
{
    /**
     * @brief Oriented Bounding Box
     *        aabb: axis-aligned bounding box
     *        axis: orientation axis
     */
    struct bbox_t
    {
        aabb_t aabb;
        quaternion_t axis;
    };

    bbox_t* ce_bbox_clear(bbox_t* bbox);
    bbox_t* ce_bbox_merge(bbox_t* bbox, const bbox_t* other);
}

#endif
