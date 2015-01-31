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
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "celib.h"
#include "cestr.h"
#include "cemath.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceoptionmanager.h"
#include "cefiguremanager.h"
#include "ceroot.h"

static ce_optparse* optparse;
static ce_figentity* figentity;
static ce_string* message;

static ce_input_supply* input_supply;
static ce_input_event* strength_event;
static ce_input_event* dexterity_event;
static ce_input_event* height_event;
static ce_input_event* anm_change_event;
static ce_input_event* anmfps_inc_event;
static ce_input_event* anmfps_dec_event;

static int anmidx = -1;
static ce_complection complection = {1.0f, 1.0f, 1.0f};

static float message_timeout;
static ce_color message_color;

static void clear()
{
    ce_input_supply_del(input_supply);
    ce_string_del(message);
    ce_optparse_del(optparse);
}

static bool update_figentity()
{
    ce_figure_manager_remove_entity(figentity);

    ce_vec3 position = CE_VEC3_ZERO;

    ce_quat orientation, q1, q2;
    ce_quat_init_polar(&q1, ce_deg2rad(180.0f), &CE_VEC3_UNIT_Z);
    ce_quat_init_polar(&q2, ce_deg2rad(270.0f), &CE_VEC3_UNIT_X);
    ce_quat_mul(&orientation, &q2, &q1);

    const char *parts[1] = {NULL}, *textures[3] = {[2] = NULL}, *figure;
    ce_optparse_get(optparse, "pritex", &textures[0]);
    ce_optparse_get(optparse, "sectex", &textures[1]);
    ce_optparse_get(optparse, "figure", &figure);

    figentity = ce_figure_manager_create_entity(figure, &complection, &position, &orientation, parts, textures);

    if (NULL == figentity) {
        return false;
    }

    ce_scenenode_attach_child(ce_root.scenemng->scenenode, figentity->scenenode);

    if (-1 != anmidx) {
        ce_figentity_play_animation(figentity, ce_figentity_get_animation_name(figentity, anmidx));
    }

    return true;
}

static void display_message(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ce_string_assign_va(message, fmt, args);
    va_end(args);

    message_timeout = 3.0f;
    message_color.a = 1.0f;
}

static void state_changed(void* CE_UNUSED(listener), int state)
{
    if (CE_SCENEMNG_STATE_READY == state) {
        if (update_figentity()) {
            const char* anmname;
            ce_optparse_get(optparse, "anmname", &anmname);

            if (NULL != anmname) {
                if (ce_figentity_play_animation(figentity, anmname)) {
                    int anmcount = ce_figentity_get_animation_count(figentity);
                    for (anmidx = 0; anmidx < anmcount; ++anmidx) {
                        if (0 == ce_strcasecmp(anmname, ce_figentity_get_animation_name(figentity, anmidx))) {
                            break;
                        }
                    }
                } else {
                    ce_logging_warning("figure viewer: could not play animation `%s'", anmname);
                }
            }
        }

        ce_vec3 position;
        ce_camera_set_position(ce_root.scenemng->camera, ce_vec3_init(&position, 0.0f, 2.0f, -4.0f));

        ce_camera_set_near(ce_root.scenemng->camera, 0.1f);
        ce_camera_yaw_pitch(ce_root.scenemng->camera, ce_deg2rad(180.0f), ce_deg2rad(30.0f));

        ce_root.scenemng->camera_move_sensitivity = 2.5f;
        ce_root.scenemng->camera_zoom_sensitivity = 0.5f;

        ce_scenemng_change_state(ce_root.scenemng, CE_SCENEMNG_STATE_LOADING);
    }
}

