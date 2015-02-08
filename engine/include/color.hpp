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

#ifndef CE_COLOR_HPP
#define CE_COLOR_HPP

namespace cursedearth
{
    struct color_t
    {
        float r, g, b, a;
    };

    extern const color_t CE_COLOR_BLACK;
    extern const color_t CE_COLOR_WHITE;
    extern const color_t CE_COLOR_RED;
    extern const color_t CE_COLOR_GREEN;
    extern const color_t CE_COLOR_BLUE;
    extern const color_t CE_COLOR_YELLOW;
    extern const color_t CE_COLOR_CYAN;
    extern const color_t CE_COLOR_MAGENTA;
    extern const color_t CE_COLOR_GRAY;
    extern const color_t CE_COLOR_CORNFLOWER;
    extern const color_t CE_COLOR_CRIMSON;
    extern const color_t CE_COLOR_GOLD;
    extern const color_t CE_COLOR_GOLDENROD;
    extern const color_t CE_COLOR_INDIGO;
    extern const color_t CE_COLOR_ROYAL_BLUE;
    extern const color_t CE_COLOR_SILVER;
    extern const color_t CE_COLOR_TOMATO;
    extern const color_t CE_COLOR_WHEAT;
    extern const color_t CE_COLOR_TRANSPARENT;

    color_t* ce_color_init(color_t* color, float r, float g, float b, float a);
    color_t* ce_color_init_array(color_t* color, const float* array);

    color_t* ce_color_init_black(color_t* color);
    color_t* ce_color_init_white(color_t* color);
    color_t* ce_color_init_red(color_t* color);
    color_t* ce_color_init_green(color_t* color);
    color_t* ce_color_init_blue(color_t* color);

    color_t* ce_color_copy(color_t* color, const color_t* other);
    color_t* ce_color_lerp(color_t* color, float u, const color_t* lhs, const color_t* rhs);
}

#endif
