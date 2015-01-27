/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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

#ifndef CE_FONT_H
#define CE_FONT_H

#include "cecolor.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_font ce_font;

extern ce_font* ce_font_new(const char* resource_path, int pixel_size);
extern void ce_font_del(ce_font* font);

extern int ce_font_get_height(ce_font* font);
extern int ce_font_get_width(ce_font* font, const char* text);

extern void ce_font_render(ce_font* font, int x, int y,
                            const ce_color* color, const char* text);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FONT_H */
