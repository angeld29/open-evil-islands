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

#include "cegl.h"
#include "cemath.h"
#include "celogging.h"
#include "cealloc.h"
#include "cescenemng.h"

// TODO: remove GLUT
#include <GL/glut.h>
#ifdef _WIN32
// fu... win32
#undef near
#undef far
#endif

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

static ce_input_event_supply* es;
static ce_input_event* toggle_bbox_event;

static void idle(void)
{
	ce_scenemng_advance(scenemng);

	float elapsed = ce_timer_elapsed(scenemng->timer);

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
		scenemng->show_bboxes = !scenemng->show_bboxes;
	}

	if (ce_input_test(CE_KB_LEFT)) {
		ce_camera_move(scenemng->camera, -10.0f * elapsed, 0.0f);
	}

	if (ce_input_test(CE_KB_UP)) {
		ce_camera_move(scenemng->camera, 0.0f, 10.0f * elapsed);
	}

	if (ce_input_test(CE_KB_RIGHT)) {
		ce_camera_move(scenemng->camera, 10.0f * elapsed, 0.0f);
	}

	if (ce_input_test(CE_KB_DOWN)) {
		ce_camera_move(scenemng->camera, 0.0f, -10.0f * elapsed);
	}

	if (ce_input_test(CE_MB_WHEELUP)) {
		ce_camera_zoom(scenemng->camera, 5.0f);
	}

	if (ce_input_test(CE_MB_WHEELDOWN)) {
		ce_camera_zoom(scenemng->camera, -5.0f);
	}

	if (ce_input_test(CE_MB_RIGHT)) {
		ce_vec2 offset = ce_input_mouse_offset();
		ce_camera_yaw_pitch(scenemng->camera, ce_deg2rad(-0.25f * offset.x),
												ce_deg2rad(-0.25f * offset.y));
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
	ce_logging_set_level(CE_LOGGING_LEVEL_INFO);
#else
	ce_logging_set_level(CE_LOGGING_LEVEL_DEBUG);
#endif
	ce_alloc_init();

	ce_optparse* optparse = ce_optparse_new(CE_SPIKE_VERSION_MAJOR,
		CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
		"This program is part of Cursed Earth spikes\n"
		"MPR Viewer %d.%d.%d - Explore clean Evil Islands zones\n\n"
		"controls:\n"
		"b   show/hide bounding boxes",
		CE_SPIKE_VERSION_MAJOR, CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH);

	ce_optgroup* general = ce_scenemng_create_group_general(optparse);

	ce_optarg* zone_name = ce_optparse_create_arg(optparse, "zone_name",
		"any zone_name.mpr file in 'ei_path/Maps'");

	if (!ce_optparse_parse_args(optparse, argc, argv)) {
		return EXIT_FAILURE;
	}

	ce_optoption* full_screen = ce_optgroup_find_option(general, "full_screen");

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);

	if (ce_optoption_value_bool(full_screen)) {
		ce_logging_write("main: trying to enter full screen mode...");

		char buffer[32];
		int width = glutGet(GLUT_SCREEN_WIDTH);
		int height = glutGet(GLUT_SCREEN_HEIGHT);

		for (int bpp = 32; bpp >= 16; bpp -= 16) {
			if (glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
				break;
			}

			for (int hertz = 100; hertz >= 10; hertz -= 10) {
				if (glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
					break;
				}

				snprintf(buffer, sizeof(buffer),
					"%dx%d:%d@%d", width, height, bpp, hertz);

				glutGameModeString(buffer);

				if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
					ce_logging_write("main: entering full "
						"screen mode %s...", buffer);
					glutEnterGameMode();
				} else {
					ce_logging_warning("main: failed to enter "
						"full screen mode %s", buffer);
				}
			}
		}

		if (!glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
			ce_logging_warning("main: full screen mode is not available");
		}
	}

	if (!glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
		const int width = 1024;
		const int height = 768;

		glutInitWindowPosition(100, 100);
		glutInitWindowSize(width, height);
		glutCreateWindow("Cursed Earth: MPR Viewer");

		ce_logging_write("main: entering window mode %dx%d...", width, height);
	}

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	ce_input_init();
	ce_gl_init();

	scenemng = ce_scenemng_new(optparse);

	if (NULL == ce_scenemng_create_terrain(scenemng, zone_name->value->str,
					&CE_VEC3_ZERO, &CE_QUAT_IDENTITY, NULL)) {
		return EXIT_FAILURE;
	}

	ce_vec3 position;
	ce_vec3_init(&position, 0.0f, scenemng->terrain->mprfile->max_y, 0.0f);

	ce_camera_set_position(scenemng->camera, &position);
	ce_camera_yaw_pitch(scenemng->camera, ce_deg2rad(45.0f), ce_deg2rad(30.0f));

	es = ce_input_event_supply_new();
	toggle_bbox_event = ce_input_event_supply_single_front_event(es,
					ce_input_event_supply_button_event(es, CE_KB_B));

	ce_optparse_del(optparse);

	glutMainLoop();
	return EXIT_SUCCESS;
}
