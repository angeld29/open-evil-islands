#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <GL/glut.h>

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE);

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(400, 300);
	glutCreateWindow("");

	printf("vendor: %s\n", glGetString(GL_VENDOR));
	printf("renderer: %s\n", glGetString(GL_RENDERER));
	printf("version: %s\n", glGetString(GL_VERSION));

	const char* extensions = glGetString(GL_EXTENSIONS);
	int length = strlen(extensions);
	char buffer[length + 1];

	strcpy(buffer, extensions);

	for (int i = 0; i < length; ++i) {
		if (' ' == buffer[i]) {
			buffer[i] = '\n';
		}
	}

	printf("extensions:\n%s\n", buffer);

	return EXIT_SUCCESS;
}
