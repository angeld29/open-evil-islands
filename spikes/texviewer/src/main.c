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

#include <GL/glut.h>

#include "celib.h"
#include "cegl.h"
#include "celogging.h"
#include "cealloc.h"
#include "ceinput.h"
#include "cetimer.h"
#include "ceresfile.h"
#include "cetexture.h"
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

static ce_resfile* res;
static ce_texture* tex;
static ce_timer* tmr;

static bool rndmode = false;
static int delay;
static bool slideshow = true;

static void idle(void)
{
	ce_timer_advance(tmr);

	float elapsed = ce_timer_elapsed(tmr);

	ce_input_advance(elapsed);

	if (ce_input_test(CE_KB_ESCAPE)) {
		ce_timer_del(tmr);
		ce_texture_del(tex);
		ce_resfile_close(res);
		ce_gl_term();
		ce_input_term();
		ce_alloc_term();
		ce_logging_term();
		exit(EXIT_SUCCESS);
	}
}

static void display(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (NULL != tex) {
		glEnable(GL_TEXTURE_2D);
  		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		ce_texture_bind(tex);
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
		ce_texture_unbind(tex);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
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
	ce_texture_del(tex);
	tex = NULL;

	void* data = ce_resfile_node_data(res, index);
	if (NULL == data) {
		return false;
	}

	tex = ce_texture_new("unknown", data);
	ce_free(data, ce_resfile_node_size(res, index));

	return NULL != tex;
}

static void next_texture(int index)
{
	if (0 == res->node_count) {
		return;
	}

	if (0 > index || index >= res->node_count) {
		ce_logging_write("main: all textures (%d) have been browsed, "
			"let's make a fresh start", res->node_count);
		index = 0;
	}

	int d = delay;

	if (!generate_texture(index)) {
		ce_logging_error("main: could not load texture: '%s'",
			ce_resfile_node_name(res, index));
		d = 0;
	} else {
		glutPostRedisplay();
	}

	if (slideshow) {
		glutTimerFunc(d, next_texture, rndmode ?
			rand() % res->node_count : index + 1);
	}
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
		"Texture Viewer %d.%d.%d - View Evil Islands textures",
		CE_SPIKE_VERSION_MAJOR, CE_SPIKE_VERSION_MINOR, CE_SPIKE_VERSION_PATCH);
	ce_optgroup* general_grp = ce_optparse_create_group(optparse, "general");
	ce_optoption* random_opt = ce_optgroup_create_option(general_grp,
		"random", 'r', "random", CE_OPTACTION_STORE_TRUE,
		"show textures in random mode", NULL);
	ce_optoption* delay_opt = ce_optgroup_create_option(general_grp,
		"delay", 'd', "delay", CE_OPTACTION_STORE,
		"specify slideshow delay in msec", "500");
	ce_optoption* name_opt = ce_optgroup_create_option(general_grp,
		"name", 'n', "name", CE_OPTACTION_STORE,
		"specify texture name (slideshow will be disabled)", NULL);
	ce_optoption* index_opt = ce_optgroup_create_option(general_grp,
		"index", 'i', "index", CE_OPTACTION_STORE,
		"specify texture index (slideshow will be disabled)", NULL);
	ce_optarg* res_path_arg = ce_optparse_create_arg(optparse, "res_path",
		"path to *.res with textures");

	if (!ce_optparse_parse_args(optparse, argc, argv)) {
		return EXIT_FAILURE;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(400, 300);
	glutCreateWindow("Cursed Earth: Texture Viewer");

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	ce_input_init();
	ce_gl_init();

	res = ce_resfile_open_file(res_path_arg->value->str);
	if (NULL == res) {
		ce_logging_fatal("main: could not open file: '%s'",
							res_path_arg->value->str);
		return EXIT_FAILURE;
	}

	rndmode = ce_optoption_value_bool(random_opt);
	delay = ce_max(0, ce_optoption_value_int(delay_opt));

	if (!ce_optoption_value_empty(name_opt) ||
			!ce_optoption_value_empty(index_opt)) {
		slideshow = false;
	}

	int index = -1;
	srand(time(NULL));

	if (!ce_optoption_value_empty(name_opt)) {
		index = ce_resfile_node_index(res, name_opt->value->str);
		if (-1 == index) {
			ce_logging_error("main: could not find texture: '%s'\n",
											name_opt->value->str);
			return EXIT_FAILURE;
		}
	} else if (!ce_optoption_value_empty(index_opt)) {
		index = ce_optoption_value_int(index_opt);
		if (index < 0) {
			index = INT_MAX;
		}
		if (0 > index || index >= res->node_count) {
			ce_logging_error("main: invalid index: %d, "
							"allowed range [0...%d]\n",
							index, res->node_count - 1);
			return EXIT_FAILURE;
		}
	} else {
		index = rndmode ? rand() % res->node_count : 0;
	}

	glutTimerFunc(0, next_texture, index);

	tmr = ce_timer_new();

	ce_optparse_del(optparse);

	glutMainLoop();
	return EXIT_SUCCESS;
}
