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

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"
#include "cesoundinstance.h"

static void ce_soundinstance_exec(ce_soundinstance* soundinstance)
{
	ce_mutex_lock(soundinstance->mutex);

	while (!soundinstance->done) {
		switch (soundinstance->state) {
		case CE_SOUNDINSTANCE_STATE_PLAYING:
			ce_waitcond_wake_all(soundinstance->waitcond);
			ce_mutex_unlock(soundinstance->mutex);

			char* block = ce_soundsystem_map_block(ce_root.soundsystem);
			size_t size = 0;

			// some resources do not return requested size in one pass
			for (size_t bytes = SIZE_MAX; 0 != bytes &&
					size < CE_SOUNDSYSTEM_BLOCK_SIZE; size += bytes) {
				bytes = ce_soundresource_read(soundinstance->soundresource,
					block + size, CE_SOUNDSYSTEM_BLOCK_SIZE - size);
			}

			// fill tail by silence
			memset(block + size, 0, CE_SOUNDSYSTEM_BLOCK_SIZE - size);

			ce_soundsystem_unmap_block(ce_root.soundsystem);

			if (0 == size) {
				ce_mutex_lock(soundinstance->mutex);
				soundinstance->state = CE_SOUNDINSTANCE_STATE_STOPPED;
				ce_mutex_unlock(soundinstance->mutex);
			}

			ce_mutex_lock(soundinstance->mutex);
			break;

		case CE_SOUNDINSTANCE_STATE_PAUSED:
		case CE_SOUNDINSTANCE_STATE_STOPPED:
			ce_waitcond_wake_all(soundinstance->waitcond);
			ce_waitcond_wait(soundinstance->waitcond, soundinstance->mutex);
			ce_soundresource_reset(soundinstance->soundresource);
			break;
		}
	}

	ce_mutex_unlock(soundinstance->mutex);
}

ce_soundinstance* ce_soundinstance_new(ce_soundresource* soundresource)
{
	ce_soundinstance* soundinstance = ce_alloc_zero(sizeof(ce_soundinstance));
	soundinstance->soundresource = soundresource;
	soundinstance->mutex = ce_mutex_new();
	soundinstance->waitcond = ce_waitcond_new();
	soundinstance->thread = ce_thread_new(ce_soundinstance_exec, soundinstance);
	return soundinstance;
}

void ce_soundinstance_del(ce_soundinstance* soundinstance)
{
	if (NULL != soundinstance) {
		ce_mutex_lock(soundinstance->mutex);
		soundinstance->done = true;
		ce_mutex_unlock(soundinstance->mutex);

		ce_waitcond_wake_all(soundinstance->waitcond);
		ce_thread_wait(soundinstance->thread);

		ce_thread_del(soundinstance->thread);
		ce_waitcond_del(soundinstance->waitcond);
		ce_mutex_del(soundinstance->mutex);

		ce_soundresource_del(soundinstance->soundresource);

		ce_free(soundinstance, sizeof(ce_soundinstance));
	}
}

void ce_soundinstance_play(ce_soundinstance* soundinstance)
{
	ce_mutex_lock(soundinstance->mutex);
	soundinstance->state = CE_SOUNDINSTANCE_STATE_PLAYING;
	ce_waitcond_wake_all(soundinstance->waitcond);
	// TODO: make async!!!
	ce_waitcond_wait(soundinstance->waitcond, soundinstance->mutex);
	ce_mutex_unlock(soundinstance->mutex);
}

void ce_soundinstance_stop(ce_soundinstance* soundinstance)
{
	ce_mutex_lock(soundinstance->mutex);
	soundinstance->state = CE_SOUNDINSTANCE_STATE_STOPPED;
	ce_waitcond_wake_all(soundinstance->waitcond);
	// TODO: make async!!!
	ce_waitcond_wait(soundinstance->waitcond, soundinstance->mutex);
	ce_mutex_unlock(soundinstance->mutex);
}
