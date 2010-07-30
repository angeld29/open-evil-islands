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

#include "cemath.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceoptionmanager.h"
#include "cesoundobject.h"
#include "ceroot.h"

static ce_color message_color;
static float alpha_sign = -1.0f;
static ce_sound_object sound_object;
static ce_optparse* optparse;
static ce_input_supply* input_supply;
static ce_input_event* stub_event;

static void clean()
{
	ce_input_supply_del(input_supply);
	ce_optparse_del(optparse);
	ce_sound_object_del(sound_object);
}

static void state_changed(void* CE_UNUSED(listener), int state)
{
	if (CE_SCENEMNG_STATE_READY == state) {
		const char* track;
		ce_optparse_get(optparse, "track", &track);

		sound_object = ce_sound_object_new(track);
		ce_sound_object_play(sound_object);

		ce_scenemng_change_state(ce_root.scenemng, CE_SCENEMNG_STATE_LOADING);
	}
}

static void advance(void* CE_UNUSED(listener), float elapsed)
{
	ce_input_supply_advance(input_supply, elapsed);

	if (ce_sound_object_is_valid(sound_object) &&
			!ce_sound_object_is_stopped(sound_object)) {
		message_color.a += elapsed * alpha_sign;
		alpha_sign = message_color.a < 0.25f ? 1.0f :
						(message_color.a > 1.0f ? -1.0f : alpha_sign);
	} else {
		message_color.a = 1.0f;
	}
}

static void render(void* CE_UNUSED(listener))
{
	if (CE_SCENEMNG_STATE_PLAYING != ce_root.scenemng->state) {
		return;
	}

	char message[32];

	const char* track;
	ce_optparse_get(optparse, "track", &track);

	if (ce_sound_object_is_valid(sound_object)) {
		if (ce_sound_object_is_stopped(sound_object)) {
			snprintf(message, sizeof(message), "Track '%s' stopped", track);
		} else {
			snprintf(message, sizeof(message), "Playing track '%s'", track);
		}
	} else {
		snprintf(message, sizeof(message), "Unable to play track '%s'", track);
	}

	ce_font_render(ce_root.scenemng->font, (ce_root.scenemng->viewport->width -
		ce_font_get_width(ce_root.scenemng->font, message)) / 2,
		1 * (ce_root.scenemng->viewport->height -
		ce_font_get_height(ce_root.scenemng->font)) / 5,
		&message_color, message);
}

int main(int argc, char* argv[])
{
	ce_alloc_init();

	optparse = ce_option_manager_create_option_parser();

	ce_optparse_set_standard_properties(optparse, CE_SPIKE_VERSION_MAJOR,
		CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
		"Cursed Earth: Sound Player", "This program is part of Cursed "
		"Earth spikes\nSound Player - play Evil Islands sounds");

	ce_optparse_add(optparse, "track", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "any TRACK.* file in 'EI/Stream'");

	if (!ce_root_init(optparse, argc, argv)) {
		clean();
		return EXIT_FAILURE;
	}

	message_color = CE_COLOR_CORNFLOWER;

	ce_root.scenemng->listener = (ce_scenemng_listener)
		{.state_changed = state_changed, .advance = advance, .render = render};

	input_supply = ce_input_supply_new(ce_root.renderwindow->input_context);
	stub_event = ce_input_supply_single_front(input_supply,
					ce_input_supply_button(input_supply, CE_KB_R));

	atexit(clean);
	return ce_root_exec();
}
