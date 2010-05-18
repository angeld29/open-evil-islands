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
#include <assert.h>

#include <argtable2.h>

#include "cestr.h"
#include "cemath.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"

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

	if (ce_input_test(CE_KB_ESCAPE)) {
		ce_input_event_supply_del(es);
		ce_scenemng_del(scenemng);
		ce_gl_term();
		ce_input_term();
		ce_alloc_term();
		ce_logging_term();
		if (glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
			glutLeaveGameMode();
		}
		exit(EXIT_SUCCESS);
	}

	if (ce_input_event_triggered(toggle_bbox_event)) {
		if (scenemng->show_bboxes) {
			if (scenemng->comprehensive_bbox_only) {
				scenemng->comprehensive_bbox_only = false;
			} else {
				scenemng->show_bboxes = false;
			}
		} else {
			scenemng->show_bboxes = true;
			scenemng->comprehensive_bbox_only = true;
		}
	}

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

	if (ce_input_test(CE_KB_LEFT)) {
		ce_camera_move(scenemng->camera, -0.5f * elapsed, 0.0f);
	}

	if (ce_input_test(CE_KB_UP)) {
		ce_camera_move(scenemng->camera, 0.0f, 0.5f * elapsed);
	}

	if (ce_input_test(CE_KB_RIGHT)) {
		ce_camera_move(scenemng->camera, 0.5f * elapsed, 0.0f);
	}

	if (ce_input_test(CE_KB_DOWN)) {
		ce_camera_move(scenemng->camera, 0.0f, -0.5f * elapsed);
	}

	if (ce_input_test(CE_MB_WHEELUP)) {
		ce_camera_zoom(scenemng->camera, 0.1f);
	}

	if (ce_input_test(CE_MB_WHEELDOWN)) {
		ce_camera_zoom(scenemng->camera, -0.1f);
	}

	if (ce_input_test(CE_MB_RIGHT)) {
		ce_vec2 offset = ce_input_mouse_offset();
		ce_camera_yaw_pitch(scenemng->camera, ce_deg2rad(-0.13f * offset.x),
												ce_deg2rad(-0.13f * offset.y));
	}

	glutPostRedisplay();
}*/

static void usage(const char* progname, void* argtable[])
{
	fprintf(stderr, "Cursed Earth is an open source, "
		"cross-platform port of Evil Islands\n"
		"Copyright (C) 2009-2010 Yanis Kurganov\n\n");

	fprintf(stderr, "This program is part of Cursed Earth spikes\n"
		"Figure Viewer - control Evil Islands figures\n\n");

	fprintf(stderr, "usage: %s", progname);
	arg_print_syntax(stderr, argtable, "\n");
	arg_print_glossary_gnu(stderr, argtable);

	void* ctrtable[] = {
		arg_rem("keyboard arrows", "move camera"),
		arg_rem("mouse motion", "rotate camera"),
		arg_rem("mouse wheel", "zoom camera"),
		arg_rem("b", "show/hide bounding boxes"),
		arg_rem("+/-", "change animation FPS"),
		arg_rem("a", "play next animation"),
		arg_rem("1", "change strength"),
		arg_rem("2", "change dexterity"),
		arg_rem("3", "change height"),
		arg_end(0)
	};

	fprintf(stderr, "controls:\n");
	arg_print_glossary_gnu(stderr, ctrtable);
	arg_freetable(ctrtable, sizeof(ctrtable) / sizeof(ctrtable[0]));
}

int main(int argc, char* argv[])
{
	struct arg_lit* help = arg_lit0("h", "help", "display this help and exit");
	struct arg_lit* version = arg_lit0("v", "version",
		"display version information and exit");
	struct arg_str* ei_path = arg_str0("b", "ei-path", "DIRECTORY",
		"path to EI directory (current directory by default)");
	struct arg_lit* full_screen = arg_lit0("f", "full-screen",
		"start program in full screen mode");
	struct arg_int* jobs = arg_int0("j", "jobs", "N",
		"allow N jobs at once; if this option is not specified, the "
		"value N will be detected automatically depending on the number "
		"of CPUs you have (or the number of cores your CPU have)");
	struct arg_str* pri_tex = arg_str0("p", "primary-texture", "NAME",
		"primary texture");
	struct arg_str* sec_tex = arg_str0("s", "secondary-texture", "NAME",
		"secondary texture");
	struct arg_str* anm_name = arg_str0("a", "anm-name", "NAME",
		"play animation with specified name");
	struct arg_str* figure = arg_str1(NULL, NULL,
		"FIGURE", "internal figure name");
	struct arg_end* end = arg_end(3);

	void* argtable[] = {
		help, version, ei_path, full_screen, jobs,
		pri_tex, sec_tex, anm_name, figure, end
	};

	ei_path->sval[0] = ".";

	int argerror_count = arg_parse(argc, argv, argtable);

	if (0 != help->count) {
		usage(argv[0], argtable);
		return EXIT_SUCCESS;
	}

	if (0 != version->count) {
		fprintf(stderr, "%d.%d.%d\n", CE_SPIKE_VERSION_MAJOR,
										CE_SPIKE_VERSION_MINOR,
										CE_SPIKE_VERSION_PATCH);
		return EXIT_SUCCESS;
	}

	if (0 != argerror_count) {
		usage(argv[0], argtable);
		arg_print_errors(stderr, end, argv[0]);
		return EXIT_FAILURE;
	}

	if (!ce_root_init(ei_path->sval[0])) {
		return EXIT_FAILURE;
	}

	// TODO: move to root
	if (0 != jobs->count) {
		ce_root.thread_count = jobs->ival[0];
	}

	ce_logging_write("root: using up to %d threads", ce_root.thread_count);

	if (0 != pri_tex->count) {
		ce_strlcpy(pri_tex_name, pri_tex->sval[0], sizeof(pri_tex_name));
	}

	if (0 != sec_tex->count) {
		ce_strlcpy(sec_tex_name, sec_tex->sval[0], sizeof(sec_tex_name));
	}

	ce_strlcpy(figure_name, figure->sval[0], sizeof(figure_name));

	if (!update_figentity()) {
		return EXIT_FAILURE;
	}

	if (0 != anm_name->count) {
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
	}

	ce_vec3 position;
	ce_vec3_init(&position, 0.0f, 2.0f, -4.0f);

	ce_camera_set_near(ce_root.scenemng->camera, 0.1f);
	ce_camera_set_position(ce_root.scenemng->camera, &position);
	ce_camera_yaw_pitch(ce_root.scenemng->camera, ce_deg2rad(180.0f), ce_deg2rad(30.0f));

	/*es = ce_input_event_supply_new();
	strength_event = ce_input_event_supply_single_front_event(es,
						ce_input_event_supply_button_event(es, CE_KB_1));
	dexterity_event = ce_input_event_supply_single_front_event(es,
						ce_input_event_supply_button_event(es, CE_KB_2));
	height_event = ce_input_event_supply_single_front_event(es,
						ce_input_event_supply_button_event(es, CE_KB_3));
	toggle_bbox_event = ce_input_event_supply_single_front_event(es,
					ce_input_event_supply_button_event(es, CE_KB_B));
	anm_change_event = ce_input_event_supply_single_front_event(es,
					ce_input_event_supply_button_event(es, CE_KB_A));
	anmfps_inc_event = ce_input_event_supply_button_event(es, CE_KB_ADD);
	anmfps_dec_event = ce_input_event_supply_button_event(es, CE_KB_SUBTRACT);*/

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

	ce_root_exec();

	return EXIT_SUCCESS;
}
