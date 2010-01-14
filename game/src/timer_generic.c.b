#include <stdlib.h>
#include <time.h>

#include "timer.h"

struct timer {
	clock_t elapsed_old;
	clock_t elapsed_new;
	float elapsed_diff;
};

timer* timer_open(void)
{
	timer* tmr = malloc(sizeof(timer));
	if (NULL == tmr) {
		return NULL;
	}
	tmr->elapsed_old = clock();
	return tmr;
}

void timer_close(timer* tmr)
{
	free(tmr);
}

void timer_advance(timer* tmr)
{
	tmr->elapsed_new = clock();
	tmr->elapsed_diff = (tmr->elapsed_new -
							tmr->elapsed_old) / (float)CLOCKS_PER_SEC;
	tmr->elapsed_old = tmr->elapsed_new;
}

float timer_elapsed(timer* tmr)
{
	return tmr->elapsed_diff;
}
