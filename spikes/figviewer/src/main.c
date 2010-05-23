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
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "celib.h"
#include "cestr.h"
#include "cemath.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"

static ce_optparse* optparse;
static ce_figentity* figentity;

static ce_input_supply* input_supply;
static ce_input_event* strength_event;
static ce_input_event* dexterity_event;
static ce_input_event* height_event;
static ce_input_event* anm_change_event;
static ce_input_event* anmfps_inc_event;
static ce_input_event* anmfps_dec_event;

static int anmidx = -1;
static ce_complection complection = { 1.0f, 1.0f, 1.0f };

static void clean()
{
	ce_input_supply_del(input_supply);
	ce_optparse_del(optparse);
}

static bool update_figentity()
{
	ce_scenemng_remove_figentity(ce_root.scenemng, figentity);

	ce_vec3 position = CE_VEC3_ZERO;

	ce_quat orientation, q1, q2;
	ce_quat_init_polar(&q1, ce_deg2rad(180.0f), &CE_VEC3_UNIT_Z);
	ce_quat_init_polar(&q2, ce_deg2rad(270.0f), &CE_VEC3_UNIT_X);
	ce_quat_mul(&orientation, &q2, &q1);

	const char *textures[2], *figure;
	ce_optparse_get(optparse, "pritex", &textures[0]);
	ce_optparse_get(optparse, "sectex", &textures[1]);
	ce_optparse_get(optparse, "figure", &figure);

	figentity = ce_scenemng_create_figentity(ce_root.scenemng, figure,
		&complection, &position, &orientation, 2, textures, NULL);

	if (NULL == figentity) {
		return false;
	}

	if (-1 != anmidx) {
		ce_figentity_play_animation(figentity,
			ce_figentity_get_animation_name(figentity, anmidx));
	}

	return true;
}

static void advance(void* listener, float elapsed)
{
	ce_unused(listener);
	ce_input_supply_advance(input_supply, elapsed);

	if (anmfps_inc_event->triggered) ce_root.anmfps += 1.0f;
	if (anmfps_dec_event->triggered) ce_root.anmfps -= 1.0f;

	ce_root.anmfps = ce_fclamp(ce_root.anmfps, 1.0f, 50.0f);

	bool need_update_figentity = false;

	if (strength_event->triggered) {
		if ((complection.strength += 0.1f) > 1.0f) {
			complection.strength = 0.0f;
		}
		need_update_figentity = true;
	}

	if (dexterity_event->triggered) {
		if ((complection.dexterity += 0.1f) > 1.0f) {
			complection.dexterity = 0.0f;
		}
		need_update_figentity = true;
	}

	if (height_event->triggered) {
		if ((complection.height += 0.1f) > 1.0f) {
			complection.height = 0.0f;
		}
		need_update_figentity = true;
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
			ce_figentity_play_animation(figentity,
				ce_figentity_get_animation_name(figentity, anmidx));
			ce_logging_write("main: new animation name: '%s'",
				ce_figentity_get_animation_name(figentity, anmidx));
		} else {
			ce_logging_write("main: new animation name: none");
		}
	}
}

static ce_scenemng_listener scenemng_listener = { .advance = advance };

int main(int argc, char* argv[])
{
	ce_alloc_init();

	optparse = ce_root_create_optparse();

	ce_optparse_set_standard_properties(optparse, CE_SPIKE_VERSION_MAJOR,
		CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
		"Cursed Earth: Figure Viewer", "This program is part of Cursed "
		"Earth spikes\nFigure Viewer - control Evil Islands figures");

	ce_optparse_add(optparse, "pritex", CE_TYPE_STRING, "default0", false,
		NULL, "primary-texture", "primary texture");
	ce_optparse_add(optparse, "sectex", CE_TYPE_STRING, "default0", false,
		NULL, "secondary-texture", "secondary texture");
	ce_optparse_add(optparse, "anmname", CE_TYPE_STRING, NULL, false,
		NULL, "animation-name", "play animation with specified name");
	ce_optparse_add(optparse, "figure", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "internal figure name");

	ce_optparse_add_control(optparse, "+/-", "change animation FPS");
	ce_optparse_add_control(optparse, "a", "play next animation");
	ce_optparse_add_control(optparse, "1", "change strength");
	ce_optparse_add_control(optparse, "2", "change dexterity");
	ce_optparse_add_control(optparse, "3", "change height");

	atexit(clean);

	if (!ce_optparse_parse(optparse, argc, argv) ||
			!ce_root_init(optparse) || !update_figentity()) {
		return EXIT_FAILURE;
	}

	const char* anmname;
	ce_optparse_get(optparse, "anmname", &anmname);

	if (NULL != anmname) {
		if (ce_figentity_play_animation(figentity, anmname)) {
			int anmcount = ce_figentity_get_animation_count(figentity);
			for (anmidx = 0; anmidx < anmcount; ++anmidx) {
				if (0 == ce_strcasecmp(anmname,
						ce_figentity_get_animation_name(figentity, anmidx))) {
					break;
				}
			}
		} else {
			ce_logging_warning("main: could not play animation '%s'", anmname);
		}
	}

	ce_vec3 position;
	ce_camera_set_position(ce_root.scenemng->camera,
		ce_vec3_init(&position, 0.0f, 2.0f, -4.0f));

	ce_camera_set_near(ce_root.scenemng->camera, 0.1f);
	ce_camera_yaw_pitch(ce_root.scenemng->camera, ce_deg2rad(180.0f),
													ce_deg2rad(30.0f));

	ce_scenemng_add_listener(ce_root.scenemng, &scenemng_listener);

	input_supply = ce_input_supply_new(ce_root.renderwindow->input_context);
	strength_event = ce_input_supply_single_front(input_supply,
						ce_input_supply_button(input_supply, CE_KB_1));
	dexterity_event = ce_input_supply_single_front(input_supply,
						ce_input_supply_button(input_supply, CE_KB_2));
	height_event = ce_input_supply_single_front(input_supply,
						ce_input_supply_button(input_supply, CE_KB_3));
	anm_change_event = ce_input_supply_single_front(input_supply,
					ce_input_supply_button(input_supply, CE_KB_A));
	anmfps_inc_event = ce_input_supply_repeat(input_supply,
						ce_input_supply_button(input_supply,
							CE_KB_ADD), CE_INPUT_NO_DELAY, 10);
	anmfps_dec_event = ce_input_supply_repeat(input_supply,
						ce_input_supply_button(input_supply,
							CE_KB_SUBTRACT), CE_INPUT_NO_DELAY, 10);

	return ce_root_exec();
}
