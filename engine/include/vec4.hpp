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

#ifndef CE_VEC4_HPP
#define CE_VEC4_HPP

namespace cursedearth
{
    struct vec4_t
    {
        float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

        vec4_t() {}
        vec4_t(float x, float y, float z, float w): x(x), y(y), z(z), w(w) {}
        explicit vec4_t(float scalar): x(scalar), y(scalar), z(scalar), w(scalar) {}
        explicit vec4_t(const float array[4]): x(array[0]), y(array[1]), z(array[2]), w(array[3]) {}
    };
}

#endif
