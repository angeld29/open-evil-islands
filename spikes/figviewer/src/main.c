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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <GL/glut.h>

#ifdef _WIN32
// fu... win32
#undef near
#undef far
#endif

#include "cestr.h"
#include "cemath.h"
#include "cegl.h"
#include "celogging.h"
#include "cealloc.h"
#include "cescenemng.h"
#include "ceoptparse.h"

#ifndef CE_SPIKE_VERSION_MAJOR
#define CE_SPIKE_VERSION_MAJOR 0
#endif
#ifndef CE_SPIKE_VERSION_MINOR
#define CE_SPIKE_VERSION_MINOR 0
#endif
#ifndef CE_SPIKE_VERSION_PATCH
#define CE_SPIKE_VERSION_PATCH 0
#endif

static ce_optparse* optparse;
static ce_scenemng* scenemng;
static ce_figentity* figentity;

static ce_input_event_supply* es;
static ce_input_event* strength_event;
static ce_input_event* dexterity_event;
static ce_input_event* height_event;
static ce_input_event* toggle_bbox_event;
static ce_input_event* anm_change_event;
static ce_input_event* anm_fps_inc_event;
static ce_input_event* anm_fps_dec_event;

static int anm_index = -1;
static float anm_fps_limit = 0.1f;
static float anm_fps_inc_counter;
static float anm_fps_dec_counter;

static ce_complection complection = { 1.0f, 1.0f, 1.0f };

static bool update_figentity()
{
	ce_scenemng_remove_figentity(scenemng, figentity);

	ce_vec3 position = CE_VEC3_ZERO;

	ce_quat orientation, q1, q2;
	ce_quat_init_polar(&q1, ce_deg2rad(180.0f), &CE_VEC3_UNIT_Z);
	ce_quat_init_polar(&q2, ce_deg2rad(270.0f), &CE_VEC3_UNIT_X);
	ce_quat_mul(&orientation, &q2, &q1);

	const char* texture_names[] = {
		ce_optparse_find_option(optparse, "general", "pri_tex")->value->str,
		ce_optparse_find_option(optparse, "general", "sec_tex")->value->str,
	};

	figentity = ce_scenemng_create_figentity(scenemng,
		ce_optparse_find_arg(optparse, "figure_name")->value->str,
		&complection, &position, &orientation,
		2, texture_names, NULL);
	if (NULL == figentity) {
		return false;
	}

	if (-1 != anm_index) {
		ce_figentity_play_animation(figentity,
			ce_figentity_get_animation_name(figentity, anm_index));
	}

	return true;
}

