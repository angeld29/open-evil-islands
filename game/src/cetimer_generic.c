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

#include <time.h>

#include "cealloc.h"
#include "cetimer.h"

struct ce_timer {
	clock_t elapsed_old;
	clock_t elapsed_new;
	float elapsed_diff;
};

ce_timer* ce_timer_new(void)
{
	ce_timer* timer = ce_alloc(sizeof(ce_timer));
	timer->elapsed_old = clock();
	return timer;
}

void ce_timer_del(ce_timer* timer)
{
	ce_free(timer, sizeof(ce_timer));
}

void ce_timer_advance(ce_timer* timer)
{
	timer->elapsed_new = clock();
	timer->elapsed_diff = (timer->elapsed_new -
							timer->elapsed_old) / (float)CLOCKS_PER_SEC;
	timer->elapsed_old = timer->elapsed_new;
}

float ce_timer_elapsed(ce_timer* timer)
{
	return timer->elapsed_diff;
}