static void advance(void* CE_UNUSED(listener), float elapsed)
{
    ce_input_supply_advance(input_supply, elapsed);

    if (message_timeout > 0.0f) {
        message_timeout -= elapsed;
        message_color.a = ce_clamp(float, message_timeout, 0.0f, 1.0f);
    }

    float animation_fps = ce_root.animation_fps;

    if (anmfps_inc_event->triggered) animation_fps += 1.0f;
    if (anmfps_dec_event->triggered) animation_fps -= 1.0f;

    if (animation_fps != ce_root.animation_fps) {
        ce_root.animation_fps = ce_clamp(float, animation_fps, 1.0f, 50.0f);
        display_message("Animation FPS: %d", (int)ce_root.animation_fps);
    }

    bool need_update_figentity = false;

    if (strength_event->triggered) {
        if ((complection.strength += 0.1f) >= 1.1f) {
            complection.strength = 0.0f;
        }
        need_update_figentity = true;
        display_message("Strength: %.2f", complection.strength);
    }

    if (dexterity_event->triggered) {
        if ((complection.dexterity += 0.1f) >= 1.1f) {
            complection.dexterity = 0.0f;
        }
        need_update_figentity = true;
        display_message("Dexterity: %.2f", complection.dexterity);
    }

    if (height_event->triggered) {
        if ((complection.height += 0.1f) >= 1.1f) {
            complection.height = 0.0f;
        }
        need_update_figentity = true;
        display_message("Height: %.2f", complection.height);
    }

    if (need_update_figentity) {
        update_figentity();
    }

    if (anm_change_event->triggered) {
        ce_figentity_stop_animation(figentity);
        int anm_count = ce_figentity_get_animation_count(figentity);
        if (++anmidx == anm_count) {
            anmidx = -1;
        }
        if (-1 != anmidx) {
            const char* anmname = ce_figentity_get_animation_name(figentity, anmidx);
            ce_figentity_play_animation(figentity, anmname);
            display_message("Animation name: %s", anmname);
        } else {
            display_message("No animation");
        }
    }
}

static void render(void* CE_UNUSED(listener))
{
    if (message_timeout > 0.0f) {
        ce_font_render(ce_root.scenemng->font,
            (ce_root.scenemng->viewport->width - ce_font_get_width(ce_root.scenemng->font, message->str)) / 2,
            1 * (ce_root.scenemng->viewport->height - ce_font_get_height(ce_root.scenemng->font)) / 5,
            &message_color, message->str);
    }
}

int main(int argc, char* argv[])
{
    ce_alloc_init();
    atexit(clear);

    optparse = ce_option_manager_create_option_parser();

    ce_optparse_set_standard_properties(optparse, CE_SPIKE_VERSION_MAJOR, CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
        "Cursed Earth: Figure Viewer", "This program is part of Cursed Earth spikes.\nFigure Viewer - explore Evil Islands figures.");

    ce_optparse_add(optparse, "pritex", CE_TYPE_STRING, "default0", false, NULL, "primary-texture", "primary texture");
    ce_optparse_add(optparse, "sectex", CE_TYPE_STRING, "default0", false, NULL, "secondary-texture", "secondary texture");
    ce_optparse_add(optparse, "anmname", CE_TYPE_STRING, NULL, false, NULL, "animation-name", "play animation with specified name");
    ce_optparse_add(optparse, "figure", CE_TYPE_STRING, NULL, true, NULL, NULL, "internal figure name");

    ce_optparse_add_control(optparse, "+/-", "change animation FPS");
    ce_optparse_add_control(optparse, "a", "play next animation");
    ce_optparse_add_control(optparse, "1", "change strength");
    ce_optparse_add_control(optparse, "2", "change dexterity");
    ce_optparse_add_control(optparse, "3", "change height");

    if (!ce_root_init(optparse, argc, argv)) {
        return EXIT_FAILURE;
    }

    ce_root.scenemng->listener = (ce_scenemng_listener){.state_changed = state_changed, .advance = advance, .render = render};

    message = ce_string_new();
    message_color = CE_COLOR_CORNFLOWER;

    input_supply = ce_input_supply_new(ce_root.renderwindow->input_context);
    strength_event = ce_input_supply_single_front(input_supply, ce_input_supply_button(input_supply, CE_KB_1));
    dexterity_event = ce_input_supply_single_front(input_supply, ce_input_supply_button(input_supply, CE_KB_2));
    height_event = ce_input_supply_single_front(input_supply, ce_input_supply_button(input_supply, CE_KB_3));
    anm_change_event = ce_input_supply_single_front(input_supply, ce_input_supply_button(input_supply, CE_KB_A));
    anmfps_inc_event = ce_input_supply_repeat(input_supply, ce_input_supply_button(input_supply, CE_KB_ADD), CE_INPUT_DEFAULT_DELAY, 10);
    anmfps_dec_event = ce_input_supply_repeat(input_supply, ce_input_supply_button(input_supply, CE_KB_SUBTRACT), CE_INPUT_DEFAULT_DELAY, 10);

    return ce_root_exec();
}
