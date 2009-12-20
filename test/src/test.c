#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <GL/glut.h>

#include "resfile.h"
#include "mprfile.h"

mprfile* mpr;

double eye[3] = { 15.0, -10.0, 15.0 };
double target[3] = { 15.0, 30.0, 5.0 };
double up[3] = { 0.0, 1.0, 0.0 };

bool mouse_left_down;
bool mouse_right_down;
bool mouse_middle_down;

float mouse_x, mouse_y;
float camera_angle_x;
float camera_angle_y;
float camera_distance;

int draw_mode;
int val;

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glTranslatef(0, camera_distance, 0);
	glRotatef(camera_angle_x, 1, 0, 0);
	glRotatef(camera_angle_y, 0, 1, 0);

	mprfile_debug_render(val, mpr);

	glPopMatrix();

	glutSwapBuffers();
}

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10.0, 150.0, -10.0, 150.0, -50.0, 1000.0);
	glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
	x = x;
	y = y;
    switch (key) {
		case 'd': // switch rendering modes (fill -> wire -> point)
		case 'D':
			draw_mode = (draw_mode + 1) % 3;
			if(draw_mode == 0) {			// fill mode
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
			} else if(draw_mode == 1) {		// wireframe mode
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
			} else {						// point mode
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
			}
			break;

		case 't':
		case 'T':
			++val;
			break;

    	case 27: // escape
			mprfile_close(mpr);
			exit(0);
			break;

		default:
			break;
    }
}

void mouse(int button, int state, int x, int y)
{
    mouse_x = x;
    mouse_y = y;

    if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			mouse_left_down = true;
		} else if (state == GLUT_UP) {
			mouse_left_down = false;
		}
	} else if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			mouse_right_down = true;
		} else if(state == GLUT_UP) {
			mouse_right_down = false;
		}
	} else if(button == GLUT_MIDDLE_BUTTON) {
		if (state == GLUT_DOWN) {
			mouse_middle_down = true;
        } else if(state == GLUT_UP) {
			mouse_middle_down = false;
		}
	}
}

void motion(int x, int y)
{
    if (mouse_left_down) {
        camera_angle_y += (x - mouse_x);
        camera_angle_x += (y - mouse_y);
        mouse_x = x;
        mouse_y = y;
    }

    if (mouse_right_down) {
        camera_distance += (y - mouse_y) * 0.2f;
        mouse_y = y;
    }
}

void idle(void)
{
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	if (3 != argc) {
		printf("Usage: %s ei_path mpr_name\n", argv[0]);
		return 1;
	}

	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutInit(&argc, argv);

	glutCreateWindow("Cursed Earth");
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClearDepth(1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye[0], eye[1], eye[2],
			target[0], target[1], target[2],
			up[0], up[1], up[2]);

	char tex_path[512];
	snprintf(tex_path, sizeof(tex_path), "%s/Res/textures.res", argv[1]);

	char mpr_path[512];
	snprintf(mpr_path, sizeof(mpr_path), "%s/Maps/%s", argv[1], argv[2]);

	resfile* tex_res = resfile_open(tex_path);
	if (NULL == tex_res) {
		printf("Could not open file '%s'\n", tex_path);
		return 1;
	}

	resfile* mpr_res = resfile_open(mpr_path);
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

	glutMainLoop();
	return 0;
}
