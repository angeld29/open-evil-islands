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

#include <getopt.h>
#include <GL/glut.h>

#include "cealloc.h"
#include "cemath.h"
#include "cegl.h"
#include "logging.h"
#include "timer.h"
#include "input.h"
#include "vec3.h"
#include "frustum.h"
#include "camera.h"
#include "resfile.h"
#include "mprfile.h"

mprfile* mpr;
camera* cam;
timer* tmr;

static void idle(void)
{
	timer_advance(tmr);

	float elapsed = timer_elapsed(tmr);

	input_advance(elapsed);

	if (input_test(KB_ESCAPE)) {
		timer_close(tmr);
		camera_close(cam);
		mprfile_close(mpr);
		input_close();
		logging_close();
		cealloc_close();
		exit(0);
	}

	if (input_test(KB_LEFT)) {
		camera_move(-10.0f * elapsed, 0.0f, cam);
	}

	if (input_test(KB_UP)) {
		camera_move(0.0f, 10.0f * elapsed, cam);
	}

	if (input_test(KB_RIGHT)) {
		camera_move(10.0f * elapsed, 0.0f, cam);
	}

	if (input_test(KB_DOWN)) {
		camera_move(0.0f, -10.0f * elapsed, cam);
	}

	if (input_test(MB_WHEELUP)) {
		camera_zoom(5.0f, cam);
	}

	if (input_test(MB_WHEELDOWN)) {
		camera_zoom(-5.0f, cam);
	}

	if (input_test(MB_RIGHT)) {
		camera_yaw_pitch(cedeg2rad(-0.25f * input_mouse_offset_x()),
						cedeg2rad(-0.25f * input_mouse_offset_y()), cam);
	}

	glutPostRedisplay();
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera_setup(cam);

	vec3 eye, forward, right, up;
	frustum f;

	frustum_init(camera_get_fov(cam), camera_get_aspect(cam),
		camera_get_near(cam), camera_get_far(cam),
		camera_get_eye(&eye, cam), camera_get_forward(&forward, cam),
		camera_get_right(&right, cam), camera_get_up(&up, cam), &f);

	mprfile_apply_frustum(&eye, &f, mpr);

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

	mprfile_render(mpr);

	glutSwapBuffers();
}

static void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	camera_set_aspect(width, height, cam);
}

static void usage(const char* name)
{
	printf("Usage: %s [options]\n"
			"Options:\n"
			"-t <tex_path> Path to EI/Res/textures.res\n"
			"-m <mpr_path> Path to EI/Maps/*.mpr\n"
			"-h Show this message\n", name);
}

int main(int argc, char* argv[])
{
	int c;
	const char* tex_path = NULL;
	const char* mpr_path = NULL;

	while (-1 != (c = getopt(argc, argv, "t:m:h")))  {
		switch (c) {
		case 't':
			tex_path = optarg;
			break;
		case 'm':
			mpr_path = optarg;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (optind != argc || NULL == tex_path || NULL == mpr_path) {
		usage(argv[0]);
		return 1;
	}

	glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1024, 768);
	glutInit(&argc, argv);

	glutCreateWindow("Cursed Earth: MPR Viewer");
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	cealloc_open();

	logging_open();
	logging_set_level(LOGGING_DEBUG_LEVEL);

	input_open();

	cegl_init();

	resfile* tex_res = resfile_open_file(tex_path);
	if (NULL == tex_res) {
		printf("Could not open file '%s'\n", tex_path);
		return 1;
	}

	resfile* mpr_res = resfile_open_file(mpr_path);
	if (NULL == mpr_res) {
		printf("Could not open file '%s'\n", mpr_path);
		resfile_close(tex_res);
		return 1;
	}

	mpr = mprfile_open(mpr_res, tex_res);
	if (!mpr) {
		printf("Could not open file '%s'\n", mpr_path);
		resfile_close(tex_res);
		resfile_close(mpr_res);
		return 1;
	}

	resfile_close(tex_res);
	resfile_close(mpr_res);

	vec3 eye;
	vec3_init(0.0f, mprfile_get_max_height(mpr), 0.0f, &eye);

	cam = camera_open();
	camera_set_eye(&eye, cam);
	camera_yaw_pitch(cedeg2rad(45.0f), cedeg2rad(30.0f), cam);

	tmr = timer_open();

	glutMainLoop();
	return 0;
}
