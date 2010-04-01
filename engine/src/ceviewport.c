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

#include "cealloc.h"
#include "ceviewport.h"

ce_viewport* ce_viewport_new(void)
{
	ce_viewport* viewport = ce_alloc(sizeof(ce_viewport));
	ce_viewport_set_rect(viewport, 0, 0, 100, 100);
	return viewport;
}

void ce_viewport_del(ce_viewport* viewport)
{
	ce_free(viewport, sizeof(ce_viewport));
}

void ce_viewport_set_corner(ce_viewport* viewport, int x, int y)
{
	viewport->x = x;
	viewport->y = y;
}

void ce_viewport_set_dimensions(ce_viewport* viewport, int width, int height)
{
	viewport->width = width;
	viewport->height = height;
}

void ce_viewport_set_rect(ce_viewport* viewport,
							int x, int y, int width, int height)
{
	ce_viewport_set_corner(viewport, x, y);
	ce_viewport_set_dimensions(viewport, width, height);
}
