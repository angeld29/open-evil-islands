/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>

#include <sys/time.h>
#include <pthread.h>

#include "cealloc.h"
#include "cetimer.h"

typedef struct {
	struct timeval start;
	struct timeval stop;
} ce_timer_posix;

static void ce_timer_get_time_of_day(struct timeval* tv)
{
	// FIXME: not POSIX stuff...

	//cpu_set_t old_cpuset, cpuset;
	//pthread_t thread = pthread_self();

	//CPU_ZERO(&cpuset);
	//CPU_SET(0, &cpuset);

	//pthread_getaffinity_np(thread, sizeof(cpu_set_t), &old_cpuset);
	//pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

	gettimeofday(tv, NULL);

	//pthread_setaffinity_np(thread, sizeof(cpu_set_t), &old_cpuset);
}

ce_timer* ce_timer_new(void)
{
	return ce_alloc(sizeof(ce_timer) + sizeof(ce_timer_posix));
}

void ce_timer_del(ce_timer* timer)
{
	ce_free(timer, sizeof(ce_timer) + sizeof(ce_timer_posix));
}

void ce_timer_start(ce_timer* timer)
{
	ce_timer_posix* posix_timer = (ce_timer_posix*)timer->impl;
	ce_timer_get_time_of_day(&posix_timer->start);
}

float ce_timer_advance(ce_timer* timer)
{
	ce_timer_posix* posix_timer = (ce_timer_posix*)timer->impl;

	ce_timer_get_time_of_day(&posix_timer->stop);

	struct timeval diff = {
		.tv_sec = posix_timer->stop.tv_sec - posix_timer->start.tv_sec,
		.tv_usec = posix_timer->stop.tv_usec - posix_timer->start.tv_usec,
	};

	if (diff.tv_usec < 0) {
		--diff.tv_sec;
		diff.tv_usec += 1000000;
	}

	timer->elapsed = diff.tv_sec + diff.tv_usec * 1e-6f;
	posix_timer->start = posix_timer->stop;

	return timer->elapsed;
}
