#include <stdlib.h>
#include <stdio.h>

#include <GL/glut.h>

#include "resfile.h"
#include "mmpfile.h"

resfile* res;

void display(void)
{
	const float w = 2.0f, h = 2.0f;

	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -10.0f);

	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(w, -h);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(w, h);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(-w, h);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(-w, -h);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(-w, -h);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(-w, h);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(w, h);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(w, -h);
	glEnd();

	glutSwapBuffers();
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, (GLdouble)width / height, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
	x = x;
	y = y;
	switch (key) {
	case 27: // escape
		exit(0);
		break;
	}
}

void timer(int index)
{
	if (0 == resfile_node_count(res)) {
		return;
	}

	if (index >= resfile_node_count(res)) {
		index = 0;
	}

	memfile* mem = resfile_node_memfile(index, res);
	if (NULL != mem) {
		GLuint texid = mmpfile_gentex(mem);
		if (0 != texid) {
			glBindTexture(GL_TEXTURE_2D, texid);
		} else {
			printf("Could not load texture '%s'\n", resfile_node_name(index, res));
		}
	} else {
		printf("Could not open texture '%s'\n", resfile_node_name(index, res));
	}

	glutTimerFunc(1000, timer, index + 1);
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	if (2 != argc) {
		printf("Usage: %s <res>\n", argv[0]);
		return 1;
	}

	res = resfile_open_file(argv[1]);
	if (NULL == res) {
		printf("Could not open file '%s'\n", argv[1]);
		return 1;
	}

	glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA);

	glutInitWindowPosition(300, 500);
	glutInitWindowSize(400, 300);
	glutInit(&argc, argv);

	glutCreateWindow("test");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(1000, timer, 0);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glColor3f(1.0f, 1.0f, 1.0f);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutMainLoop();
	return 0;
}
