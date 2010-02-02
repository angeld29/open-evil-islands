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
#include <assert.h>

#include <getopt.h>
#include <GL/glut.h>

#include "cegl.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceinput.h"
#include "cetimer.h"
#include "cecamera.h"
#include "cemath.h"
#include "vec3.h"
#include "frustum.h"
#include "resfile.h"
#include "mprfile.h"

#ifndef CE_SPIKE_VERSION_MAJOR
#define CE_SPIKE_VERSION_MAJOR 0
#endif
#ifndef CE_SPIKE_VERSION_MINOR
#define CE_SPIKE_VERSION_MINOR 0
#endif
#ifndef CE_SPIKE_VERSION_PATCH
#define CE_SPIKE_VERSION_PATCH 0
#endif

mprfile* mpr;
ce_camera* cam;
ce_timer* tmr;

ce_input_event_supply* es;
ce_input_event* night_event;

static void idle(void)
{
	ce_timer_advance(tmr);

	float elapsed = ce_timer_elapsed(tmr);

	ce_input_advance(elapsed);
	ce_input_event_supply_advance(es, elapsed);

	if (ce_input_test(CE_KB_ESCAPE)) {
		ce_timer_close(tmr);
		ce_camera_close(cam);
		mprfile_close(mpr);
		ce_input_event_supply_close(es);
		ce_input_close();
		ce_logging_close();
		ce_alloc_close();
		if (glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
			glutLeaveGameMode();
		}
		exit(0);
	}

	if (ce_input_test(CE_KB_LEFT)) {
		ce_camera_move(-10.0f * elapsed, 0.0f, cam);
	}

	if (ce_input_test(CE_KB_UP)) {
		ce_camera_move(0.0f, 10.0f * elapsed, cam);
	}

	if (ce_input_test(CE_KB_RIGHT)) {
		ce_camera_move(10.0f * elapsed, 0.0f, cam);
	}

	if (ce_input_test(CE_KB_DOWN)) {
		ce_camera_move(0.0f, -10.0f * elapsed, cam);
	}

	if (ce_input_test(CE_MB_WHEELUP)) {
		ce_camera_zoom(5.0f, cam);
	}

	if (ce_input_test(CE_MB_WHEELDOWN)) {
		ce_camera_zoom(-5.0f, cam);
	}

	if (ce_input_test(CE_MB_RIGHT)) {
		ce_camera_yaw_pitch(ce_deg2rad(-0.25f * ce_input_mouse_offset_x()),
						ce_deg2rad(-0.25f * ce_input_mouse_offset_y()), cam);
	}

	if (ce_input_event_triggered(night_event)) {
		mprfile_set_night(!mprfile_get_night(mpr), mpr);
	}

	glutPostRedisplay();
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_POSITION, (float[]){ 0.0f, 0.0f, 0.0f, 1.0f });
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, ce_camera_get_fov(cam) / 2.0f);
	glLightfv(GL_LIGHT0, GL_AMBIENT, (float[]){ 2.0f, 2.0f, 2.0f, 1.0f });
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (float[]){ 0.5f, 0.5f, 0.5f, 1.0f });
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	ce_camera_setup(cam);

	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(100.0f, 0.0f, 0.0f);
	glEnd();
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 100.0f, 0.0f);
	glEnd();
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 100.0f);
	glEnd();

	vec3 eye, forward, right, up;
	frustum f;

	frustum_init(ce_camera_get_fov(cam), ce_camera_get_aspect(cam),
		ce_camera_get_near(cam), ce_camera_get_far(cam),
		ce_camera_get_eye(&eye, cam), ce_camera_get_forward(&forward, cam),
		ce_camera_get_right(&right, cam), ce_camera_get_up(&up, cam), &f);

	mprfile_apply_frustum(&eye, &f, mpr);
	mprfile_render(mpr);

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

	glutSwapBuffers();
}

static void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	ce_camera_set_aspect(width, height, cam);
}

