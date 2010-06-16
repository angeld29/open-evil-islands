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

#include "ceroot.h"
#include "cesoundhelper.h"

float ce_sound_helper_time(ce_sound_id sound_id)
{
	ce_sound_instance* sound_instance = ce_sound_manager_find(ce_root.sound_manager, sound_id);
	return NULL != sound_instance ? ce_sound_instance_time(sound_instance) : 0.0f;
}

bool ce_sound_helper_is_stopped(ce_sound_id sound_id)
{
	ce_sound_instance* sound_instance = ce_sound_manager_find(ce_root.sound_manager, sound_id);
	if (NULL != sound_instance) {
		return ce_sound_instance_is_stopped(sound_instance);
	}
	return true;
}

void ce_sound_helper_play(ce_sound_id sound_id)
{
	ce_sound_instance* sound_instance = ce_sound_manager_find(ce_root.sound_manager, sound_id);
	if (NULL != sound_instance) {
		ce_sound_instance_play(sound_instance);
	}
}

void ce_sound_helper_pause(ce_sound_id sound_id)
{
	ce_sound_instance* sound_instance = ce_sound_manager_find(ce_root.sound_manager, sound_id);
	if (NULL != sound_instance) {
		ce_sound_instance_pause(sound_instance);
	}
}

void ce_sound_helper_stop(ce_sound_id sound_id)
{
	ce_sound_instance* sound_instance = ce_sound_manager_find(ce_root.sound_manager, sound_id);
	if (NULL != sound_instance) {
		ce_sound_instance_stop(sound_instance);
	}
}
