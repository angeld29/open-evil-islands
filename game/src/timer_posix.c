#include <sys/time.h>

#include "memory.h"
#include "timer.h"

struct timer {
	struct timeval elapsed_old;
	struct timeval elapsed_new;
	struct timeval elapsed_sub;
	float elapsed_diff;
};

timer* timer_open(void)
{
	timer* tmr = memory_alloc(sizeof(timer));
	if (NULL == tmr) {
		return NULL;
	}
	gettimeofday(&tmr->elapsed_old, NULL);
	return tmr;
}

void timer_close(timer* tmr)
{
	memory_free(tmr, sizeof(timer));
}

void timer_advance(timer* tmr)
{
	gettimeofday(&tmr->elapsed_new, NULL);
	timersub(&tmr->elapsed_new, &tmr->elapsed_old, &tmr->elapsed_sub);
	tmr->elapsed_diff = tmr->elapsed_sub.tv_sec +
						tmr->elapsed_sub.tv_usec * 1e-6f;
	tmr->elapsed_old = tmr->elapsed_new;
}

float timer_elapsed(timer* tmr)
{
	return tmr->elapsed_diff;
}