static void usage()
{
	fprintf(stderr,
		"===============================================================================\n"
		"Cursed Earth is an open source, cross-platform port of Evil Islands.\n"
		"Copyright (C) 2009-2010 Yanis Kurganov.\n\n"
		"This program is free software: you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation, either version 3 of the License, or\n"
		"(at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
		"GNU General Public License for more details.\n"
		"===============================================================================\n\n"
		"This program is part of Cursed Earth spikes.\n"
		"mprviewer %d.%d.%d - View and explore Evil Islands maps.\n\n"
		"Usage: mprviewer [options] <mpr_path>\n"
		"Where: <mpr_path> Path to 'EI/Maps/*.mpr'.\n"
		"Options:\n"
		"-t <tex_path> Path to 'EI/Res/textures.res'. Required.\n"
		"-f Start program in Full Screen mode.\n"
		"-n Start at night (experimental). "
			"Toggle the night by pressing 'N' key in game.\n"
		"-v Display program version.\n"
		"-h Display this message.\n", CE_SPIKE_VERSION_MAJOR,
		CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH);
}

int main(int argc, char* argv[])
{
	int c;
	const char* tex_path = NULL;
	bool fullscreen = false;
	bool night = false;

	opterr = 0;

	while (-1 != (c = getopt(argc, argv, ":t:fnvh")))  {
		switch (c) {
		case 't':
			tex_path = optarg;
			break;
		case 'f':
			fullscreen = true;
			break;
		case 'n':
			night = true;
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
			fprintf(stderr, "\nOption '-%c' requires an argument.\n", optopt);
			return 1;
		case '?':
			usage();
			fprintf(stderr, "\nUnknown option '-%c'.\n", optopt);
			return 1;
		default:
			assert(false);
			usage();
			return 1;
		}
	}

	if (optind == argc) {
		usage();
		fprintf(stderr, "\nPlease, specify a path to any MPR file.\n");
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

	if (NULL == tex_path) {
		usage();
		fprintf(stderr, "\nPlease, specify a path to 'textures.res'.\n");
		return 1;
	}

	ce_alloc_open();
	ce_logging_open();
#ifdef NDEBUG
	ce_logging_set_level(CE_LOGGING_LEVEL_WARNING);
#else
	ce_logging_set_level(CE_LOGGING_LEVEL_DEBUG);
#endif

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);

	if (fullscreen) {
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "%dx%d:32",
			glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
		glutGameModeString(buffer);
		if (!glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
			ce_logging_warning("Full Screen mode is not available.");
			fullscreen = false;
		}
	}

	if (fullscreen) {
		glutEnterGameMode();
	} else {
		glutInitWindowPosition(100, 100);
		glutInitWindowSize(1024, 768);
		glutCreateWindow("Cursed Earth: MPR Viewer");
	}

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	ce_input_open();
	ce_gl_init();

	resfile* tex_res = resfile_open_file(tex_path);
	if (NULL == tex_res) {
		fprintf(stderr, "Could not open file '%s'.\n", tex_path);
		return 1;
	}

	resfile* mpr_res = resfile_open_file(argv[optind]);
	if (NULL == mpr_res) {
		fprintf(stderr, "Could not open file '%s'.\n", argv[optind]);
		return 1;
	}

	mpr = mprfile_open(mpr_res, tex_res);
	if (!mpr) {
		fprintf(stderr, "Could not open mpr file.\n");
		return 1;
	}

	resfile_close(tex_res);
	resfile_close(mpr_res);

	mprfile_set_night(night, mpr);

	vec3 eye;
	vec3_init(0.0f, mprfile_get_max_height(mpr), 0.0f, &eye);

	cam = ce_camera_open();
	ce_camera_set_eye(&eye, cam);
	ce_camera_yaw_pitch(ce_deg2rad(45.0f), ce_deg2rad(30.0f), cam);

	tmr = ce_timer_open();

	es = ce_input_event_supply_open();
	night_event = ce_input_create_single_front_event(es,
					ce_input_create_button_event(es, CE_KB_N));

	glutMainLoop();
	return 0;
}
