#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <GL/glut.h>

#include "cemath.h"
#include "cegl.h"
#include "logging.h"
#include "timer.h"
#include "input.h"
#include "camera.h"
#include "resfile.h"
#include "mprfile.h"

mprfile* mpr;
camera* cam;

static void idle(void)
{
	timer_advance();

	float elapsed = timer_elapsed();

	input_advance(elapsed);

	if (input_test(KB_ESCAPE)) {
		camera_delete(cam);
		mprfile_close(mpr);
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
		camera_yaw_pitch(deg2rad(-0.13f * input_mouse_offset_x()),
						deg2rad(-0.13f * input_mouse_offset_y()), cam);
	}

	glutPostRedisplay();
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera_setup(cam);

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

	mprfile_debug_render(mpr);

	glutSwapBuffers();
}

static void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	camera_set_aspect(width, height, cam);
}

int main(int argc, char* argv[])
{
	if (3 != argc) {
		printf("Usage: %s ei_path mpr_name\n", argv[0]);
		return 1;
	}

	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1024, 768);
	glutInit(&argc, argv);

	glutCreateWindow("Cursed Earth Spikes: MPR Viewer");
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	logging_open();
	logging_set_level(LOGGING_DEBUG_LEVEL);
	atexit(logging_close);

	input_open();
	atexit(input_close);

	gl_init();

	char tex_path[512];
	snprintf(tex_path, sizeof(tex_path), "%s/Res/textures.res", argv[1]);

	char mpr_path[512];
	snprintf(mpr_path, sizeof(mpr_path), "%s/Maps/%s", argv[1], argv[2]);

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

	mprfile_debug_print(mpr);

	cam = camera_new();
	camera_set_eye((float[]){ 25.0f, 50.0f, 25.0f }, cam);
	camera_yaw_pitch(deg2rad(90.0f), deg2rad(60.0f), cam);

	timer_start();

	glutMainLoop();
	return 0;
}
