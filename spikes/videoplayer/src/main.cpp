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
#include <assert.h>

#include "cealloc.h"
#include "celogging.h"
#include "ceoptionmanager.h"
#include "cevideoobject.h"
#include "ceroot.h"

static bool pause;
static ce_video_object video_object;
static ce_optparse* optparse;
static ce_input_supply* input_supply;
static ce_input_event* pause_event;

static void clear()
{
    ce_input_supply_del(input_supply);
    ce_optparse_del(optparse);
}

static void state_changed(void* CE_UNUSED(listener), int state)
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

static void advance(void* CE_UNUSED(listener), float elapsed)
{
    ce_input_supply_advance(input_supply, elapsed);
    ce_video_object_advance(video_object, elapsed);

    if (pause_event->triggered) {
        pause = !pause;
        if (pause) {
            ce_video_object_pause(video_object);
        } else {
            ce_video_object_play(video_object);
        }
    }
}

static void render(void* CE_UNUSED(listener))
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

    ce_root.scenemng->listener = (ce_scenemng_listener){.state_changed = state_changed, .advance = advance, .render = render};

    input_supply = ce_input_supply_new(ce_root.renderwindow->input_context);
    pause_event = ce_input_supply_single_front(input_supply, ce_input_supply_button(input_supply, CE_KB_SPACE));

    return ce_root_exec();
}