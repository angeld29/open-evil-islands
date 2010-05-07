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
#include "cedisplay.h"

ce_displaymode* ce_displaymode_new(int width, int height, int bpp, int rate)
{
	ce_displaymode* mode = ce_alloc(sizeof(ce_displaymode));
	mode->width = width;
	mode->height = height;
	mode->bpp = bpp;
	mode->rate = rate;
	return mode;
}

void ce_displaymode_del(ce_displaymode* mode)
{
	if (NULL != mode) {
		ce_free(mode, sizeof(ce_displaymode));
	}
}

ce_displaymng* ce_displaymng_new(ce_displaymng_vtable vtable, size_t size, ...)
{
	ce_displaymng* displaymng = ce_alloc(sizeof(ce_displaymng) + size);
	displaymng->modes = ce_vector_new();
	displaymng->rotation = CE_DISPLAY_ROTATION_NONE;
	displaymng->reflection = CE_DISPLAY_REFLECTION_NONE;
	displaymng->vtable = vtable;
	displaymng->size = size;
	va_list args;
	va_start(args, size);
	(*vtable.ctor)(displaymng, args);
	va_end(args);
	return displaymng;
}

void ce_displaymng_del(ce_displaymng* displaymng)
{
	if (NULL != displaymng) {
		(*displaymng->vtable.dtor)(displaymng);
		ce_vector_for_each(displaymng->modes, ce_displaymode_del);
		ce_vector_del(displaymng->modes);
		ce_free(displaymng, sizeof(ce_displaymng) + displaymng->size);
	}
}

void ce_displaymng_restore(ce_displaymng* displaymng)
{
	(*displaymng->vtable.restore)(displaymng);
}

void ce_displaymng_change(ce_displaymng* displaymng,
						int width, int height, int bpp, int rate,
						ce_display_rotation rotation,
						ce_display_reflection reflection)
{
	// FIXME: find best mode
	(*displaymng->vtable.change)(displaymng, 0, rotation, reflection);
}
