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
#include <limits.h>
#include <time.h>
#include <assert.h>

#include <getopt.h>
#include <GL/glut.h>

#include "celib.h"
#include "cegl.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceinput.h"
#include "timer.h"
#include "memfile.h"
#include "resfile.h"
#include "texture.h"

#ifndef CE_SPIKE_VERSION_MAJOR
#define CE_SPIKE_VERSION_MAJOR 0
#endif
#ifndef CE_SPIKE_VERSION_MINOR
#define CE_SPIKE_VERSION_MINOR 0
#endif
#ifndef CE_SPIKE_VERSION_PATCH
#define CE_SPIKE_VERSION_PATCH 0
#endif

#define DEFAULT_DELAY 500

resfile* res;
texture* tex;
timer* tmr;

bool rndmode = false;
int delay = DEFAULT_DELAY;
bool slideshow = true;

static void idle(void)
{
	timer_advance(tmr);

	float elapsed = timer_elapsed(tmr);

	ce_input_advance(elapsed);

	if (ce_input_test(CE_KB_ESCAPE)) {
		timer_close(tmr);
		texture_close(tex);
		resfile_close(res);
		ce_input_close();
		ce_logging_close();
		ce_alloc_close();
		exit(0);
	}
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (NULL != tex) {
  		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		texture_bind(tex);
	} else {
		glColor3f(1.0f, 0.0f, 0.0f);
	}

	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
	glEnd();

	if (NULL != tex) {
		texture_unbind(tex);
		glDisable(GL_BLEND);
	}

	glutSwapBuffers();
}

static void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

static bool generate_texture(int index)
{
	texture_close(tex);
	tex = NULL;

	void* data = ce_alloc(resfile_node_size(index, res));
	if (NULL == data || !resfile_node_data(index, data, res)) {
		ce_free(data, resfile_node_size(index, res));
		return false;
	}

	tex = texture_open(data);
	ce_free(data, resfile_node_size(index, res));

	return NULL != tex;
}

static void next_texture(int index)
{
	if (0 == resfile_node_count(res)) {
		return;
	}

	if (0 > index || index >= resfile_node_count(res)) {
		fprintf(stderr, "All textures (%d) have been browsed. "
			"Let's make a fresh start.\n", resfile_node_count(res));
		index = 0;
	}

	int d = delay;

	if (!generate_texture(index)) {
		fprintf(stderr, "Could not load texture '%s'.\n",
			resfile_node_name(index, res));
		d = 0;
	} else {
		glutPostRedisplay();
	}

	if (slideshow) {
		glutTimerFunc(d, next_texture, rndmode ?
			rand() % resfile_node_count(res) : index + 1);
	}
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
		"texviewer %d.%d.%d - View Evil Islands textures.\n\n"
		"Usage: texviewer [options] <res_path>\n"
		"Where: <res_path> Path to *.res with textures.\n"
		"Options:\n"
		"-r Show textures in random mode.\n"
		"-d <delay, msec> Slideshow delay (default: %d).\n"
		"-n <name> Specify texture name (slideshow will be disabled).\n"
		"-i <index> Specify texture index (slideshow will be disabled).\n"
		"-v Display program version.\n"
		"-h Display this message.\n", CE_SPIKE_VERSION_MAJOR,
		CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH, DEFAULT_DELAY);
}

int main(int argc, char* argv[])
{
	srand(time(NULL));

	int c, index = -1;
	const char* name = NULL;

	opterr = 0;

	while (-1 != (c = getopt(argc, argv, ":rd:n:i:vh")))  {
		switch (c) {
		case 'r':
			rndmode = true;
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'n':
			slideshow = false;
			name = optarg;
			break;
		case 'i':
			slideshow = false;
			if ((index = atoi(optarg)) < 0) {
				index = INT_MAX;
			}
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
		fprintf(stderr, "\nPlease, specify a path to any res file with textures.\n");
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
	glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(400, 300);
	glutCreateWindow("Cursed Earth: Texture Viewer");

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	ce_alloc_open();
	ce_logging_open();
#ifdef NDEBUG
	ce_logging_set_level(CE_LOGGING_LEVEL_WARNING);
#else
	ce_logging_set_level(CE_LOGGING_LEVEL_DEBUG);
#endif

	ce_input_open();
	ce_gl_init();

	res = resfile_open_file(argv[optind]);
	if (NULL == res) {
		fprintf(stderr, "Could not open file '%s'.\n", argv[optind]);
		return 1;
	}

	if (NULL != name) {
		index = resfile_node_index(name, res);
		if (-1 == index) {
			fprintf(stderr, "Could not find texture '%s'.\n", name);
			return 1;
		}
	} else if (-1 != index) {
		if (0 > index || index >= resfile_node_count(res)) {
			fprintf(stderr,
				"Invalid index: %d. Allowed range for '%s' is [0...%d]\n",
				index, argv[optind], resfile_node_count(res) - 1);
			return 1;
		}
	} else {
		index = rndmode ? rand() % resfile_node_count(res) : 0;
	}

	glutTimerFunc(0, next_texture, index);

	tmr = timer_open();

	glutMainLoop();
	return 0;
}
