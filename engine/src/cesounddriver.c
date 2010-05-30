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
#include "cesounddriver.h"

static void ce_sounddriver_exec(ce_sounddriver* sounddriver)
{
	for (size_t i = 0; !sounddriver->done; ++i) {
		ce_thread_sem_acquire(sounddriver->used_blocks, 1);

		if (sounddriver->done) {
			break;
		}

		(*sounddriver->vtable.write)(sounddriver,
			sounddriver->blocks->items[i % sounddriver->blocks->count]);

		ce_thread_sem_release(sounddriver->free_blocks, 1);
	}
}

ce_sounddriver* ce_sounddriver_new(ce_sounddriver_vtable vtable, ...)
{
	ce_sounddriver* sounddriver = ce_alloc_zero(sizeof(ce_sounddriver) + vtable.size);
	sounddriver->vtable = vtable;

	va_list args;
	va_start(args, vtable);

	sounddriver->bps = va_arg(args, int);
	sounddriver->rate = va_arg(args, int);
	sounddriver->channels = va_arg(args, int);

	sounddriver->sample_size = sounddriver->bps * sounddriver->channels / 8;
	sounddriver->block_size = 1024 * sounddriver->sample_size;

	// reserve 4 blocks (usually ~16 kb total)
	sounddriver->blocks = ce_vector_new_reserved(4);
	ce_vector_resize(sounddriver->blocks, sounddriver->blocks->capacity);

	for (int i = 0; i < sounddriver->blocks->count; ++i) {
		sounddriver->blocks->items[i] = ce_alloc(sounddriver->block_size);
	}

	sounddriver->free_blocks = ce_thread_sem_new(sounddriver->blocks->count);
	sounddriver->used_blocks = ce_thread_sem_new(0);

	sounddriver->thread = ce_thread_new(ce_sounddriver_exec, sounddriver);

	if (!(*vtable.ctor)(sounddriver, args)) {
		ce_sounddriver_del(sounddriver);
		sounddriver = NULL;
	}

	va_end(args);

	return sounddriver;
}

void ce_sounddriver_del(ce_sounddriver* sounddriver)
{
	if (NULL != sounddriver) {
		sounddriver->done = true;

		ce_thread_sem_release(sounddriver->used_blocks, 1);
		ce_thread_wait(sounddriver->thread);

		if (NULL != sounddriver->vtable.dtor) {
			(*sounddriver->vtable.dtor)(sounddriver);
		}

		ce_thread_del(sounddriver->thread);
		ce_thread_sem_del(sounddriver->used_blocks);
		ce_thread_sem_del(sounddriver->free_blocks);

		for (int i = 0; i < sounddriver->blocks->count; ++i) {
			ce_free(sounddriver->blocks->items[i], sounddriver->block_size);
		}

		ce_vector_del(sounddriver->blocks);
		ce_free(sounddriver, sizeof(ce_sounddriver) + sounddriver->vtable.size);
	}
}

void* ce_sounddriver_map_block(ce_sounddriver* sounddriver)
{
	ce_thread_sem_acquire(sounddriver->free_blocks, 1);
	return sounddriver->blocks->items[sounddriver->block_index++ %
										sounddriver->blocks->count];
}

void ce_sounddriver_unmap_block(ce_sounddriver* sounddriver)
{
	ce_thread_sem_release(sounddriver->used_blocks, 1);
}

ce_sounddriver* ce_sounddriver_create_null(int bps, int rate, int channels)
{
	// TODO: stub
	(void)(bps);
	(void)(rate);
	(void)(channels);
	return NULL;
}
