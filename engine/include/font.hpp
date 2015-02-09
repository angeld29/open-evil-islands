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

#ifndef CE_FONT_HPP
#define CE_FONT_HPP

#include <string>

#include "color.hpp"

namespace cursedearth
{
    typedef struct ce_font ce_font;

    ce_font* ce_font_new(const char* resource_path, int pixel_size);
    void ce_font_del(ce_font*);

    int ce_font_get_height(ce_font*);
    int ce_font_get_width(ce_font*, const std::string& text);

    void ce_font_render(ce_font*, int x, int y, const color_t*, const std::string& text);
}

#endif
