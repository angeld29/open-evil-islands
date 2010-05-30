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
#include "cesounddriver.h"

static void ce_sounddriver_exec(ce_sounddriver* sounddriver)
{
	size_t size = ce_ringbuffer_get_read_avail(sounddriver->ringbuffer);
	size -= size % sounddriver->sample_size;

	if (0 == size) {
		size = sounddriver->sample_size;
	}

	//ce_thread_sem_acquire(sounddriver->used_bytes, size);
	// TODO: write to device
	//ce_thread_sem_release(sounddriver->free_bytes, size);
}

ce_sounddriver* ce_sounddriver_new(ce_sounddriver_vtable vtable, size_t size, ...)
{
	ce_sounddriver* sounddriver = ce_alloc_zero(sizeof(ce_sounddriver) + size);

	sounddriver->vtable = vtable;
	sounddriver->size = size;

	va_list args;
	va_start(args, size);

	sounddriver->bps = va_arg(args, int);
	sounddriver->rate = va_arg(args, int);
	sounddriver->channels = va_arg(args, int);

	sounddriver->sample_size = sounddriver->bps * sounddriver->channels / 8;

	sounddriver->ringbuffer = ce_ringbuffer_new(4096);

	sounddriver->free_bytes = ce_thread_sem_new(sounddriver->ringbuffer->size);
	sounddriver->used_bytes = ce_thread_sem_new(0);

	sounddriver->thread = ce_thread_new(ce_sounddriver_exec, sounddriver);

	if (NULL != vtable.ctor) {
		if (!(*vtable.ctor)(sounddriver, args)) {
			ce_sounddriver_del(sounddriver);
			sounddriver = NULL;
		}
	}

	va_end(args);

	return sounddriver;
}

void ce_sounddriver_del(ce_sounddriver* sounddriver)
{
	if (NULL != sounddriver) {
		ce_thread_wait(sounddriver->thread);

		if (NULL != sounddriver->vtable.dtor) {
			(*sounddriver->vtable.dtor)(sounddriver);
		}

		ce_thread_del(sounddriver->thread);
		ce_thread_sem_del(sounddriver->used_bytes);
		ce_thread_sem_del(sounddriver->free_bytes);
		ce_ringbuffer_del(sounddriver->ringbuffer);

		ce_free(sounddriver, sizeof(ce_sounddriver) + sounddriver->size);
	}
}

ce_sounddriver* ce_sounddriver_create_null(int bps, int rate, int channels)
{
	// TODO: stub
	return NULL;
}
