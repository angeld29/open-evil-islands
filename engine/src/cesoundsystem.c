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

#include <assert.h>

#include "cealloc.h"
#include "celogging.h"
#include "cesoundsystem.h"

static void ce_soundsystem_exec(ce_soundsystem* soundsystem)
{
	for (size_t i = 0; !soundsystem->done; ++i) {
		ce_thread_sem_acquire(soundsystem->used_blocks, 1);

		if (soundsystem->done) {
			break;
		}

		(*soundsystem->vtable.write)(soundsystem,
			soundsystem->blocks->items[i % soundsystem->blocks->count]);

		ce_thread_sem_release(soundsystem->free_blocks, 1);
	}
}

ce_soundsystem* ce_soundsystem_new(ce_soundsystem_vtable vtable, ...)
{
	ce_soundsystem* soundsystem = ce_alloc_zero(sizeof(ce_soundsystem) + vtable.size);
	soundsystem->vtable = vtable;

	soundsystem->bps = 16;
	soundsystem->rate = 44100;
	soundsystem->channels = 2;

	soundsystem->sample_size = soundsystem->bps * soundsystem->channels / 8;
	soundsystem->block_size = 1024 * soundsystem->sample_size;
	soundsystem->sample_count = soundsystem->block_size / soundsystem->sample_size;

	// reserve 4 blocks (usually ~16 kb total)
	soundsystem->blocks = ce_vector_new_reserved(4);
	ce_vector_resize(soundsystem->blocks, soundsystem->blocks->capacity);

	for (int i = 0; i < soundsystem->blocks->count; ++i) {
		soundsystem->blocks->items[i] = ce_alloc(soundsystem->block_size);
	}

	soundsystem->free_blocks = ce_thread_sem_new(soundsystem->blocks->count);
	soundsystem->used_blocks = ce_thread_sem_new(0);

	soundsystem->thread = ce_thread_new(ce_soundsystem_exec, soundsystem);

	va_list args;
	va_start(args, vtable);

	if (!(*vtable.ctor)(soundsystem, args)) {
		ce_soundsystem_del(soundsystem);
		soundsystem = NULL;
	}

	va_end(args);

	return soundsystem;
}

void ce_soundsystem_del(ce_soundsystem* soundsystem)
{
	if (NULL != soundsystem) {
		soundsystem->done = true;

		ce_thread_sem_release(soundsystem->used_blocks, 1);
		ce_thread_wait(soundsystem->thread);

		if (NULL != soundsystem->vtable.dtor) {
			(*soundsystem->vtable.dtor)(soundsystem);
		}

		ce_thread_del(soundsystem->thread);
		ce_thread_sem_del(soundsystem->used_blocks);
		ce_thread_sem_del(soundsystem->free_blocks);

		for (int i = 0; i < soundsystem->blocks->count; ++i) {
			ce_free(soundsystem->blocks->items[i], soundsystem->block_size);
		}

		ce_vector_del(soundsystem->blocks);
		ce_free(soundsystem, sizeof(ce_soundsystem) + soundsystem->vtable.size);
	}
}

void* ce_soundsystem_map_block(ce_soundsystem* soundsystem)
{
	ce_thread_sem_acquire(soundsystem->free_blocks, 1);
	return soundsystem->blocks->items[soundsystem->block_index++ %
										soundsystem->blocks->count];
}

void ce_soundsystem_unmap_block(ce_soundsystem* soundsystem)
{
	ce_thread_sem_release(soundsystem->used_blocks, 1);
}

ce_soundsystem* ce_soundsystem_create_null(void)
{
	// TODO: stub
	return NULL;
}
