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

#include <assert.h>

#include "cesoundmanager.h"
#include "cesoundobject.h"

ce_sound_object ce_sound_object_new(const char* name)
{
    return ce_sound_manager_create_object(name);
}

void ce_sound_object_del(ce_sound_object sound_object)
{
    if (0 != sound_object) {
        ce_sound_manager_remove_object(sound_object);
    }
}

bool ce_sound_object_is_valid(ce_sound_object sound_object)
{
    ce_sound_bundle* sound_bundle = ce_sound_manager_get_bundle(sound_object);
    return NULL != sound_bundle;
}

void ce_sound_object_stop(ce_sound_object sound_object)
{
    ce_sound_manager_state_object(sound_object, CE_SOUND_STATE_STOPPED);
}

void ce_sound_object_pause(ce_sound_object sound_object)
{
    ce_sound_manager_state_object(sound_object, CE_SOUND_STATE_PAUSED);
}

void ce_sound_object_play(ce_sound_object sound_object)
{
    ce_sound_manager_state_object(sound_object, CE_SOUND_STATE_PLAYING);
}

int ce_sound_object_get_state(ce_sound_object sound_object)
{
    ce_sound_bundle* sound_bundle = ce_sound_manager_get_bundle(sound_object);
    return NULL != sound_bundle ? sound_bundle->state : CE_SOUND_STATE_STOPPED;
}

float ce_sound_object_get_time(ce_sound_object sound_object)
{
    ce_sound_bundle* sound_bundle = ce_sound_manager_get_bundle(sound_object);
    return NULL != sound_bundle ? sound_bundle->time : 0.0f;
}
