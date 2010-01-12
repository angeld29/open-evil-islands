#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <getopt.h>
#include <GL/glut.h>

#include "cegl.h"
#include "logging.h"
#include "timer.h"
#include "input.h"
#include "memfile.h"
#include "resfile.h"
#include "mmpfile.h"

resfile* res;
GLuint texid;

bool rnd = false;
int delay = 500;
bool slideshow = true;

static void idle(void)
{
	timer_advance();

	float elapsed = timer_elapsed();

	input_advance(elapsed);

	if (input_test(KB_ESCAPE)) {
		resfile_close(res);
		exit(0);
	}
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
	glEnd();

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
	void* data = malloc(resfile_node_size(index, res));
	if (NULL == data || !resfile_node_data(index, data, res)) {
		free(data);
		return false;
	}
	texid = mmpfile_generate_texture(texid, data);
	free(data);
	if (0 == texid) {
		return false;
	}
	return true;
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

	if (!generate_texture(index)) {
		printf("Could not load texture '%s'\n", resfile_node_name(index, res));
	} else {
		glutPostRedisplay();
	}

	if (slideshow) {
		glutTimerFunc(delay, next_texture, rnd ?
			rand() % resfile_node_count(res) : index + 1);
	}
}

static void usage(const char* name)
{
	printf("Usage: %s [options] <resfile>\n"
			"Options:\n"
			"-r Random texture mode\n"
			"-d <delay, msec> Slideshow delay (default %d)\n"
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
		case 'h':
		case '?':
			usage(argv[0]);
			return 1;
		}
	}

	if (optind >= argc) {
		usage(argv[0]);
		return 1;
	}

	glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE);

	glutInitWindowPosition(300, 500);
	glutInitWindowSize(400, 300);
	glutInit(&argc, argv);

	glutCreateWindow("Cursed Earth Spikes: Texture Viewer");
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	logging_open();
	logging_set_level(LOGGING_DEBUG_LEVEL);
	atexit(logging_close);

	input_open();
	atexit(input_close);

	timer_start();

	gl_init();

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	glutMainLoop();
	return 0;
}
