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

#include "vec3.hpp"
#include "segment.hpp"

namespace cursedearth
{
    struct ray_t
    {
        vec3_t origin, direction;

        ray_t() {}
        ray_t(const vec3_t& origin, const vec3_t& direction): origin(origin), direction(direction) {}
        explicit ray_t(const segment_t& segment): origin(segment.start), direction(segment.end - segment.start)
        {
            direction.normalize();
        }
    };
}

#endif
