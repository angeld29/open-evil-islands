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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <exception>

#include "alloc.hpp"
#include "logging.hpp"
#include "optionmanager.hpp"
#include "soundobject.hpp"
#include "root.hpp"

using namespace cursedearth;

static color_t message_color;
static float alpha_sign = -1.0f;
static sound_object_t sound_object;
static sound_object_t lightning_object;
static ce_optparse* optparse;
static input_supply_ptr_t input_supply;
static input_event_const_ptr_t lightning_event;

static void clear()
{
    ce_optparse_del(optparse);
}

static void state_changed(void*, int state)
{
    if (CE_SCENEMNG_STATE_READY == state) {
        const char* track;
        ce_optparse_get(optparse, "track", &track);

        sound_object = make_sound_object(track);
        if (0 == sound_object) {
            ce_logging_error("sound player: could not play sound track `%s'", track);
        } else {
            play_sound_object(sound_object);
        }

        ce_scenemng_change_state(ce_root::instance()->scenemng, CE_SCENEMNG_STATE_LOADING);
    }
}

static void advance(void*, float elapsed)
{
    input_supply->advance(elapsed);
    sound_object_advance(sound_object, elapsed);

    if (lightning_event->triggered()) {
        lightning_object = make_sound_object("magic\\Lightning\\start.wav");
        play_sound_object(lightning_object);
    }

    if (sound_object_is_valid(sound_object) && !sound_object_is_stopped(sound_object)) {
        message_color.a += elapsed * alpha_sign;
        alpha_sign = message_color.a < 0.25f ? 1.0f : (message_color.a > 1.0f ? -1.0f : alpha_sign);
    } else {
        message_color.a = 1.0f;
    }
}

static void render(void*)
{
    if (CE_SCENEMNG_STATE_PLAYING != ce_root::instance()->scenemng->state) {
        return;
    }

    char message[32];
    const char* track;
    ce_optparse_get(optparse, "track", &track);

    if (sound_object_is_valid(sound_object)) {
        if (sound_object_is_stopped(sound_object)) {
            snprintf(message, sizeof(message), "Track `%s' stopped", track);
        } else {
            snprintf(message, sizeof(message), "Playing track `%s'", track);
        }
    } else {
        snprintf(message, sizeof(message), "Unable to play track `%s'", track);
    }

    ce_font_render(ce_root::instance()->scenemng->font,
        (ce_root::instance()->scenemng->viewport->width - ce_font_get_width(ce_root::instance()->scenemng->font, message)) / 2,
        1 * (ce_root::instance()->scenemng->viewport->height - ce_font_get_height(ce_root::instance()->scenemng->font)) / 5,
        &message_color, message);
}

int main(int argc, char* argv[])
{
    ce_alloc_init();
    atexit(clear);

    try {
        optparse = ce_option_manager_create_option_parser();

        ce_optparse_set_standard_properties(optparse, CE_SPIKE_VERSION_MAJOR, CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
            "Cursed Earth: Sound Player", "This program is part of Cursed Earth spikes.\nSound Player - play Evil Islands sounds.");

        ce_optparse_add(optparse, "track", CE_TYPE_STRING, NULL, true, NULL, NULL, "any TRACK.* file in `EI/Stream'");

        ce_root root(optparse, argc, argv);

        message_color = CE_COLOR_CORNFLOWER;

        ce_root::instance()->scenemng->listener.state_changed = state_changed;
        ce_root::instance()->scenemng->listener.advance = advance;
        ce_root::instance()->scenemng->listener.render = render;

        input_supply = std::make_shared<input_supply_t>(ce_root::instance()->renderwindow->input_context());
        lightning_event = input_supply->single_front(input_supply->push(input_button_t::kb_l));

        return ce_root::instance()->exec();
    } catch (const std::exception& error) {
        ce_logging_fatal("sound player: %s", error.what());
    }
    return EXIT_FAILURE;
}
