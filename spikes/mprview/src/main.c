#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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
		camera_delete(cam);
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
		camera_yaw_pitch(deg2rad(-0.25f * input_mouse_offset_x()),
						deg2rad(-0.25f * input_mouse_offset_y()), cam);
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

	mprfile_apply_frustum(&f, mpr);

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

int main(int argc, char* argv[])
{
	if (3 != argc) {
		printf("Usage: %s <ei_path> <mpr_name>\n", argv[0]);
		return 1;
	}

	glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);

	glutInitWindowPosition(100, 600);
	glutInitWindowSize(400, 300);
	glutInit(&argc, argv);

	glutCreateWindow("Cursed Earth Spikes: MPR Viewer");
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	cealloc_open();

	logging_open();
	logging_set_level(LOGGING_DEBUG_LEVEL);

	input_open();

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

	vec3 eye;
	vec3_init(0.0f, mprfile_get_max_height(mpr), 0.0f, &eye);

	cam = camera_new();
	camera_set_eye(&eye, cam);
	camera_yaw_pitch(deg2rad(45.0f), deg2rad(30.0f), cam);

	tmr = timer_open();

	glutMainLoop();
	return 0;
}
