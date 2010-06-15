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
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceopengl.h"
#include "cetexture.h"
#include "ceroot.h"
#include "cesoundinstance.h"
#include "cevideoinstance.h"

static ce_optparse* optparse;
static ce_inputsupply* inputsupply;
static ce_inputevent* pause_event;
static ce_video_id video_id;
static bool pause;

static void clean()
{
	ce_inputsupply_del(inputsupply);
	ce_optparse_del(optparse);
}

static void advance(void* listener, float elapsed)
{
	ce_unused(listener);
	ce_inputsupply_advance(inputsupply, elapsed);

	if (pause_event->triggered) {
		pause = !pause;
		if (pause) {
			ce_video_manager_pause(ce_root.video_manager, video_id);
		} else {
			ce_video_manager_play(ce_root.video_manager, video_id);
		}
	}
}

int main(int argc, char* argv[])
{
	ce_alloc_init();
	atexit(clean);

	optparse = ce_root_create_optparse();

	ce_optparse_add(optparse, "track", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any TRACK.* file in 'EI/Movies'");

	if (!ce_optparse_parse(optparse, argc, argv) || !ce_root_init(optparse)) {
		return EXIT_FAILURE;
	}

	const char* track;
	ce_optparse_get(optparse, "track", &track);

	video_id = ce_video_manager_create(ce_root.video_manager, track);
	if (0 == video_id) {
		ce_logging_error("video player: could not play video track '%s'", track);
	} else {
		ce_video_manager_play(ce_root.video_manager, video_id);
	}

	ce_scenemng_listener scenemng_listener = {.advance = advance, .render = NULL};
	ce_scenemng_add_listener(ce_root.scenemng, &scenemng_listener);

	inputsupply = ce_inputsupply_new(ce_root.renderwindow->inputcontext);
	pause_event = ce_inputsupply_single_front(inputsupply,
					ce_inputsupply_button(inputsupply, CE_KB_SPACE));

	return ce_root_exec();
}
