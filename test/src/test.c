#include <stdlib.h>
#include <stdio.h>

#include <GL/glut.h>

#include "mmpfile.h"

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
	display();
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

int main(int argc, char* argv[])
{
	if (2 != argc) {
		printf("Usage: %s <mmp>\n", argv[0]);
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

	GLuint texid = mmpfile_gentex_file(argv[1]);
	if (0 == texid) {
		printf("Could not load texture '%s'\n", argv[1]);
		return 1;
	}

	glClearColor(1.0, 1.0, 1.0, 1.0);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texid);
	glEnable(GL_CULL_FACE);
	glColor3f(1.0f, 1.0f, 1.0f);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutMainLoop();
	return 0;
}
