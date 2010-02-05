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
#include "cevec3.h"
#include "cefrustum.h"
#include "ceresfile.h"
#include "cemprfile.h"

#ifndef CE_SPIKE_VERSION_MAJOR
#define CE_SPIKE_VERSION_MAJOR 0
#endif
#ifndef CE_SPIKE_VERSION_MINOR
#define CE_SPIKE_VERSION_MINOR 0
#endif
#ifndef CE_SPIKE_VERSION_PATCH
#define CE_SPIKE_VERSION_PATCH 0
#endif

ce_mprfile* mpr;
ce_camera* cam;
ce_timer* tmr;

ce_input_event_supply* es;
ce_input_event* night_event;
int night_index;

GLint sunlight_gipat[24][3] = {
 	{63, 93, 157},
 	{63, 93, 157},
 	{92, 122, 165},
	{ 171, 131, 119},
 	{255, 140, 104},
 	{255, 198, 129},
 	{255, 248, 207},
 	{255, 255, 255},
 	{255, 255, 255},
	{ 255, 255, 255},
 	{255, 255, 255},
	{ 255, 255, 255},
 	{255, 255, 255},
 	{255, 255, 255},
 	{255, 255, 255},
 	{255, 255, 255},
	{ 255, 255, 255},
	{ 255, 255, 255},
	{ 255, 248, 207},
	{ 255, 202, 136},
 	{255, 123, 82},
 	{166, 128, 116},
	{ 92, 122, 165},
	{ 63, 93, 157}
};

GLint ambient_gipat[24][3] = {
 	{54, 83, 147},
	{ 54, 83, 147},
	{ 87, 89, 110},
 	{122, 81, 68},
 	{133, 96, 85},
 	{135, 117, 94},
 	{129, 135, 125},
	{ 129, 135, 125},
 	{129, 135, 125},
 	{129, 135, 125},
 	{129, 135, 125},
	{ 129, 135, 125},
	{ 129, 135, 125},
 	{129, 135, 125},
	{ 129, 135, 125},
	{ 129, 135, 125},
 	{129, 135, 125},
 	{129, 135, 125},
 	{129, 135, 125},
 	{135, 117, 94},
 	{133, 96, 85},
 	{122, 81, 68},
 	{87, 89, 110},
 	{54, 83, 147}
};

GLint sky_gipat[24][3] = {
 	{0, 0, 118},
 	{0, 0, 118},
 	{76, 78, 97},
	{151, 80, 58},
 	{196, 89, 56},
 	{175, 150, 126},
 	{138, 176, 177},
 	{53, 182, 198},
 	{46, 181, 217},
 	{46, 181, 217},
 	{46, 181, 217},
 	{46, 181, 217},
 	{46, 181, 217},
 	{46, 181, 217},
 	{46, 181, 217},
 	{46, 181, 217},
 	{46, 181, 217},
	{103, 181, 183},
 	{174, 168, 157},
 	{210, 154, 101},
 	{210, 95, 36},
 	{136, 81, 81},
 	{76, 78, 97},
 	{0, 0, 118}
};



GLint sunlight_ingos[24][3] = {
	{ 63, 93, 157 },
	{ 63, 93, 157 },
	{ 92, 113, 165 },
	{ 171, 131, 119 },
	{ 255, 140, 104 },
	{ 255, 198, 129 },
	{ 255, 248, 207 },
	{ 255, 255, 255 },
	{ 255, 255, 255 },
	{ 255, 255, 255 },
	{ 255, 255, 255 },
	{ 255, 255, 255 },
	{ 255, 255, 255 },
	{ 255, 255, 255 },
	{ 255, 255, 255 },
	{ 255, 255, 255 },
	{ 255, 255, 255 },
	{ 255, 255, 255 },
	{ 255, 248, 207 },
	{ 255, 202, 136 },
	{ 255, 123, 82 },
	{ 166, 128, 116 },
	{ 92, 122, 165 },
	{ 63, 93, 157 }
};

GLint ambient_ingos[24][3] = {
	{ 54, 83, 147 },
	{ 54, 83, 147 },
	{ 77, 87, 135 },
	{ 88, 94, 133 },
	{ 99, 104, 133 },
	{ 108, 115, 138 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 111, 121, 151 },
	{ 108, 115, 138 },
	{ 99, 104, 133 },
	{ 88, 94, 133 },
	{ 77, 87, 135 },
	{ 54, 83, 147 }
};

GLint sky_ingos[24][3] = {
	{ 0, 0, 118 },
	{ 0, 0, 118 },
	{ 75, 83, 95 },
	{ 151, 80, 58 },
	{ 196, 89, 56 },
	{ 175, 150, 126 },
	{ 138, 167, 177 },
	{ 53, 157, 198 },
	{ 46, 132, 217 },
	{ 46, 132, 217 },
	{ 46, 132, 217 },
	{ 46, 132, 217 },
	{ 46, 132, 217 },
	{ 46, 132, 217 },
	{ 46, 132, 217 },
	{ 46, 132, 217 },
	{ 53, 157, 198 },
	{ 103, 168, 183 },
	{ 174, 168, 157 },
	{ 210, 154, 101 },
	{ 210, 95, 36 },
	{ 136, 81, 81 },
	{ 75, 83, 95 },
	{ 0, 0, 118 }
};

