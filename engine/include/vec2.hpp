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

#ifndef CE_VEC2_HPP
#define CE_VEC2_HPP

namespace cursedearth
{
    struct vec2_t
    {
        float x = 0.0f, y = 0.0f;

        vec2_t() {}
        vec2_t(float x, float y): x(x), y(y) {}
        explicit vec2_t(float scalar): x(scalar), y(scalar) {}
        explicit vec2_t(const float array[2]): x(array[0]), y(array[1]) {}

        vec2_t& operator -()
        {
            x = -x;
            y = -y;
            return *this;
        }

        static vec2_t zero() { return { 0.0f, 0.0f }; }
        static vec2_t unit_x() { return { 1.0f, 0.0f }; }
        static vec2_t unit_y() { return { 0.0f, 1.0f }; }
        static vec2_t unit_scale() { return { 1.0f, 1.0f }; }
        static vec2_t neg_unit_x() { return { -1.0f, 0.0f }; }
        static vec2_t neg_unit_y() { return { 0.0f, -1.0f }; }
        static vec2_t neg_unit_scale() { return { -1.0f, -1.0f }; }
    };
}

#endif
