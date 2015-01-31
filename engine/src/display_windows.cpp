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

#include <cstdio>

#include <windows.h>

#include "alloc.hpp"
#include "logging.hpp"
#include "display.hpp"
#include "display_windows.hpp"

namespace cursedearth
{
typedef struct {
    ce_vector* modes;
    DEVMODE orig_mode;
} ce_dmmng;

static void ce_dmmng_change_display_settings(DEVMODE* dm, DWORD flags)
{
    LONG code = ChangeDisplaySettingsEx(NULL, dm, NULL, flags, NULL);

    switch (code) {
    case DISP_CHANGE_BADDUALVIEW:
        ce_logging_error("displaymng: the settings change was unsuccessful "
                                    "because the system is DualView capable");
        break;
    case DISP_CHANGE_BADFLAGS:
        ce_logging_error("displaymng: an invalid set of flags was passed in");
        break;
    case DISP_CHANGE_BADMODE:
        ce_logging_error("displaymng: the graphics mode is not supported");
        break;
    case DISP_CHANGE_BADPARAM:
        ce_logging_error("displaymng: an invalid parameter was passed in; this "
                        "can include an invalid flag or combination of flags");
        break;
    case DISP_CHANGE_FAILED:
        ce_logging_error("displaymng: the display driver failed "
                                    "the specified graphics mode");
        break;
    case DISP_CHANGE_NOTUPDATED:
        ce_logging_error("displaymng: unable to write settings to the registry");
        break;
    case DISP_CHANGE_RESTART:
        ce_logging_error("displaymng: the computer must be restarted "
                                        "for the graphics mode to work");
        break;
    }

    if (DISP_CHANGE_SUCCESSFUL != code) {
        ce_logging_error("displaymng: could not change display settings");
    }
}

static void ce_dmmng_ctor(ce_displaymng* displaymng, va_list /*args*/)
{
    ce_logging_write("displaymng: using native Device Context Windows API");

    ce_dmmng* dmmng = (ce_dmmng*)displaymng->impl;
    dmmng->modes = ce_vector_new();

    ZeroMemory(&dmmng->orig_mode, sizeof(DEVMODE));
    dmmng->orig_mode.dmSize = sizeof(DEVMODE);

    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmmng->orig_mode);

    DEVMODE mode;
    ZeroMemory(&mode, sizeof(DEVMODE));
    mode.dmSize = sizeof(DEVMODE);

    for (DWORD i = 0; EnumDisplaySettings(NULL, i, &mode); ++i) {
        if (!(mode.dmFields & DM_PELSWIDTH &&
                mode.dmFields & DM_PELSHEIGHT &&
                mode.dmFields & DM_BITSPERPEL &&
                mode.dmFields & DM_DISPLAYFREQUENCY)) {
            continue;
        }

        ce_vector_push_back(dmmng->modes, ce_alloc(sizeof(DEVMODE)));
        *(DEVMODE*)ce_vector_back(dmmng->modes) = mode;

        ce_vector_push_back(displaymng->supported_modes,
            ce_displaymode_new(mode.dmPelsWidth, mode.dmPelsHeight,
                mode.dmBitsPerPel, mode.dmDisplayFrequency));
    }
}

static void ce_dmmng_enter(ce_displaymng* displaymng, size_t index, ce_display_rotation, ce_display_reflection)
{
    ce_dmmng* dmmng = (ce_dmmng*)displaymng->impl;
    DEVMODE* mode = dmmng->modes->items[index];

    ce_dmmng_change_display_settings(mode, CDS_FULLSCREEN);
}

static void ce_dmmng_exit(ce_displaymng* displaymng)
{
    ce_dmmng* dmmng = (ce_dmmng*)displaymng->impl;

    ce_dmmng_change_display_settings(&dmmng->orig_mode, 0);
}

static void ce_dmmng_dtor(ce_displaymng* displaymng)
{
    ce_dmmng* dmmng = (ce_dmmng*)displaymng->impl;

    ce_dmmng_exit(displaymng);

    for (size_t i = 0; i < dmmng->modes->count; ++i) {
        ce_free(dmmng->modes->items[i], sizeof(DEVMODE));
    }
    ce_vector_del(dmmng->modes);
}

ce_displaymng* ce_displaymng_create(void)
{
    ce_displaymng_vtable vtable = {
        ce_dmmng_ctor, ce_dmmng_dtor, ce_dmmng_enter, ce_dmmng_exit
    };
    return ce_displaymng_new(vtable, sizeof(ce_dmmng));
}
}
