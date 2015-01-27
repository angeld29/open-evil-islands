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

#include <stdlib.h>
#include <stdio.h>

#include "cemath.h"
#include "cealloc.h"
#include "ceoptionmanager.h"
#include "ceroot.h"

static ce_optparse* optparse;

static void clean()
{
    ce_optparse_del(optparse);
}

static void state_changed(void* CE_UNUSED(listener), int state)
{
    if (CE_SCENEMNG_STATE_READY == state) {
        const char* zone;
        ce_optparse_get(optparse, "zone", &zone);
        ce_scenemng_load_mpr(ce_root.scenemng, zone);
        ce_scenemng_change_state(ce_root.scenemng, CE_SCENEMNG_STATE_LOADING);
    }

    if (CE_SCENEMNG_STATE_PLAYING == state) {
        if (NULL != ce_root.scenemng->terrain) {
            ce_vec3 position;
            ce_camera_set_position(ce_root.scenemng->camera, ce_vec3_init(&position,
                0.0f, ce_root.scenemng->terrain->mprfile->max_y, 0.0f));
            ce_camera_yaw_pitch(ce_root.scenemng->camera, ce_deg2rad(45.0f),
                                                            ce_deg2rad(30.0f));
        }
    }
}

int main(int argc, char* argv[])
{
    ce_alloc_init();
    atexit(clean);

    optparse = ce_option_manager_create_option_parser();

    ce_optparse_set_standard_properties(optparse, CE_SPIKE_VERSION_MAJOR,
        CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
        "Cursed Earth: MPR Viewer", "This program is part of Cursed "
        "Earth spikes\nMPR Viewer - explore clean Evil Islands zones");

    ce_optparse_add(optparse, "zone", CE_TYPE_STRING, NULL, true,
        NULL, NULL, "any ZONE.mpr file in 'EI/Maps'");

    if (!ce_root_init(optparse, argc, argv)) {
        return EXIT_FAILURE;
    }

    ce_root.scenemng->listener = (ce_scenemng_listener)
        {.state_changed = state_changed, .advance = NULL, .render = NULL};

    return ce_root_exec();
}
