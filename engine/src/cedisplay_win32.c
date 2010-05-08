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

#include <stdio.h>
#include <assert.h>

#include <windows.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cedisplay.h"

#include "cedisplay_win32.h"

typedef struct {
	ce_vector* modes;
	DEVMODE orig_mode;
} ce_dmmng;

static void ce_dmmng_ctor(ce_displaymng* displaymng, va_list args)
{
	ce_unused(args);

	ce_logging_write("display: using native Windows API");

	ce_dmmng* dmmng = (ce_dmmng*)displaymng->impl;
	dmmng->modes = ce_vector_new();

	ZeroMemory(&dmmng->orig_mode, sizeof(DEVMODE));
	dmmng->orig_mode.dmSize = sizeof(DEVMODE);

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmmng->orig_mode);

	DEVMODE mode;
	DWORD mode_index = 0;

	ZeroMemory(&mode, sizeof(DEVMODE));
	mode.dmSize = sizeof(DEVMODE);

	while (EnumDisplaySettings(NULL, mode_index++, &mode)) {
		if (!(mode.dmFields & DM_PELSWIDTH &&
				mode.dmFields & DM_PELSHEIGHT &&
				mode.dmFields & DM_BITSPERPEL &&
				mode.dmFields & DM_DISPLAYFREQUENCY)) {
			continue;
		}

		ce_vector_push_back(dmmng->modes, ce_alloc(sizeof(DEVMODE)));
		*(DEVMODE*)ce_vector_back(dmmng->modes) = mode;

		ce_vector_push_back(displaymng->modes,
			ce_displaymode_new(mode.dmPelsWidth, mode.dmPelsHeight,
				mode.dmBitsPerPel, mode.dmDisplayFrequency));
	}
}

static void ce_dmmng_restore(ce_displaymng* displaymng)
{
	ce_dmmng* dmmng = (ce_dmmng*)displaymng->impl;

	ChangeDisplaySettingsEx(NULL, &dmmng->orig_mode, NULL, 0, NULL);
}

static void ce_dmmng_dtor(ce_displaymng* displaymng)
{
	ce_dmmng* dmmng = (ce_dmmng*)displaymng->impl;

	ce_dmmng_restore(displaymng);

	for (int i = 0; i < dmmng->modes->count; ++i) {
		ce_free(dmmng->modes->items[i], sizeof(DEVMODE));
	}
	ce_vector_del(dmmng->modes);
}

static void ce_dmmng_change(ce_displaymng* displaymng, int index,
	ce_display_rotation rotation, ce_display_reflection reflection)
{
	ce_unused(rotation), ce_unused(reflection);

	ce_dmmng* dmmng = (ce_dmmng*)displaymng->impl;
	DEVMODE* mode = dmmng->modes->items[index];

	LONG code = ChangeDisplaySettingsEx(NULL, mode, NULL, CDS_FULLSCREEN, NULL);
	if (DISP_CHANGE_SUCCESSFUL != code) {
		ce_logging_error("display: could not change display settings");
	}
}

ce_displaymng* ce_displaymng_create(void)
{
	ce_displaymng_vtable vtable = {
		ce_dmmng_ctor, ce_dmmng_dtor, ce_dmmng_restore, ce_dmmng_change
	};
	return ce_displaymng_new(vtable, sizeof(ce_dmmng));
}
