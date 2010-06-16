/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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
#include "cesoundobject.h"
#include "cesoundmanager.h"
#include "ceroot.h"

static ce_sound_object sound_object;
static ce_optparse* optparse;
static ce_inputsupply* inputsupply;
static ce_inputevent* rewind_event;

static void clean()
{
	ce_inputsupply_del(inputsupply);
	ce_optparse_del(optparse);
}

static void state_changed(void* listener, int state)
{
	ce_unused(listener);

	if (CE_SCENEMNG_STATE_READY == state) {
		const char* track;
		ce_optparse_get(optparse, "track", &track);

		sound_object = ce_sound_manager_create(track);
		if (0 == sound_object) {
			ce_logging_error("sound player: could not play audio track '%s'", track);
		} else {
			ce_sound_object_play(sound_object);
		}

		ce_scenemng_change_state(ce_root.scenemng, CE_SCENEMNG_STATE_LOADING);
	}
}

static void advance(void* listener, float elapsed)
{
	ce_unused(listener);
	ce_inputsupply_advance(inputsupply, elapsed);

	if (rewind_event->triggered) {
	}
}

int main(int argc, char* argv[])
{
	ce_alloc_init();
	atexit(clean);

	optparse = ce_root_create_optparse();

	ce_optparse_add(optparse, "track", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any TRACK.* file in 'EI/Stream'");

	if (!ce_root_init(optparse, argc, argv)) {
		return EXIT_FAILURE;
	}

	ce_root.scenemng->listener = (ce_scenemng_listener)
		{.state_changed = state_changed, .advance = advance, .render = NULL};

	inputsupply = ce_inputsupply_new(ce_root.renderwindow->inputcontext);
	rewind_event = ce_inputsupply_single_front(inputsupply,
					ce_inputsupply_button(inputsupply, CE_KB_R));

	return ce_root_exec();
}
