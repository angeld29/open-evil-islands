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
#include <time.h>

#include <getopt.h>
#include <GL/glut.h>

#include "cealloc.h"
#include "cegl.h"
#include "ceinput.h"
#include "logging.h"
#include "timer.h"
#include "memfile.h"
#include "resfile.h"
#include "texture.h"

resfile* res;
texture* tex;
timer* tmr;

bool rnd = false;
int delay = 500;
bool slideshow = true;

static void idle(void)
{
	timer_advance(tmr);

	float elapsed = timer_elapsed(tmr);

	ceinput_advance(elapsed);

	if (ceinput_test(CEKB_ESCAPE)) {
		timer_close(tmr);
		texture_close(tex);
		resfile_close(res);
		ceinput_close();
		logging_close();
		cealloc_close();
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

	void* data = malloc(resfile_node_size(index, res));
	if (NULL == data || !resfile_node_data(index, data, res)) {
		free(data);
		return false;
	}

	tex = texture_open(data);
	free(data);

	return NULL != tex;
}

static void next_texture(int index)
{
	if (0 == resfile_node_count(res)) {
		return;
	}

	if (0 > index || index >= resfile_node_count(res)) {
		printf("Next cycle...\n");
		index = 0;
	}

	int d = delay;

	if (!generate_texture(index)) {
		printf("Could not load texture '%s'\n", resfile_node_name(index, res));
		d = 0;
	} else {
		glutPostRedisplay();
	}

	if (slideshow) {
		glutTimerFunc(d, next_texture, rnd ?
			rand() % resfile_node_count(res) : index + 1);
	}
}

static void usage(const char* name)
{
	printf("Usage: %s [options] <res_path>\n"
			"Options:\n"
			"-r Random texture mode\n"
			"-d <delay, msec> Slideshow delay (default: %d)\n"
			"-i <index> Specify texture index (slideshow disabled)\n"
			"-n <name> Specify texture name (slideshow disabled)\n"
			"-h Show this message\n", name, delay);
}

int main(int argc, char* argv[])
{
	srand(time(NULL));

	int c, index = -1;
	const char* name = NULL;

	while (-1 != (c = getopt(argc, argv, "rd:i:n:h")))  {
		switch (c) {
		case 'r':
			rnd = true;
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'i':
			slideshow = false;
			index = atoi(optarg);
			break;
		case 'n':
			slideshow = false;
			name = optarg;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (optind >= argc) {
		printf("Please, specify a path to any res file with textures\n");
		usage(argv[0]);
		return 1;
	}

	glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE);

	glutInitWindowPosition(100, 600);
	glutInitWindowSize(400, 300);
	glutInit(&argc, argv);

	glutCreateWindow("Cursed Earth: Texture Viewer");
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	cealloc_open();

	logging_open();
	logging_set_level(LOGGING_DEBUG_LEVEL);

	ceinput_open();

	cegl_init();

	res = resfile_open_file(argv[optind]);
	if (NULL == res) {
		printf("Could not open file '%s'\n", argv[optind]);
		return 1;
	}

	if (NULL != name) {
		index = resfile_node_index(name, res);
		if (-1 == index) {
			printf("Could not find texture '%s'\n", name);
			return 1;
		}
	} else if (-1 != index) {
		if (0 > index || index >= resfile_node_count(res)) {
			printf("Invalid index '%d'\n", index);
			return 1;
		}
	} else {
		index = 0;
	}

	glutTimerFunc(0, next_texture, index);

	tmr = timer_open();

	glutMainLoop();
	return 0;
}
