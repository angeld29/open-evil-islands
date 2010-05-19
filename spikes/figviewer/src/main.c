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

#include "cestr.h"
#include "cemath.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"

static ce_optparse* optparse;
static ce_figentity* figentity;

/*static ce_input_event_supply* es;
static ce_input_event* strength_event;
static ce_input_event* dexterity_event;
static ce_input_event* height_event;
static ce_input_event* toggle_bbox_event;
static ce_input_event* anm_change_event;
static ce_input_event* anmfps_inc_event;
static ce_input_event* anmfps_dec_event;*/

static int anm_index = -1;
//static float anmfps_limit = 0.1f;
//static float anmfps_inc_counter;
//static float anmfps_dec_counter;

static char pri_tex_name[32] = "default0";
static char sec_tex_name[32] = "default0";
static char figure_name[32];

static ce_complection complection = { 1.0f, 1.0f, 1.0f };

static bool update_figentity()
{
	ce_scenemng_remove_figentity(ce_root.scenemng, figentity);

	ce_vec3 position = CE_VEC3_ZERO;

	ce_quat orientation, q1, q2;
	ce_quat_init_polar(&q1, ce_deg2rad(180.0f), &CE_VEC3_UNIT_Z);
	ce_quat_init_polar(&q2, ce_deg2rad(270.0f), &CE_VEC3_UNIT_X);
	ce_quat_mul(&orientation, &q2, &q1);

	figentity = ce_scenemng_create_figentity(ce_root.scenemng,
		figure_name, &complection, &position, &orientation,
		2, (const char*[]){ pri_tex_name, sec_tex_name }, NULL);

	if (NULL == figentity) {
		return false;
	}

	if (-1 != anm_index) {
		ce_figentity_play_animation(figentity,
			ce_figentity_get_animation_name(figentity, anm_index));
	}

	return true;
}

/*static void idle(void)
{
	ce_input_event_supply_advance(es, elapsed);

	anmfps_inc_counter += elapsed;
	anmfps_dec_counter += elapsed;

	if (ce_input_event_triggered(anmfps_inc_event) &&
			anmfps_inc_counter >= anmfps_limit) {
		scenemng->anmfps += 1.0f;
		anmfps_inc_counter = 0.0f;
	}

	if (ce_input_event_triggered(anmfps_dec_event) &&
			anmfps_dec_counter >= anmfps_limit) {
		scenemng->anmfps -= 1.0f;
		anmfps_dec_counter = 0.0f;
	}

	scenemng->anmfps = ce_fclamp(scenemng->anmfps, 1.0f, 50.0f);

	bool need_update_figentity = false;

	if (ce_input_event_triggered(strength_event)) {
		if ((complection.strength += 0.1f) > 1.0f) {
			complection.strength = 0.0f;
		}
		need_update_figentity = true;
	}

	if (ce_input_event_triggered(dexterity_event)) {
		if ((complection.dexterity += 0.1f) > 1.0f) {
			complection.dexterity = 0.0f;
		}
		need_update_figentity = true;
	}

	if (ce_input_event_triggered(height_event)) {
		if ((complection.height += 0.1f) > 1.0f) {
			complection.height = 0.0f;
		}
		need_update_figentity = true;
	}

	if (need_update_figentity) {
		update_figentity();
	}

	if (ce_input_event_triggered(anm_change_event)) {
		ce_figentity_stop_animation(figentity);
		int anm_count = ce_figentity_get_animation_count(figentity);
		if (++anm_index == anm_count) {
			anm_index = -1;
		}
		if (-1 != anm_index) {
			ce_figentity_play_animation(figentity,
				ce_figentity_get_animation_name(figentity, anm_index));
			ce_logging_write("main: new animation name: '%s'",
				ce_figentity_get_animation_name(figentity, anm_index));
		} else {
			ce_logging_write("main: new animation name: none");
		}
	}
}*/

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
		NULL, "anim-name", "play animation with specified name");
	ce_optparse_add(optparse, "figure", CE_TYPE_STRING, NULL, true,
		NULL, NULL, "internal figure name");

	ce_optparse_add_control(optparse, "+/-", "change animation FPS");
	ce_optparse_add_control(optparse, "a", "play next animation");
	ce_optparse_add_control(optparse, "1", "change strength");
	ce_optparse_add_control(optparse, "2", "change dexterity");
	ce_optparse_add_control(optparse, "3", "change height");

	if (!ce_optparse_parse(optparse, argc, argv) || !ce_root_init(optparse)) {
		ce_optparse_del(optparse);
		return EXIT_FAILURE;
	}

	if (!update_figentity()) {
		return EXIT_FAILURE;
	}

	/*if (0 != anm_name->count) {
		if (ce_figentity_play_animation(figentity, anm_name->sval[0])) {
			int anm_count = ce_figentity_get_animation_count(figentity);
			for (anm_index = 0; anm_index < anm_count &&
					0 == ce_strcasecmp(anm_name->sval[0],
					ce_figentity_get_animation_name(figentity, anm_index));
					++anm_index) {
			}
		} else {
			ce_logging_warning("main: "
				"could not play animation: '%s'", anm_name->sval[0]);
		}
	}*/

	ce_vec3 position;
	ce_camera_set_position(ce_root.scenemng->camera,
		ce_vec3_init(&position, 0.0f, 2.0f, -4.0f));

	ce_camera_set_near(ce_root.scenemng->camera, 0.1f);
	ce_camera_yaw_pitch(ce_root.scenemng->camera, ce_deg2rad(180.0f),
													ce_deg2rad(30.0f));

	/*es = ce_input_event_supply_new();
	strength_event = ce_input_event_supply_single_front_event(es,
						ce_input_event_supply_button_event(es, CE_KB_1));
	dexterity_event = ce_input_event_supply_single_front_event(es,
						ce_input_event_supply_button_event(es, CE_KB_2));
	height_event = ce_input_event_supply_single_front_event(es,
						ce_input_event_supply_button_event(es, CE_KB_3));
	anm_change_event = ce_input_event_supply_single_front_event(es,
					ce_input_event_supply_button_event(es, CE_KB_A));
	anmfps_inc_event = ce_input_event_supply_button_event(es, CE_KB_ADD);
	anmfps_dec_event = ce_input_event_supply_button_event(es, CE_KB_SUBTRACT);*/

	return ce_root_exec();
}
