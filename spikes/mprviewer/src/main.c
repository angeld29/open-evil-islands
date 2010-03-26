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

#include <GL/glut.h>

#ifdef _WIN32
// fu... win32
#undef near
#undef far
#endif

#include "cegl.h"
#include "celogging.h"
#include "cealloc.h"
#include "ceinput.h"
#include "cetimer.h"
#include "cemath.h"
#include "cevec3.h"
#include "cescenemng.h"
#include "cecfgfile.h"
#include "celightcfg.h"
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

#define DAY_NIGHT_CHANGE_SPEED_DEFAULT 0.08f

ce_scenemng* scenemng;
ce_lightcfg* lightcfg;

float time_of_day = 12.0f;
float day_night_change_speed = DAY_NIGHT_CHANGE_SPEED_DEFAULT;

ce_input_event_supply* es;

static void idle(void)
{
	ce_scenemng_advance(scenemng);

	float elapsed = ce_timer_elapsed(scenemng->timer);

	if ((time_of_day += day_night_change_speed * elapsed) >= 24.0f) {
		time_of_day = 0.0f;
	}

	ce_input_event_supply_advance(es, elapsed);

	if (ce_input_test(CE_KB_ESCAPE)) {
		ce_input_event_supply_del(es);
		ce_lightcfg_del(lightcfg);
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
	// FIXME: move to the engine!!!
	/*float time_index, time_next_index;
	float time_factor = modff(time_of_day, &time_index);

	if ((time_next_index = time_index + 1.0f) >= 24.0f) {
		time_next_index = 0.0f;
	}

	ce_color sky, ambient, sunlight;

	ce_color_lerp(&sky, time_factor,
		&lightcfg->sky[(int)time_index],
		&lightcfg->sky[(int)time_next_index]);

	ce_color_lerp(&ambient, time_factor,
		&lightcfg->ambient[(int)time_index],
		&lightcfg->ambient[(int)time_next_index]);

	ce_color_lerp(&sunlight, time_factor,
		&lightcfg->sunlight[(int)time_index],
		&lightcfg->sunlight[(int)time_next_index]);*/

	//glClearColor(sky.r, sky.g, sky.b, sky.a);

/*#ifdef GL_VERSION_1_2
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#endif
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (float[]) { ambient.r, ambient.g,
													ambient.b, ambient.a });*/

	// FIXME: sunlight after camera?
	/*glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, (float[]) { 0.0f, 1.0f, 0.0f, 0.0f });
	glLightfv(GL_LIGHT0, GL_AMBIENT, (float[]) { sunlight.r, sunlight.g,
												sunlight.b, sunlight.a });
	glLightfv(GL_LIGHT0, GL_DIFFUSE, (float[]) { 0.0f, 0.0f, 0.0f, 0.0f });*/

	//glDisable(GL_LIGHT0);

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

	ce_optparse* optparse = ce_optparse_new(CE_SPIKE_VERSION_MAJOR,
		CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH,
		"This program is part of Cursed Earth spikes\n"
		"MPR Viewer %d.%d.%d - Explore clean Evil Islands zones",
		CE_SPIKE_VERSION_MAJOR, CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH);
	ce_optgroup* general = ce_optparse_create_group(optparse, "general");
	ce_optoption* ei_path = ce_optgroup_create_option(general,
		"ei_path", 'b', "ei-path", CE_OPTACTION_STORE,
		"path to EI root dir (current dir by default)", ".");
	ce_optoption* full_screen = ce_optgroup_create_option(general,
		"full_screen", 'f', "full-screen", CE_OPTACTION_STORE_TRUE,
		"start program in Full Screen mode", NULL);
	/*ce_optoption* speed = ce_optgroup_create_option(general,
		"speed", 's', "speed", CE_OPTACTION_STORE,
		"day/night change speed ([0.0 ... 1.0])", NULL);*/
	ce_optarg* zone_name = ce_optparse_create_arg(optparse, "zone_name",
		"any zone_name.mpr file in 'ei_path/Maps'");

	if (!ce_optparse_parse_args(optparse, argc, argv)) {
		return EXIT_FAILURE;
	}

	//day_night_change_speed = ce_fclamp(atof(optarg), 0.0f, 1.0f);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);

	if (ce_optoption_value_bool(full_screen)) {
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
		glutCreateWindow("Cursed Earth: MPR Viewer");
	}

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	ce_input_init();
	ce_gl_init();

	if (NULL == (scenemng = ce_scenemng_new(ei_path->value->str))) {
		return EXIT_FAILURE;
	}

	if (NULL == ce_scenemng_create_terrain(scenemng, zone_name->value->str,
					&CE_VEC3_ZERO, &CE_QUAT_IDENTITY, NULL)) {
		return EXIT_FAILURE;
	}

	char cfg_path[512];
	snprintf(cfg_path, sizeof(cfg_path),
			"%s/Config/lightsgipat.ini", ei_path->value->str);

	ce_cfgfile* cfgfile = ce_cfgfile_open(cfg_path);
	if (NULL == cfgfile) {
		ce_logging_error("main: could not open config file: '%s'", cfg_path);
		return EXIT_FAILURE;
	}

	lightcfg = ce_lightcfg_new(cfgfile);
	if (NULL == lightcfg) {
		return EXIT_FAILURE;
	}

	ce_cfgfile_close(cfgfile);

	ce_vec3 position;
	ce_vec3_init(&position, 0.0f, scenemng->terrain->mprfile->max_y, 0.0f);

	ce_camera_set_position(scenemng->camera, &position);
	ce_camera_yaw_pitch(scenemng->camera, ce_deg2rad(45.0f), ce_deg2rad(30.0f));

	es = ce_input_event_supply_new();

	ce_optparse_del(optparse);

	glutMainLoop();
	return EXIT_SUCCESS;
}
