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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cebyteorder.h"
#include "cesoundinstance.h"

static void ce_soundinstance_exec(ce_soundinstance* soundinstance)
{
	while (!soundinstance->done) {
		switch (soundinstance->state) {
			case CE_SOUNDINSTANCE_STATE_PLAYING:
			ce_pass(); // make C happy :)

			char* block = ce_sounddriver_map_block(soundinstance->sounddriver);
			size_t size = 0;

			// some decoders do not return requested size in one pass
			for (size_t bytes = SIZE_MAX; 0 != bytes &&
					size < soundinstance->sounddriver->block_size; size += bytes) {
				bytes = (*soundinstance->vtable.read)(soundinstance,
					block + size, soundinstance->sounddriver->block_size - size);
			}

			// fill tail by silence
			memset(block + size, 0, soundinstance->sounddriver->block_size - size);

			if (0 == size) {
				soundinstance->state = CE_SOUNDINSTANCE_STATE_STOPPED;
			}

			ce_sounddriver_unmap_block(soundinstance->sounddriver);
			break;

		case CE_SOUNDINSTANCE_STATE_PAUSED:
		case CE_SOUNDINSTANCE_STATE_STOPPED:
			ce_thread_cond_wait(soundinstance->cond, soundinstance->mutex);
			break;
		}
	}
}

ce_soundinstance* ce_soundinstance_new(ce_soundinstance_vtable vtable, ...)
{
	ce_soundinstance* soundinstance = ce_alloc_zero(sizeof(ce_soundinstance) + vtable.size);
	soundinstance->vtable = vtable;

	va_list args;
	va_start(args, vtable);

	if (!(*vtable.ctor)(soundinstance, args)) {
		ce_soundinstance_del(soundinstance);
		soundinstance = NULL;
	}

	va_end(args);

	if (NULL != soundinstance) {
		soundinstance->sounddriver = ce_sounddriver_create_platform(
			soundinstance->bps, soundinstance->rate, soundinstance->channels);

		soundinstance->mutex = ce_thread_mutex_new();
		soundinstance->cond = ce_thread_cond_new();
		soundinstance->thread = ce_thread_new(ce_soundinstance_exec, soundinstance);
	}

	return soundinstance;
}

void ce_soundinstance_del(ce_soundinstance* soundinstance)
{
	if (NULL != soundinstance) {
		soundinstance->done = true;

		ce_thread_cond_wake_all(soundinstance->cond);
		ce_thread_wait(soundinstance->thread);

		if (NULL != soundinstance->vtable.dtor) {
			(*soundinstance->vtable.dtor)(soundinstance);
		}

		ce_thread_del(soundinstance->thread);
		ce_thread_cond_del(soundinstance->cond);
		ce_thread_mutex_del(soundinstance->mutex);
		ce_sounddriver_del(soundinstance->sounddriver);

		ce_free(soundinstance, sizeof(ce_soundinstance) + soundinstance->vtable.size);
	}
}

void ce_soundinstance_play(ce_soundinstance* soundinstance)
{
	soundinstance->state = CE_SOUNDINSTANCE_STATE_PLAYING;
	ce_thread_cond_wake_all(soundinstance->cond);
}

extern const size_t CE_SOUNDINSTANCE_DECODER_VTABLE_COUNT;
extern ce_soundinstance_vtable ce_soundinstance_decoder_vtables[];

ce_soundinstance* ce_soundinstance_create_path(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL == file) {
		ce_logging_error("soundinstance: could not open file '%s'", path);
		return NULL;
	}

	// TODO: loop and test
	ce_soundinstance* soundinstance = ce_soundinstance_new(ce_soundinstance_decoder_vtables[0], file);
	if (NULL == soundinstance) {
		fclose(file);
	}

	return soundinstance;
}
