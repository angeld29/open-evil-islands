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

#ifndef CE_VIEWPORT_HPP
#define CE_VIEWPORT_HPP

typedef struct {
    int x, y; // lower left corner in pixels
    int width, height;
} ce_viewport;

extern ce_viewport* ce_viewport_new(void);
extern void ce_viewport_del(ce_viewport* viewport);

extern void ce_viewport_set_corner(ce_viewport* viewport, int x, int y);
extern void ce_viewport_set_dimensions(ce_viewport* viewport,
                                        int width, int height);
extern void ce_viewport_set_rect(ce_viewport* viewport,
                                    int x, int y, int width, int height);

#endif /* CE_VIEWPORT_HPP */
