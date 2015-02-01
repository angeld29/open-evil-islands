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

static bool paused;
static ce_video_object video_object;
static ce_optparse* optparse;
static input_supply_ptr_t input_supply;
static input_event_const_ptr_t pause_event;

static void clear()
{
    ce_optparse_del(optparse);
}

static void state_changed(void* /*listener*/, int state)
{
    if (CE_SCENEMNG_STATE_READY == state) {
        const char* track;
        ce_optparse_get(optparse, "track", &track);

        video_object = ce_video_object_new(track);
        if (0 == video_object) {
            ce_logging_error("video player: could not play video track `%s'", track);
        } else {
            ce_video_object_play(video_object);
        }

        ce_scenemng_change_state(ce_root.scenemng, CE_SCENEMNG_STATE_LOADING);
    }
}

static void advance(void* /*listener*/, float elapsed)
{
    input_supply->advance(elapsed);
    ce_video_object_advance(video_object, elapsed);

    if (pause_event->is_triggered()) {
        paused = !paused;
        if (paused) {
            ce_video_object_pause(video_object);
        } else {
            ce_video_object_play(video_object);
        }
    }
}

static void render(void*)
{
    ce_video_object_render(video_object);
}

int main(int argc, char* argv[])
{
    ce_alloc_init();
    atexit(clear);

    optparse = ce_option_manager_create_option_parser();

    ce_optparse_set_standard_properties(optparse, CE_SPIKE_VERSION_MAJOR, CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
        "Cursed Earth: Video Player", "This program is part of Cursed Earth spikes.\nVideo Player - play Evil Islands videos.");

    ce_optparse_add(optparse, "track", CE_TYPE_STRING, NULL, true, NULL, NULL, "any TRACK.* file in `EI/Movies'");

    if (!ce_root_init(optparse, argc, argv)) {
        return EXIT_FAILURE;
    }

    ce_root.scenemng->listener.state_changed = state_changed;
    ce_root.scenemng->listener.advance = advance;
    ce_root.scenemng->listener.render = render;

    input_supply = std::unique_ptr<input_supply_t>(new input_supply_t(ce_root.renderwindow->input_context));
    pause_event = input_supply->single_front(input_supply->push(CE_KB_SPACE));

    return ce_root_exec();
}
