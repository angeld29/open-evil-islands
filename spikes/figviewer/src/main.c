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

ce_scenemng* scenemng;

ce_figproto* figproto;
ce_figmesh* figmesh;
ce_figentity* figentity;

ce_input_event_supply* es;
ce_input_event* test_event;

int ttt;

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

	if (ce_input_event_triggered(test_event)) {
		if (++ttt > 50) {
			ttt = 0;
		}
	}

	ce_figentity_advance(figentity, 1.0f, elapsed);

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
	fprintf(stderr, "figviewer");
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
	const char* primary_texture_name = "default0";
	const char* secondary_texture_name = "default0";
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
		fprintf(stderr, "\nPlease, specify a model name\n");
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

	if (NULL == (figproto = ce_figprotomng_get_figproto(
			ce_root_get_figprotomng(), argv[optind]))) {
		ce_logging_fatal("main: failed to get figure proto: '%s'", argv[optind]);
		return 1;
	}

	ce_complection complection;
	ce_complection_init(&complection, 1.0f, 1.0f, 1.0f);

	figmesh = ce_figmesh_new(figproto, &complection);

	const char* texture_names[] = { primary_texture_name,
									secondary_texture_name };

	ce_vec3 position = CE_VEC3_ZERO;
	ce_vec3_init(&position, 1.0f, 0.0f, 1.0f);

	ce_quat orientation, q1 = CE_QUAT_IDENTITY, q2 = CE_QUAT_IDENTITY;
	ce_quat_init_polar(&q1, ce_deg2rad(153.0f), &CE_VEC3_UNIT_Z);
	ce_quat_init_polar(&q2, ce_deg2rad(208.0f), &CE_VEC3_UNIT_X);
	ce_quat_mul(&orientation, &q2, &q1);

	figentity = ce_figentity_new(figmesh, &position, &orientation,
								texture_names, scenemng->scenenode);

	if (NULL != anm_name) {
		if (!ce_figentity_play_animation(figentity, anm_name)) {
			ce_logging_warning("main: could not play animation: '%s'", anm_name);
		}
	}

	ce_vec3 eye;
	ce_vec3_init(&eye, 0.0f, 2.0f, -4.0f);

	ce_camera_set_near(scenemng->camera, 0.1f);
	ce_camera_set_eye(scenemng->camera, &eye);
	ce_camera_yaw_pitch(scenemng->camera, ce_deg2rad(180.0f), ce_deg2rad(30.0f));

	es = ce_input_event_supply_new();
	test_event = ce_input_event_supply_single_front_event(es,
		ce_input_event_supply_button_event(es, CE_KB_N));

	glutMainLoop();
	return EXIT_SUCCESS;
}
