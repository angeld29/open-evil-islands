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

#include "alloc.hpp"
#include "logging.hpp"
#include "optionmanager.hpp"
#include "videoobject.hpp"
#include "root.hpp"

using namespace cursedearth;

static bool video_paused;
static video_object_t video_object;
static ce_optparse* optparse;
static input_supply_ptr_t input_supply;
static input_event_const_ptr_t pause_event;

static void clear()
{
    ce_optparse_del(optparse);
}

static void state_changed(void*, int state)
{
    if (CE_SCENEMNG_STATE_READY == state) {
        const char* track;
        ce_optparse_get(optparse, "track", &track);

        video_object = make_video_object(track);
        if (0 == video_object) {
            ce_logging_error("video player: could not play video track `%s'", track);
        } else {
            play_video_object(video_object);
        }

        ce_scenemng_change_state(ce_root::instance()->scenemng, CE_SCENEMNG_STATE_LOADING);
    }
}

static void advance(void*, float elapsed)
{
    input_supply->advance(elapsed);
    video_object_advance(video_object, elapsed);

    if (pause_event->triggered()) {
        video_paused = !video_paused;
        if (video_paused) {
            pause_video_object(video_object);
        } else {
            play_video_object(video_object);
        }
    }
}

static void render(void*)
{
    video_object_render(video_object);
}

int main(int argc, char* argv[])
{
    ce_alloc_init();
    atexit(clear);

    try {
        optparse = ce_option_manager_create_option_parser();

        ce_optparse_set_standard_properties(optparse, CE_SPIKE_VERSION_MAJOR, CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
            "Cursed Earth: Video Player", "This program is part of Cursed Earth spikes.\nVideo Player - play Evil Islands videos.");

        ce_optparse_add(optparse, "track", CE_TYPE_STRING, NULL, true, NULL, NULL, "any TRACK.* file in `EI/Movies'");

        ce_root root(optparse, argc, argv);

        ce_root::instance()->scenemng->listener.state_changed = state_changed;
        ce_root::instance()->scenemng->listener.advance = advance;
        ce_root::instance()->scenemng->listener.render = render;

        input_supply = std::make_shared<input_supply_t>(ce_root::instance()->renderwindow->input_context());
        pause_event = input_supply->single_front(input_supply->push(input_button_t::kb_space));

        return ce_root::instance()->exec();
    } catch (const std::exception& error) {
        ce_logging_fatal("video player: %s", error.what());
    }
    return EXIT_FAILURE;
}
