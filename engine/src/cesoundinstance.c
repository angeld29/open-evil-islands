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
	while (!soundinstance->done) {
		switch (soundinstance->state) {
		case CE_SOUNDINSTANCE_STATE_PLAYING:
			ce_pass(); // make C happy :)

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

			if (0 == size) {
				soundinstance->state = CE_SOUNDINSTANCE_STATE_STOPPED;
			}

			ce_soundsystem_unmap_block(ce_root.soundsystem);
			break;

		case CE_SOUNDINSTANCE_STATE_PAUSED:
		case CE_SOUNDINSTANCE_STATE_STOPPED:
			ce_thread_mutex_lock(soundinstance->mutex);
			ce_thread_cond_wait(soundinstance->cond, soundinstance->mutex);
			ce_thread_mutex_unlock(soundinstance->mutex);
			break;
		}
	}
}

ce_soundinstance* ce_soundinstance_new(ce_soundresource* soundresource)
{
	ce_soundinstance* soundinstance = ce_alloc_zero(sizeof(ce_soundinstance));
	soundinstance->soundresource = soundresource;
	soundinstance->mutex = ce_thread_mutex_new();
	soundinstance->cond = ce_thread_cond_new();
	soundinstance->thread = ce_thread_new(ce_soundinstance_exec, soundinstance);
	return soundinstance;
}

void ce_soundinstance_del(ce_soundinstance* soundinstance)
{
	if (NULL != soundinstance) {
		ce_thread_mutex_lock(soundinstance->mutex);
		soundinstance->done = true;
		ce_thread_mutex_unlock(soundinstance->mutex);

		ce_thread_cond_wake_all(soundinstance->cond);
		ce_thread_wait(soundinstance->thread);

		ce_thread_del(soundinstance->thread);
		ce_thread_cond_del(soundinstance->cond);
		ce_thread_mutex_del(soundinstance->mutex);

		ce_free(soundinstance, sizeof(ce_soundinstance));
	}
}

void ce_soundinstance_play(ce_soundinstance* soundinstance)
{
	soundinstance->state = CE_SOUNDINSTANCE_STATE_PLAYING;
	ce_thread_cond_wake_all(soundinstance->cond);
}
