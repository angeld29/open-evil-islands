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
    typedef struct {
        float r, g, b, a;
    } ce_color;

    extern const ce_color CE_COLOR_BLACK;
    extern const ce_color CE_COLOR_WHITE;
    extern const ce_color CE_COLOR_RED;
    extern const ce_color CE_COLOR_GREEN;
    extern const ce_color CE_COLOR_BLUE;
    extern const ce_color CE_COLOR_YELLOW;
    extern const ce_color CE_COLOR_CYAN;
    extern const ce_color CE_COLOR_MAGENTA;
    extern const ce_color CE_COLOR_GRAY;
    extern const ce_color CE_COLOR_CORNFLOWER;
    extern const ce_color CE_COLOR_CRIMSON;
    extern const ce_color CE_COLOR_GOLD;
    extern const ce_color CE_COLOR_GOLDENROD;
    extern const ce_color CE_COLOR_INDIGO;
    extern const ce_color CE_COLOR_ROYAL_BLUE;
    extern const ce_color CE_COLOR_SILVER;
    extern const ce_color CE_COLOR_TOMATO;
    extern const ce_color CE_COLOR_WHEAT;
    extern const ce_color CE_COLOR_TRANSPARENT;

    ce_color* ce_color_init(ce_color* color, float r, float g, float b, float a);
    ce_color* ce_color_init_array(ce_color* color, const float* array);

    ce_color* ce_color_init_black(ce_color* color);
    ce_color* ce_color_init_white(ce_color* color);
    ce_color* ce_color_init_red(ce_color* color);
    ce_color* ce_color_init_green(ce_color* color);
    ce_color* ce_color_init_blue(ce_color* color);

    ce_color* ce_color_copy(ce_color* color, const ce_color* other);
    ce_color* ce_color_lerp(ce_color* color, float u, const ce_color* lhs, const ce_color* rhs);
}

#endif
