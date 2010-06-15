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

bool ce_sound_helper_is_stopped(ce_sound_id sound_id)
{
	ce_soundinstance* soundinstance = ce_soundmanager_find(ce_root.soundmanager, sound_id);
	if (NULL != soundinstance) {
		return ce_soundinstance_is_stopped(soundinstance);
	}
	return true;
}

void ce_sound_helper_play(ce_sound_id sound_id)
{
	ce_soundinstance* soundinstance = ce_soundmanager_find(ce_root.soundmanager, sound_id);
	if (NULL != soundinstance) {
		ce_soundinstance_play(soundinstance);
	}
}

void ce_sound_helper_pause(ce_sound_id sound_id)
{
	ce_soundinstance* soundinstance = ce_soundmanager_find(ce_root.soundmanager, sound_id);
	if (NULL != soundinstance) {
		ce_soundinstance_pause(soundinstance);
	}
}

void ce_sound_helper_stop(ce_sound_id sound_id)
{
	ce_soundinstance* soundinstance = ce_soundmanager_find(ce_root.soundmanager, sound_id);
	if (NULL != soundinstance) {
		ce_soundinstance_stop(soundinstance);
	}
}

float ce_sound_helper_time(ce_sound_id sound_id)
{
	ce_soundinstance* soundinstance = ce_soundmanager_find(ce_root.soundmanager, sound_id);
	return NULL != soundinstance ? ce_soundinstance_time(soundinstance) : 0.0f;
}
