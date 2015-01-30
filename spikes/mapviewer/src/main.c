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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "celib.h"
#include "cemath.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceoptionmanager.h"
#include "cefiguremanager.h"
#include "cemobloader.h"
#include "ceroot.h"
#include "cecamfile.h"

static ce_optparse* optparse;

static ce_input_supply* input_supply;
static ce_input_event* anmfps_inc_event;
static ce_input_event* anmfps_dec_event;

static float message_timeout;
static ce_color message_color;

static void clean()
{
    ce_input_supply_del(input_supply);
    ce_optparse_del(optparse);
}

static void state_changed(void* CE_UNUSED(listener), int state)
{
    if (CE_SCENEMNG_STATE_READY == state) {
        const char* zone;
        ce_optparse_get(optparse, "zone", &zone);
        ce_scenemng_load_mpr(ce_root.scenemng, zone);
        ce_mob_loader_load_mob(zone);
        ce_scenemng_change_state(ce_root.scenemng, CE_SCENEMNG_STATE_LOADING);
    }

    if (CE_SCENEMNG_STATE_PLAYING == state) {
        // play random animations
        srand(time(NULL));
        for (size_t i = 0; i < ce_figure_manager->entities->count; ++i) {
            ce_figentity* figentity = ce_figure_manager->entities->items[i];
            int anm_count = ce_figentity_get_animation_count(figentity);
            if (anm_count > 0) {
                const char* name = ce_figentity_get_animation_name(figentity,
                                                        rand() % anm_count);
                ce_figentity_play_animation(figentity, name);
            }
        }

        if (NULL != ce_root.scenemng->terrain) {
            ce_vec3 position;
            ce_camera_set_position(ce_root.scenemng->camera, ce_vec3_init(&position,
                0.0f, ce_root.scenemng->terrain->mprfile->max_y, 0.0f));
            ce_camera_yaw_pitch(ce_root.scenemng->camera, ce_deg2rad(45.0f),
                                                            ce_deg2rad(30.0f));
        }
    }
}

static void advance(void* CE_UNUSED(listener), float elapsed)
{
    ce_input_supply_advance(input_supply, elapsed);

    float animation_fps = ce_root.animation_fps;

    if (anmfps_inc_event->triggered) animation_fps += 1.0f;
    if (anmfps_dec_event->triggered) animation_fps -= 1.0f;

    if (message_timeout > 0.0f) {
        message_timeout -= elapsed;
    }

    if (animation_fps != ce_root.animation_fps) {
        message_timeout = 3.0f;
    }

    message_color.a = ce_clamp(float, message_timeout, 0.0f, 1.0f);
    ce_root.animation_fps = ce_clamp(float, animation_fps, 1.0f, 50.0f);
}

static void render(void* CE_UNUSED(listener))
{
    if (message_timeout > 0.0f) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Animation FPS: %d", (int)ce_root.animation_fps);
        ce_font_render(ce_root.scenemng->font, (ce_root.scenemng->viewport->width -
            ce_font_get_width(ce_root.scenemng->font, buffer)) / 2,
            1 * (ce_root.scenemng->viewport->height -
            ce_font_get_height(ce_root.scenemng->font)) / 5,
            &message_color, buffer);
    }
}

int main(int argc, char* argv[])
{
    ce_alloc_init();
    atexit(clean);

    optparse = ce_option_manager_create_option_parser();

    ce_optparse_set_standard_properties(optparse, CE_SPIKE_VERSION_MAJOR,
        CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
        "Cursed Earth: Map Viewer", "This program is part of Cursed Earth "
        "spikes\nMap Viewer - explore Evil Islands zones with creatures");

    ce_optparse_add(optparse, "zone", CE_TYPE_STRING, NULL, true,
        NULL, NULL, "any ZONE.mpr file in `EI/Maps'");

    ce_optparse_add_control(optparse, "+/-", "change animation FPS");

    if (!ce_root_init(optparse, argc, argv)) {
        return EXIT_FAILURE;
    }

    const char* ei_path;
    ce_optparse_get(optparse, "ei_path", &ei_path);

#if 0
    const char* zone;
    ce_optparse_get(optparse, "zone", &zone);

    char path[512];
    snprintf(path, sizeof(path), "%s/Camera/%s.cam",
        ei_path, zone /*"mainmenu"*/ /*"camera01"*/);

    ce_mem_file* mem_file = ce_mem_file_new_path(path);
    if (NULL != mem_file) {
        ce_cam_file* cam_file = ce_cam_file_new(mem_file);

        for (size_t i = 0; i < cam_file->record_count; ++i) {
            printf("%u %u\n", cam_file->records[i].time, cam_file->records[i].unknown);
            printf("%f %f %f\n", cam_file->records[i].position[0], cam_file->records[i].position[1], cam_file->records[i].position[2]);
            printf("%f %f %f %f\n", cam_file->records[i].rotation[0], cam_file->records[i].rotation[1], cam_file->records[i].rotation[2], cam_file->records[i].rotation[3]);
            printf("-----\n");
        }

        ce_vec3 position;
        ce_vec3_init_array(&position, cam_file->records[0].position);
        ce_swap_temp(float, &position.z, &position.y);
        position.z = -position.z;

        ce_camera_set_position(ce_root.scenemng->camera, &position);

        ce_quat orientation, temp, temp2, temp3;
        ce_quat_init_array(&temp, cam_file->records[0].rotation);

        ce_quat_init_polar(&temp2, ce_deg2rad(90), &CE_VEC3_UNIT_X);
        ce_quat_mul(&temp3, &temp2, &temp);

        ce_quat_init_polar(&temp2, ce_deg2rad(180), &CE_VEC3_UNIT_Y);
        ce_quat_mul(&orientation, &temp2, &temp3);

        ce_quat_conj(&orientation, &orientation);

        ce_camera_set_orientation(ce_root.scenemng->camera, &orientation);
        ce_camera_set_orientation(ce_root.scenemng->camera, &temp);

        ce_cam_file_del(cam_file);
        ce_mem_file_del(mem_file);
    } else {
        ce_logging_error("map viewer: could not open file `%s'", path);
    }
#endif

    ce_root.scenemng->listener = (ce_scenemng_listener)
        {.state_changed = state_changed, .advance = advance, .render = render};

    message_color = CE_COLOR_CORNFLOWER;

    input_supply = ce_input_supply_new(ce_root.renderwindow->input_context);
    anmfps_inc_event = ce_input_supply_repeat(input_supply,
                        ce_input_supply_button(input_supply,
                            CE_KB_ADD), CE_INPUT_DEFAULT_DELAY, 10);
    anmfps_dec_event = ce_input_supply_repeat(input_supply,
                        ce_input_supply_button(input_supply,
                            CE_KB_SUBTRACT), CE_INPUT_DEFAULT_DELAY, 10);

    return ce_root_exec();
}
