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
	struct timeval elapsed_old;
	struct timeval elapsed_new;
	struct timeval elapsed_sub;
	float elapsed_diff;
};

ce_timer* ce_timer_open(void)
{
	ce_timer* tmr = ce_alloc(sizeof(ce_timer));
	if (NULL == tmr) {
		return NULL;
	}
	gettimeofday(&tmr->elapsed_old, NULL);
	return tmr;
}

void ce_timer_close(ce_timer* tmr)
{
	ce_free(tmr, sizeof(ce_timer));
}

void ce_timer_advance(ce_timer* tmr)
{
	gettimeofday(&tmr->elapsed_new, NULL);
	timersub(&tmr->elapsed_new, &tmr->elapsed_old, &tmr->elapsed_sub);
	tmr->elapsed_diff = tmr->elapsed_sub.tv_sec +
						tmr->elapsed_sub.tv_usec * 1e-6f;
	tmr->elapsed_old = tmr->elapsed_new;
}

float ce_timer_elapsed(ce_timer* tmr)
{
	return tmr->elapsed_diff;
}
