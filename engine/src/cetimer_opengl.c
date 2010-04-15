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

#include <GL/glut.h>

#include "cealloc.h"
#include "cetimer.h"

static const float CE_TIMER_THOUSAND_INV = 1.0f / 1000.0f;

struct ce_timer {
	int start;
	int stop;
	float diff;
};

ce_timer* ce_timer_new(void)
{
	ce_timer* timer = ce_alloc(sizeof(ce_timer));
	timer->start = glutGet(GLUT_ELAPSED_TIME);
	return timer;
}

void ce_timer_del(ce_timer* timer)
{
	ce_free(timer, sizeof(ce_timer));
}

void ce_timer_advance(ce_timer* timer)
{
	timer->stop = glutGet(GLUT_ELAPSED_TIME);
	timer->diff = (timer->stop - timer->start) * CE_TIMER_THOUSAND_INV;
	timer->start = timer->stop;
}

float ce_timer_elapsed(ce_timer* timer)
{
	return timer->diff;
}