static void idle(void)
{
	ce_timer_advance(tmr);

	float elapsed = ce_timer_elapsed(tmr);

	ce_input_advance(elapsed);
	ce_input_event_supply_advance(es, elapsed);

	if (ce_input_test(CE_KB_ESCAPE)) {
		ce_timer_close(tmr);
		ce_camera_close(cam);
		ce_mprfile_close(mpr);
		ce_gl_close();
		ce_input_event_supply_close(es);
		ce_input_close();
		ce_logging_term();
		ce_alloc_term();
		if (glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
			glutLeaveGameMode();
		}
		exit(0);
	}

	if (ce_input_test(CE_KB_LEFT)) {
		ce_camera_move(cam, -10.0f * elapsed, 0.0f);
	}

	if (ce_input_test(CE_KB_UP)) {
		ce_camera_move(cam, 0.0f, 10.0f * elapsed);
	}

	if (ce_input_test(CE_KB_RIGHT)) {
		ce_camera_move(cam, 10.0f * elapsed, 0.0f);
	}

	if (ce_input_test(CE_KB_DOWN)) {
		ce_camera_move(cam, 0.0f, -10.0f * elapsed);
	}

	if (ce_input_test(CE_MB_WHEELUP)) {
		ce_camera_zoom(cam, 5.0f);
	}

	if (ce_input_test(CE_MB_WHEELDOWN)) {
		ce_camera_zoom(cam, -5.0f);
	}

	if (ce_input_test(CE_MB_RIGHT)) {
		ce_camera_yaw_pitch(cam, ce_deg2rad(-0.25f * ce_input_mouse_offset_x()),
							ce_deg2rad(-0.25f * ce_input_mouse_offset_y()));
	}

	if (ce_input_event_triggered(night_event)) {
		if (++night_index >= 24) {
			night_index = 0;
		}
	}

	glutPostRedisplay();
}

static void display(void)
{
	glEnable(GL_LIGHTING);
	// TODO: disable it...
	glEnable(GL_NORMALIZE);

	float coef = 255.0f;

	float sky[4] = { sky_gipat[night_index][0] / coef,
					sky_gipat[night_index][1] / coef,
					sky_gipat[night_index][2] / coef,
					1.0f };

	glClearColor(sky[0], sky[1], sky[2], sky[3]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	// -----

	float aaa[4] = { ambient_gipat[night_index][0] / coef,
					ambient_gipat[night_index][1] / coef,
					ambient_gipat[night_index][2] / coef,
					1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, aaa);

#ifdef GL_VERSION_1_2
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#endif
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	// -----

	float sss[4] = { sunlight_gipat[night_index][0] / coef,
					sunlight_gipat[night_index][1] / coef,
					sunlight_gipat[night_index][2] / coef,
					1.0f };

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, sss);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, (float[]){ 0.0f, 0.0f, 0.0f, 0.0f });

	// -----

	//glEnable(GL_LIGHT1);
	//glLightfv(GL_LIGHT1, GL_POSITION, (float[]){ 0.0f, 0.0f, 0.0f, 1.0f });
	//glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, ce_camera_get_fov(cam) / 3.0f);
	//glLightfv(GL_LIGHT1, GL_DIFFUSE, sss);

	ce_camera_setup(cam);

	glLightfv(GL_LIGHT0, GL_POSITION, (float[]){ 0.0f, 50.0f, 0.0f, 1.0f });

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

	ce_vec3 eye, forward, right, up;
	ce_frustum f;

	ce_frustum_init(&f, ce_camera_get_fov(cam), ce_camera_get_aspect(cam),
		ce_camera_get_near(cam), ce_camera_get_far(cam),
		ce_camera_get_eye(cam, &eye), ce_camera_get_forward(cam, &forward),
		ce_camera_get_right(cam, &right), ce_camera_get_up(cam, &up));

	ce_mprfile_apply_frustum(mpr, &eye, &f);
	ce_mprfile_render(mpr);

	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

	glutSwapBuffers();
}

static void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	ce_camera_set_aspect(cam, width, height);
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

	ce_logging_init();
#ifdef NDEBUG
	ce_logging_set_level(CE_LOGGING_LEVEL_WARNING);
#else
	ce_logging_set_level(CE_LOGGING_LEVEL_DEBUG);
#endif
	ce_alloc_init();

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
	ce_gl_open();

	ce_resfile* tex_res = ce_resfile_open_file(tex_path);
	if (NULL == tex_res) {
		fprintf(stderr, "Could not open file '%s'.\n", tex_path);
		return 1;
	}

	ce_resfile* mpr_res = ce_resfile_open_file(argv[optind]);
	if (NULL == mpr_res) {
		fprintf(stderr, "Could not open file '%s'.\n", argv[optind]);
		return 1;
	}

	mpr = ce_mprfile_open(mpr_res, tex_res);
	if (!mpr) {
		fprintf(stderr, "Could not open mpr file.\n");
		return 1;
	}

	ce_resfile_close(tex_res);
	ce_resfile_close(mpr_res);

	ce_vec3 eye;
	ce_vec3_init(&eye, 100.0f, ce_mprfile_get_max_height(mpr), 0.0f);

	cam = ce_camera_open();
	ce_camera_set_eye(cam, &eye);
	ce_camera_yaw_pitch(cam, ce_deg2rad(0.0f), ce_deg2rad(30.0f));

	tmr = ce_timer_open();

	es = ce_input_event_supply_open();
	night_event = ce_input_create_single_front_event(es,
					ce_input_create_button_event(es, CE_KB_N));

	glutMainLoop();
	return 0;
}
