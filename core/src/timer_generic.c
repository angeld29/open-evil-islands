#include <assert.h>

#include <GL/glut.h>

#include "timer.h"

static int elapsed_old;
static int elapsed_new;
static int elapsed_diff;
static float elapsed;

void timer_start(void)
{
	elapsed_old = glutGet(GLUT_ELAPSED_TIME);
}

void timer_advance(void)
{
	elapsed_new = glutGet(GLUT_ELAPSED_TIME);
	elapsed_diff = elapsed_new - elapsed_old;
	elapsed_old = elapsed_new;
	elapsed = elapsed_diff / 1000.0f;
}

float timer_elapsed(void)
{
	return elapsed;
}
