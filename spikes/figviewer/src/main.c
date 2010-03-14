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

#include <getopt.h>
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
#include "ceroot.h"
#include "cescenemng.h"
#include "cefigmesh.h"
#include "cefigentity.h"

#ifndef CE_SPIKE_VERSION_MAJOR
#define CE_SPIKE_VERSION_MAJOR 0
#endif
#ifndef CE_SPIKE_VERSION_MINOR
#define CE_SPIKE_VERSION_MINOR 0
#endif
#ifndef CE_SPIKE_VERSION_PATCH
#define CE_SPIKE_VERSION_PATCH 0
#endif

static ce_scenemng* scenemng;

static ce_figproto* figproto;
static ce_figmesh* figmesh;
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

static const char* figure_name = NULL;
static const char* texture_names[] = { "default0", "default0" };

static bool update_figentity()
{
	ce_figentity_del(figentity);

	ce_vec3 position = CE_VEC3_ZERO;

	ce_quat orientation, q1, q2;
	ce_quat_init_polar(&q1, ce_deg2rad(180.0f), &CE_VEC3_UNIT_Z);
	ce_quat_init_polar(&q2, ce_deg2rad(270.0f), &CE_VEC3_UNIT_X);
	ce_quat_mul(&orientation, &q2, &q1);

	figentity = ce_figmng_create_figentity(ce_root_get_figmng(),
										figure_name, &complection,
										&position, &orientation,
										texture_names, scenemng->scenenode);

	if (-1 != anm_index) {
		ce_figentity_play_animation(figentity,
			ce_figentity_get_animation_name(figentity, anm_index));
	}

	return NULL != figentity;
}

static void idle(void)
{
	ce_scenemng_advance(scenemng);

	float elapsed = ce_timer_elapsed(scenemng->timer);

	ce_input_event_supply_advance(es, elapsed);

	if (ce_input_test(CE_KB_ESCAPE)) {
		ce_input_event_supply_del(es);
		ce_figentity_del(figentity);
		ce_figmesh_del(figmesh);
		ce_figproto_del(figproto);
		ce_scenemng_del(scenemng);
		ce_root_term();
		ce_gl_term();
		ce_input_term();
		ce_alloc_term();
		ce_logging_term();
		exit(EXIT_SUCCESS);
	}

	if (ce_input_event_triggered(toggle_bbox_event)) {
		scenemng->show_bboxes = !scenemng->show_bboxes;
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

	ce_figentity_advance(figentity, scenemng->anm_fps, elapsed);

	glutPostRedisplay();
}

static void display(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ce_scenemng_render(scenemng);

	glutSwapBuffers();
}

static void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	ce_camera_set_aspect(scenemng->camera, width, height);
}

static void usage(void)
{
	fprintf(stderr,
		"===============================================================================\n"
		"Cursed Earth is an open source, cross-platform port of Evil Islands\n"
		"Copyright (C) 2009-2010 Yanis Kurganov\n\n"
		"This program is free software: you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation, either version 3 of the License, or\n"
		"(at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
		"GNU General Public License for more details.\n"
		"===============================================================================\n\n"
		"This program is part of Cursed Earth spikes\n"
		"Figure Viewer %d.%d.%d - Control Evil Islands figures\n\n"
		"Usage: figviewer [options] <figure_name>\n"
		"Options:\n"
		"-b <ei_path> Path to EI base dir (current dir by default)\n"
		"-p <tex_name> Primary texture\n"
		"-s <tex_name> Secondary texture\n"
		"-a <anm_name> Play animation with specified name\n"
		"-f Start program in Full Screen mode\n"
		"-v Display program version\n"
		"-h Display this message\n\n"
		"Controls:\n"
		"1 Change strength\n"
		"2 Change dexterity\n"
		"3 Change height\n"
		"b Show/hide bounding boxes\n"
		"a Play next animation\n"
		"+/- Change animation FPS\n",
			CE_SPIKE_VERSION_MAJOR,
			CE_SPIKE_VERSION_MINOR,
			CE_SPIKE_VERSION_PATCH);
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

	int c;
	const char* ei_path = ".";
	const char* primary_texture_name = NULL;
	const char* secondary_texture_name = NULL;
	const char* anm_name = NULL;

	bool fullscreen = false;

	opterr = 0;

	while (-1 != (c = getopt(argc, argv, ":b:p:s:a:fvh")))  {
		switch (c) {
		case 'b':
			ei_path = optarg;
			break;
		case 'p':
			primary_texture_name = optarg;
			break;
		case 's':
			secondary_texture_name = optarg;
			break;
		case 'a':
			anm_name = optarg;
			break;
		case 'f':
			fullscreen = true;
			break;
		case 'v':
			fprintf(stderr, "%d.%d.%d\n", CE_SPIKE_VERSION_MAJOR,
					CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH);
			return 0;
		case 'h':
			usage();
			return 0;
		case ':':
			usage();
			fprintf(stderr, "\nOption '-%c' requires an argument\n", optopt);
			return 1;
		case '?':
			usage();
			fprintf(stderr, "\nUnknown option '-%c'\n", optopt);
			return 1;
		default:
			assert(false);
			usage();
			return 1;
		}
	}

	if (optind == argc) {
		usage();
		fprintf(stderr, "\nPlease, specify a figure name\n");
		return 1;
	}

	if (argc - optind > 1) {
		usage();
		fprintf(stderr, "\nToo much non-option arguments:\n");
		for (int i = optind; i < argc; ++i) {
			fprintf(stderr, "%s\n", argv[i]);
		}
		return 1;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);

	if (fullscreen) {
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "%dx%d:32",
			glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
		glutGameModeString(buffer);
		if (!glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
			ce_logging_warning("main: full screen mode is not available");
			fullscreen = false;
		}
	}

	if (fullscreen) {
		glutEnterGameMode();
	} else {
		glutInitWindowPosition(100, 100);
		glutInitWindowSize(1024, 768);
		glutCreateWindow("Cursed Earth: Figure Viewer");
	}

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	ce_input_init();
	ce_gl_init();

	if (!ce_root_init(ei_path)) {
		return 1;
	}

	if (NULL == (scenemng = ce_scenemng_new())) {
		return 1;
	}

	if (NULL != primary_texture_name) {
		texture_names[0] = primary_texture_name;
	}

	if (NULL != secondary_texture_name) {
		texture_names[1] = secondary_texture_name;
	}

	figure_name = argv[optind];

	if (!update_figentity()) {
		return 1;
	}

	if (NULL != anm_name) {
		if (ce_figentity_play_animation(figentity, anm_name)) {
			int anm_count = ce_figentity_get_animation_count(figentity);
			for (anm_index = 0; anm_index < anm_count &&
					0 == ce_strcasecmp(anm_name,
					ce_figentity_get_animation_name(figentity, anm_index));
					++anm_index) {
			}
		} else {
			ce_logging_warning("main: could not play animation: '%s'", anm_name);
		}
	}

	ce_vec3 eye;
	ce_vec3_init(&eye, 0.0f, 2.0f, -4.0f);

	ce_camera_set_near(scenemng->camera, 0.1f);
	ce_camera_set_eye(scenemng->camera, &eye);
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
