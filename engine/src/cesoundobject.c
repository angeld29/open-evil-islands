/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cealloc.h"
#include "cesoundmanager.h"
#include "cesoundobject.h"

ce_sound_object* ce_sound_object_new(const char* name)
{
	ce_sound_object* sound_object = ce_alloc_zero(sizeof(ce_sound_object));
	sound_object->hash_key = ce_sound_manager_create_instance(name);
	return sound_object;
}

void ce_sound_object_del(ce_sound_object* sound_object)
{
	if (NULL != sound_object) {
		ce_sound_manager_remove_instance(sound_object->hash_key);
		ce_free(sound_object, sizeof(ce_sound_object));
	}
}

void ce_sound_object_advance(ce_sound_object* sound_object, float CE_UNUSED(elapsed))
{
	ce_sound_query* sound_query = ce_sound_manager_get_query(sound_object->hash_key);
	if (NULL != sound_query) {
		sound_object->state = sound_query->state;
		sound_object->time = sound_query->time;
	}
}

void ce_sound_object_stop(ce_sound_object* sound_object)
{
	ce_sound_manager_change_instance_state(sound_object->hash_key, CE_SOUND_STATE_STOPPED);
}

void ce_sound_object_pause(ce_sound_object* sound_object)
{
	ce_sound_manager_change_instance_state(sound_object->hash_key, CE_SOUND_STATE_PAUSED);
}

void ce_sound_object_play(ce_sound_object* sound_object)
{
	ce_sound_manager_change_instance_state(sound_object->hash_key, CE_SOUND_STATE_PLAYING);
}
