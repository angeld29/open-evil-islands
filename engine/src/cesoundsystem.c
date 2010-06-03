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

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cesoundsystem.h"

static void ce_soundsystem_exec(ce_soundsystem* soundsystem)
{
	for (size_t i = 0; !soundsystem->done; ++i) {
		ce_thread_sem_acquire(soundsystem->used_blocks, 1);

		if (!(*soundsystem->vtable.write)(soundsystem,
				soundsystem->blocks[i % CE_SOUNDSYSTEM_BLOCK_COUNT])) {
			ce_logging_critical("soundsystem: could not write block");
		}

		ce_thread_sem_release(soundsystem->free_blocks, 1);
	}
}

ce_soundsystem* ce_soundsystem_new(ce_soundsystem_vtable vtable)
{
	ce_soundsystem* soundsystem = ce_alloc_zero(sizeof(ce_soundsystem) + vtable.size);
	soundsystem->vtable = vtable;

	soundsystem->free_blocks = ce_thread_sem_new(CE_SOUNDSYSTEM_BLOCK_COUNT);
	soundsystem->used_blocks = ce_thread_sem_new(0);

	soundsystem->thread = ce_thread_new(ce_soundsystem_exec, soundsystem);

	if (!(*vtable.ctor)(soundsystem)) {
		ce_soundsystem_del(soundsystem);
		return NULL;
	}

	return soundsystem;
}

static bool ce_soundsystem_null_ctor(ce_soundsystem* soundsystem)
{
	ce_unused(soundsystem);
	ce_logging_write("soundsystem: using null output");
	return true;
}

static bool ce_soundsystem_null_write(ce_soundsystem* soundsystem, const void* block)
{
	ce_unused(soundsystem), ce_unused(block);
	return true;
}

ce_soundsystem* ce_soundsystem_new_null(void)
{
	return ce_soundsystem_new((ce_soundsystem_vtable){0,
		ce_soundsystem_null_ctor, NULL, ce_soundsystem_null_write});
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

		ce_free(soundsystem, sizeof(ce_soundsystem) + soundsystem->vtable.size);
	}
}

void* ce_soundsystem_map_block(ce_soundsystem* soundsystem)
{
	ce_thread_sem_acquire(soundsystem->free_blocks, 1);
	return soundsystem->blocks[soundsystem->next_block++ %
								CE_SOUNDSYSTEM_BLOCK_COUNT];
}

void ce_soundsystem_unmap_block(ce_soundsystem* soundsystem)
{
	ce_thread_sem_release(soundsystem->used_blocks, 1);
}
