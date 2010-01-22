#include <GL/glut.h>

#include "memory.h"
#include "timer.h"

struct timer {
	int elapsed_old;
	int elapsed_new;
	float elapsed_diff;
};

timer* timer_open(void)
{
	timer* tmr = memory_alloc(sizeof(timer));
	if (NULL == tmr) {
		return NULL;
	}
	tmr->elapsed_old = glutGet(GLUT_ELAPSED_TIME);
	return tmr;
}

void timer_close(timer* tmr)
{
	memory_free(tmr, sizeof(timer));
}

void timer_advance(timer* tmr)
{
	tmr->elapsed_new = glutGet(GLUT_ELAPSED_TIME);
	tmr->elapsed_diff = (tmr->elapsed_new - tmr->elapsed_old) / 1000.0f;
	tmr->elapsed_old = tmr->elapsed_new;
}

float timer_elapsed(timer* tmr)
{
	return tmr->elapsed_diff;
}
