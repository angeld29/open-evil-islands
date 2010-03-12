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

#include "cestr.h"
#include "cealloc.h"
#include "ceanmstate.h"

ce_anmstate* ce_anmstate_new(void)
{
	ce_anmstate* anmstate = ce_alloc(sizeof(ce_anmstate));
	anmstate->anmfile = NULL;
	return anmstate;
}

void ce_anmstate_del(ce_anmstate* anmstate)
{
	if (NULL != anmstate) {
		ce_free(anmstate, sizeof(ce_anmstate));
	}
}

void ce_anmstate_advance(ce_anmstate* anmstate, float fps, float elapsed)
{
	anmstate->frame += fps * elapsed;
	if (anmstate->frame >= anmstate->frame_count) {
		anmstate->frame = 0.0f;
	}

	anmstate->coef = modff(anmstate->frame, &anmstate->prev_frame);
	anmstate->next_frame = anmstate->prev_frame + 1.0f;
	if (anmstate->next_frame >= anmstate->frame_count) {
		anmstate->next_frame = 0.0f;
	}
}

bool ce_anmstate_play_animation(ce_anmstate* anmstate,
								ce_vector* anmfiles,
								const char* name)
{
	for (int i = 0; i < anmfiles->count; ++i) {
		ce_anmfile* anmfile = anmfiles->items[i];
		if (0 == ce_strcasecmp(name, anmfile->name->str)) {
			assert(anmfile->rotation_frame_count ==
					anmfile->translation_frame_count);
			anmstate->anmfile = anmfile;
			anmstate->frame_count = anmfile->rotation_frame_count;
			anmstate->prev_frame = 0.0f;
			anmstate->next_frame = 0.0f;
			anmstate->frame = 0.0f;
			anmstate->coef = 0.0f;
			return true;
		}
	}
	return false;
}
