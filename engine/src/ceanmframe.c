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

#include <math.h>
#include <assert.h>

#include "ceanmframe.h"

void ce_anmframe_reset(ce_anmframe* anmframe)
{
	anmframe->prev_frame = 0.0f;
	anmframe->next_frame = 0.0f;
	anmframe->frame = 0.0f;
	anmframe->coef = 0.0f;
}

void ce_anmframe_advance(ce_anmframe* anmframe, float distance)
{
	anmframe->frame += distance;
	anmframe->coef = modff(anmframe->frame, &anmframe->prev_frame);
	anmframe->next_frame = anmframe->prev_frame + 1.0f;
}
