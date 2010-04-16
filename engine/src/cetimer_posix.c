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

#include <sys/time.h>

#include "cealloc.h"
#include "cetimer.h"

struct ce_timer {
	struct timeval start;
	struct timeval stop;
	struct timeval sub;
	float diff;
};

ce_timer* ce_timer_new(void)
{
	ce_timer* timer = ce_alloc(sizeof(ce_timer));
	gettimeofday(&timer->start, NULL);
	return timer;
}

void ce_timer_del(ce_timer* timer)
{
	ce_free(timer, sizeof(ce_timer));
}

void ce_timer_advance(ce_timer* timer)
{
	gettimeofday(&timer->stop, NULL);
	// WARNING: BSD extension, not POSIX!
	timersub(&timer->stop, &timer->start, &timer->sub);
	timer->diff = timer->sub.tv_sec + timer->sub.tv_usec * 1e-6f;
	timer->start = timer->stop;
}

float ce_timer_elapsed(ce_timer* timer)
{
	return timer->diff;
}
