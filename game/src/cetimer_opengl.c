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

#include "celogging.h"
#include "cealloc.h"
#include "cetimer.h"

struct ce_timer {
	int elapsed_old;
	int elapsed_new;
	float elapsed_diff;
};

ce_timer* ce_timer_new(void)
{
	ce_timer* tmr = ce_alloc(sizeof(ce_timer));
	if (NULL == tmr) {
		ce_logging_error("timer: could not allocate memory");
		return NULL;
	}
	tmr->elapsed_old = glutGet(GLUT_ELAPSED_TIME);
	return tmr;
}

void ce_timer_del(ce_timer* tmr)
{
	ce_free(tmr, sizeof(ce_timer));
}

void ce_timer_advance(ce_timer* tmr)
{
	tmr->elapsed_new = glutGet(GLUT_ELAPSED_TIME);
	tmr->elapsed_diff = (tmr->elapsed_new - tmr->elapsed_old) / 1000.0f;
	tmr->elapsed_old = tmr->elapsed_new;
}

float ce_timer_elapsed(ce_timer* tmr)
{
	return tmr->elapsed_diff;
}
