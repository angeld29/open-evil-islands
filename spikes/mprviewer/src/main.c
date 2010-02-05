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
#include "cecfgfile.h"
#include "celightcfg.h"

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

ce_lightcfg gipat_light;
ce_lightcfg ingos_light;
ce_lightcfg suslanger_light;
ce_lightcfg* light_cfg;

ce_input_event_supply* es;
ce_input_event* night_event;
int night_index;

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
	glClearColor(light_cfg->sky[night_index][0], light_cfg->sky[night_index][1],
		light_cfg->sky[night_index][2], light_cfg->sky[night_index][3]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	glEnable(GL_LIGHTING);

#ifdef GL_VERSION_1_2
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#endif
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	/*float coef = 3.0f;
	float aaa[4] = { light_cfg->ambient[night_index][0] * coef,
					light_cfg->ambient[night_index][1] * coef,
					light_cfg->ambient[night_index][2] * coef,
					light_cfg->ambient[night_index][3] * coef};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, aaa);*/
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_cfg->ambient[night_index]);

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_cfg->sunlight[night_index]);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, light_cfg->sunlight[night_index]);
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, (float[]){ 0.0f, 0.0f, 0.0f, 0.0f });

	//glEnable(GL_LIGHT1);
	//glLightfv(GL_LIGHT1, GL_POSITION, (float[]){ 0.0f, 0.0f, 0.0f, 1.0f });
	//glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, ce_camera_get_fov(cam) / 3.0f);
	//glLightfv(GL_LIGHT1, GL_DIFFUSE, sss);

	ce_camera_setup(cam);

	glLightfv(GL_LIGHT0, GL_POSITION, (float[]){ 0.0f, 30.0f, 0.0f, 1.0f });

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

static bool load_light(ce_lightcfg* light, const char* ei_path,
											const char* cfg_name)
{
	char cfg_path[512];
	snprintf(cfg_path, sizeof(cfg_path), "%s/Config/%s.ini", ei_path, cfg_name);

	ce_cfgfile* cfg = ce_cfgfile_open(cfg_path);
	if (NULL == cfg) {
		ce_logging_error("could not open config file '%s'", cfg_path);
		return false;
	}

	bool ok = ce_lightcfg_init(light, cfg);

	ce_cfgfile_close(cfg);
	return ok;
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
		"This program is part of Cursed Earth spikes\n"
		"mprviewer %d.%d.%d - View and explore Evil Islands maps\n\n"
		"Usage: mprviewer [options] <zone_name>\n"
		"Where: <zone_name> Any zone_name.mpr file in 'ei_path/Maps'\n"
		"Options:\n"
		"-b <ei_path> Path to EI base dir (current dir by default)\n"
		"-f Start program in Full Screen mode\n"
		"-v Display program version\n"
		"-h Display this message\n", CE_SPIKE_VERSION_MAJOR,
		CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH);
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
	bool fullscreen = false;

	opterr = 0;

	while (-1 != (c = getopt(argc, argv, ":b:fvh")))  {
		switch (c) {
		case 'b':
			ei_path = optarg;
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
		fprintf(stderr, "\nPlease, specify any MPR file name\n");
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

	char path[512];

	snprintf(path, sizeof(path), "%s/Res/textures.res", ei_path);
	ce_resfile* tex_res = ce_resfile_open_file(path);
	if (NULL == tex_res) {
		fprintf(stderr, "Could not open file '%s'\n", path);
		return 1;
	}

	snprintf(path, sizeof(path), "%s/Maps/%s.mpr", ei_path, argv[optind]);
	ce_resfile* mpr_res = ce_resfile_open_file(path);
	if (NULL == mpr_res) {
		fprintf(stderr, "Could not open file '%s'\n", path);
		return 1;
	}

	mpr = ce_mprfile_open(mpr_res, tex_res);
	if (!mpr) {
		fprintf(stderr, "Could not open mpr file\n");
		return 1;
	}

	ce_resfile_close(tex_res);
	ce_resfile_close(mpr_res);

	if (!load_light(&gipat_light, ei_path, "lightsgipat") ||
			!load_light(&ingos_light, ei_path, "lightsingos") ||
			!load_light(&suslanger_light, ei_path, "lightssuslanger")) {
		ce_logging_error("failed to load lighting configuration");
		return 1;
	}
	light_cfg = &gipat_light;

	ce_vec3 eye;
	ce_vec3_init(&eye, 0.0f, ce_mprfile_get_max_height(mpr), 0.0f);

	cam = ce_camera_open();
	ce_camera_set_eye(cam, &eye);
	ce_camera_yaw_pitch(cam, ce_deg2rad(45.0f), ce_deg2rad(30.0f));

	tmr = ce_timer_open();

	es = ce_input_event_supply_open();
	night_event = ce_input_create_single_front_event(es,
					ce_input_create_button_event(es, CE_KB_N));

	glutMainLoop();
	return 0;
}