static void idle(void)
{
	ce_scenemng_advance(scenemng);

	float elapsed = ce_timer_elapsed(scenemng->timer);

	ce_input_event_supply_advance(es, elapsed);

	if (ce_input_test(CE_KB_ESCAPE)) {
		ce_input_event_supply_del(es);
		ce_scenemng_del(scenemng);
		ce_optparse_del(optparse);
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

	anm_fps_inc_counter += elapsed;
	anm_fps_dec_counter += elapsed;

	if (ce_input_event_triggered(anm_fps_inc_event) &&
			anm_fps_inc_counter >= anm_fps_limit) {
		scenemng->anm_fps += 1.0f;
		anm_fps_inc_counter = 0.0f;
	}

	if (ce_input_event_triggered(anm_fps_dec_event) &&
			anm_fps_dec_counter >= anm_fps_limit) {
		scenemng->anm_fps -= 1.0f;
		anm_fps_dec_counter = 0.0f;
	}

	scenemng->anm_fps = ce_fclamp(scenemng->anm_fps, 1.0f, 50.0f);

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
}

static void display(void)
{
	ce_scenemng_render(scenemng);

	glutSwapBuffers();
}

static void reshape(int width, int height)
{
	ce_viewport_set_rect(scenemng->viewport, 0, 0, width, height);
	ce_camera_set_aspect(scenemng->camera, (float)width / height);
}

int main(int argc, char* argv[])
{
	ce_logging_init();
#ifdef NDEBUG
	ce_logging_set_level(CE_LOGGING_LEVEL_WARNING);
#else
	ce_logging_set_level(CE_LOGGING_LEVEL_DEBUG);
#endif
	ce_alloc_init();

	optparse = ce_optparse_new(CE_SPIKE_VERSION_MAJOR,
		CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
		"This program is part of Cursed Earth spikes\n"
		"Figure Viewer %d.%d.%d - Control Evil Islands figures\n\n"
		"controls:\n"
		"1     change strength\n"
		"2     change dexterity\n"
		"3     change height\n"
		"b     show/hide bounding boxes\n"
		"a     play next animation\n"
		"+/-   change animation FPS",
		CE_SPIKE_VERSION_MAJOR, CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH);
	ce_optgroup* general_grp = ce_optparse_create_group(optparse, "general");
	ce_optoption* ei_path_opt = ce_optgroup_create_option(general_grp,
		"ei_path", 'b', "ei-path", CE_OPTACTION_STORE,
		"path to EI root dir (current dir by default)", ".");
	ce_optoption* full_screen_opt = ce_optgroup_create_option(general_grp,
		"full_screen", 'f', "full-screen", CE_OPTACTION_STORE_TRUE,
		"start program in Full Screen mode", NULL);
	ce_optgroup_create_option(general_grp,
		"pri_tex", 'p', "primary-texture", CE_OPTACTION_STORE,
		"primary texture", "default0");
	ce_optgroup_create_option(general_grp,
		"sec_tex", 's', "secondary-texture", CE_OPTACTION_STORE,
		"secondary texture", "default0");
	ce_optoption* anm_name_opt = ce_optgroup_create_option(general_grp,
		"anm_name", 'a', "anm-name", CE_OPTACTION_STORE,
		"play animation with specified name", NULL);
	ce_optparse_create_arg(optparse, "figure_name", "internal figure name");

	if (!ce_optparse_parse_args(optparse, argc, argv)) {
		return EXIT_FAILURE;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);

	if (ce_optoption_value_bool(full_screen_opt)) {
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "%dx%d:32",
			glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
		glutGameModeString(buffer);
		if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
			glutEnterGameMode();
		} else {
			ce_logging_warning("main: full screen mode is not available");
		}
	}

	if (!glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
		glutInitWindowPosition(100, 100);
		glutInitWindowSize(1024, 768);
		glutCreateWindow("Cursed Earth: Figure Viewer");
	}

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	ce_input_init();
	ce_gl_init();

	if (NULL == (scenemng = ce_scenemng_new(ei_path_opt->value->str))) {
		return EXIT_FAILURE;
	}

	if (!update_figentity()) {
		return EXIT_FAILURE;
	}

	if (!ce_optoption_value_empty(anm_name_opt)) {
		if (ce_figentity_play_animation(figentity, anm_name_opt->value->str)) {
			int anm_count = ce_figentity_get_animation_count(figentity);
			for (anm_index = 0; anm_index < anm_count &&
					0 == ce_strcasecmp(anm_name_opt->value->str,
					ce_figentity_get_animation_name(figentity, anm_index));
					++anm_index) {
			}
		} else {
			ce_logging_warning("main: could not play animation: '%s'",
											anm_name_opt->value->str);
		}
	}

	ce_vec3 position;
	ce_vec3_init(&position, 0.0f, 2.0f, -4.0f);

	ce_camera_set_near(scenemng->camera, 0.1f);
	ce_camera_set_position(scenemng->camera, &position);
	ce_camera_yaw_pitch(scenemng->camera, ce_deg2rad(180.0f), ce_deg2rad(30.0f));

	es = ce_input_event_supply_new();
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
	anm_fps_inc_event = ce_input_event_supply_button_event(es, CE_KB_ADD);
	anm_fps_dec_event = ce_input_event_supply_button_event(es, CE_KB_SUBTRACT);

	glutMainLoop();
	return EXIT_SUCCESS;
}
