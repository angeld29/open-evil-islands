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

#include <assert.h>

#include "cealloc.h"
#include "cerenderwindow.h"

ce_renderwindow_mode*
ce_renderwindow_mode_new(int width, int height, int bpp, int rate)
{
	ce_renderwindow_mode* mode = ce_alloc(sizeof(ce_renderwindow_mode));
	mode->width = width;
	mode->height = height;
	mode->bpp = bpp;
	mode->rate = rate;
	return mode;
}

void ce_renderwindow_mode_del(ce_renderwindow_mode* mode)
{
	if (NULL != mode) {
		ce_free(mode, sizeof(ce_renderwindow_mode));
	}
}

ce_renderwindow_modemng*
ce_renderwindow_modemng_new(ce_renderwindow_modemng_vtable vtable, size_t size, ...)
{
	ce_renderwindow_modemng* modemng = ce_alloc(sizeof(ce_renderwindow_modemng) + size);
	modemng->modes = ce_vector_new();
	modemng->vtable = vtable;
	modemng->size = size;
	va_list args;
	va_start(args, size);
	(*vtable.ctor)(modemng, args);
	va_end(args);
	return modemng;
}

void ce_renderwindow_modemng_del(ce_renderwindow_modemng* modemng)
{
	if (NULL != modemng) {
		(*modemng->vtable.dtor)(modemng);
		ce_vector_for_each(modemng->modes, ce_renderwindow_mode_del);
		ce_vector_del(modemng->modes);
		ce_free(modemng, sizeof(ce_renderwindow_modemng) + modemng->size);
	}
}

void ce_renderwindow_modemng_change(ce_renderwindow_modemng* modemng, int index)
{
	(*modemng->vtable.change)(modemng, index);
}
