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

#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cesoundsystem.h"

static void ce_sound_system_exec(ce_sound_system* sound_system)
{
	for (size_t i = 0; !sound_system->done; ++i) {
		ce_semaphore_acquire(sound_system->used_blocks, 1);

		if (!(*sound_system->vtable.write)(sound_system,
				sound_system->blocks[i % CE_SOUND_SYSTEM_BLOCK_COUNT])) {
			ce_logging_critical("sound system: could not write block");
		}

		ce_semaphore_release(sound_system->free_blocks, 1);
	}
}

ce_sound_system* ce_sound_system_new(ce_sound_system_vtable vtable)
{
	ce_sound_system* sound_system = ce_alloc_zero(sizeof(ce_sound_system) + vtable.size);
	sound_system->sample_rate = CE_SOUND_SYSTEM_SAMPLE_RATE;
	sound_system->vtable = vtable;

	sound_system->free_blocks = ce_semaphore_new(CE_SOUND_SYSTEM_BLOCK_COUNT);
	sound_system->used_blocks = ce_semaphore_new(0);

	sound_system->thread = ce_thread_new(ce_sound_system_exec, sound_system);

	if (!(*vtable.ctor)(sound_system)) {
		ce_sound_system_del(sound_system);
		return NULL;
	}

	if (CE_SOUND_SYSTEM_SAMPLE_RATE != sound_system->sample_rate) {
		ce_logging_warning("sound system: sample rate %u Hz not supported "
							"by the implementation/hardware, using %u Hz",
							CE_SOUND_SYSTEM_SAMPLE_RATE, sound_system->sample_rate);
	}

	return sound_system;
}

static bool ce_sound_system_null_ctor(ce_sound_system* sound_system)
{
	ce_unused(sound_system);
	ce_logging_write("sound system: using null output");
	return true;
}

static bool ce_sound_system_null_write(ce_sound_system* sound_system, const void* block)
{
	ce_unused(sound_system), ce_unused(block);
	return true;
}

ce_sound_system* ce_sound_system_new_null(void)
{
	return ce_sound_system_new((ce_sound_system_vtable){0,
		ce_sound_system_null_ctor, NULL, ce_sound_system_null_write});
}

void ce_sound_system_del(ce_sound_system* sound_system)
{
	if (NULL != sound_system) {
		sound_system->done = true;

		ce_semaphore_release(sound_system->used_blocks, 1);
		ce_thread_wait(sound_system->thread);

		if (NULL != sound_system->vtable.dtor) {
			(*sound_system->vtable.dtor)(sound_system);
		}

		ce_thread_del(sound_system->thread);
		ce_semaphore_del(sound_system->used_blocks);
		ce_semaphore_del(sound_system->free_blocks);

		ce_free(sound_system, sizeof(ce_sound_system) + sound_system->vtable.size);
	}
}

void* ce_sound_system_map_block(ce_sound_system* sound_system)
{
	ce_semaphore_acquire(sound_system->free_blocks, 1);
	return sound_system->blocks[sound_system->next_block++ %
								CE_SOUND_SYSTEM_BLOCK_COUNT];
}

void ce_sound_system_unmap_block(ce_sound_system* sound_system)
{
	ce_semaphore_release(sound_system->used_blocks, 1);
}
