#include <iostream>
#include <string>

#include <boost/shared_ptr.hpp>

#include <GL/glut.h>

#include "resfile.h"
#include "mprfile.h"

boost::shared_ptr<resfile> tex_res;
boost::shared_ptr<resfile> mpr_res;
boost::shared_ptr<mprfile> mpr;

int elapsed;

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

int drawMode;

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glTranslatef(0, camera_distance, 0);
	glRotatef(camera_angle_x, 1, 0, 0);
	glRotatef(camera_angle_y, 0, 1, 0);

	glColor3f(0.0f, 0.0f, 0.5f);

	mprfile_debug_render(mpr.get());

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
    switch (key) {
		case 'd': // switch rendering modes (fill -> wire -> point)
		case 'D':
			drawMode = ++drawMode % 3;
			if(drawMode == 0) {				// fill mode
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
			} else if(drawMode == 1) {		// wireframe mode
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
			} else {						// point mode
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
			}
			break;

    	case 27: // escape
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
	static int prev_time = glutGet(GLUT_ELAPSED_TIME);
	int time = glutGet(GLUT_ELAPSED_TIME);
	elapsed = time - prev_time;
	prev_time = time;

    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	if (3 != argc) {
		std::cout << "Usage: " << argv[0] <<
			" <ei_path> <mpr_name>" << std::endl;
		return 1;
	}

	std::string ei_path(argv[1]);
	std::string tex_path(ei_path + "/Res/textures.res");
	std::string mpr_path(ei_path + "/Maps/" + argv[2]);

	tex_res = boost::shared_ptr<resfile>(
		resfile_open_file(tex_path.c_str()), resfile_close);
	if (!tex_res) {
		std::cout << "Could not open file '" << tex_path << "'" << std::endl;
		return 1;
	}

	mpr_res = boost::shared_ptr<resfile>(
		resfile_open_file(mpr_path.c_str()), resfile_close);
	if (!mpr_res) {
		std::cout << "Could not open file '" << mpr_path << "'" << std::endl;
		return 1;
	}

	mpr = boost::shared_ptr<mprfile>(mprfile_open(mpr_res.get()), mprfile_close);
	if (!mpr) {
		std::cout << "Could not open file '" << mpr_path << "'" << std::endl;
		return 1;
	}

	mprfile_debug_print(mpr.get());

	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutInit(&argc, argv);

	glutCreateWindow("CE");
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClearDepth(1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye[0], eye[1], eye[2],
			target[0], target[1], target[2],
			up[0], up[1], up[2]);

	glutMainLoop();
	return 0;
}
