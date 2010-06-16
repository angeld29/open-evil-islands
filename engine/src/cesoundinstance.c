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
#include <limits.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"
#include "cesoundinstance.h"

static void ce_sound_instance_exec(ce_sound_instance* sound_instance)
{
	ce_mutex_lock(sound_instance->mutex);

	while (!sound_instance->done) {
		switch (sound_instance->state) {
		case CE_SOUND_INSTANCE_STATE_PLAYING:
			ce_mutex_unlock(sound_instance->mutex);

			char* block = ce_sound_system_map_block(ce_root.sound_system);
			size_t size = 0;

			// the resource may not return requested size in one pass
			for (size_t bytes = SIZE_MAX; 0 != bytes &&
					size < CE_SOUND_SYSTEM_BLOCK_SIZE; size += bytes) {
				bytes = ce_sound_resource_read(sound_instance->sound_resource,
					block + size, CE_SOUND_SYSTEM_BLOCK_SIZE - size);
			}

			// fill tail by silence
			memset(block + size, 0, CE_SOUND_SYSTEM_BLOCK_SIZE - size);

			ce_sound_system_unmap_block(ce_root.sound_system);

			ce_mutex_lock(sound_instance->mutex);
			sound_instance->time = sound_instance->sound_resource->time;

			if (0 == size) {
				sound_instance->state = CE_SOUND_INSTANCE_STATE_STOPPED;
			}
			break;

		case CE_SOUND_INSTANCE_STATE_STOPPED:
			sound_instance->time = 0.0f;
			ce_sound_resource_reset(sound_instance->sound_resource);
			ce_waitcond_wake_all(sound_instance->waitcond);

		case CE_SOUND_INSTANCE_STATE_PAUSED:
			ce_waitcond_wait(sound_instance->waitcond, sound_instance->mutex);
			break;
		}
	}

	ce_mutex_unlock(sound_instance->mutex);
}

ce_sound_instance* ce_sound_instance_new(ce_sound_object sound_object,
										ce_sound_resource* sound_resource)
{
	ce_sound_instance* sound_instance = ce_alloc_zero(sizeof(ce_sound_instance));
	sound_instance->sound_object = sound_object;
	sound_instance->sound_resource = sound_resource;
	sound_instance->mutex = ce_mutex_new();
	sound_instance->waitcond = ce_waitcond_new();
	sound_instance->thread = ce_thread_new(ce_sound_instance_exec, sound_instance);
	return sound_instance;
}

void ce_sound_instance_del(ce_sound_instance* sound_instance)
{
	if (NULL != sound_instance) {
		ce_mutex_lock(sound_instance->mutex);
		sound_instance->done = true;
		ce_mutex_unlock(sound_instance->mutex);

		ce_waitcond_wake_all(sound_instance->waitcond);
		ce_thread_wait(sound_instance->thread);

		ce_thread_del(sound_instance->thread);
		ce_waitcond_del(sound_instance->waitcond);
		ce_mutex_del(sound_instance->mutex);

		ce_sound_resource_del(sound_instance->sound_resource);

		ce_free(sound_instance, sizeof(ce_sound_instance));
	}
}

float ce_sound_instance_time(ce_sound_instance* sound_instance)
{
	ce_mutex_lock(sound_instance->mutex);
	float time = sound_instance->time;
	ce_mutex_unlock(sound_instance->mutex);
	return time;
}

void ce_sound_instance_play(ce_sound_instance* sound_instance)
{
	ce_mutex_lock(sound_instance->mutex);
	sound_instance->state = CE_SOUND_INSTANCE_STATE_PLAYING;
	ce_waitcond_wake_all(sound_instance->waitcond);
	ce_mutex_unlock(sound_instance->mutex);
}

void ce_sound_instance_pause(ce_sound_instance* sound_instance)
{
	ce_mutex_lock(sound_instance->mutex);
	sound_instance->state = CE_SOUND_INSTANCE_STATE_PAUSED;
	ce_waitcond_wake_all(sound_instance->waitcond);
	ce_mutex_unlock(sound_instance->mutex);
}

void ce_sound_instance_stop(ce_sound_instance* sound_instance)
{
	ce_mutex_lock(sound_instance->mutex);
	sound_instance->state = CE_SOUND_INSTANCE_STATE_STOPPED;
	ce_waitcond_wake_all(sound_instance->waitcond);
	// TODO: make async
	ce_waitcond_wait(sound_instance->waitcond, sound_instance->mutex);
	ce_mutex_unlock(sound_instance->mutex);
}
