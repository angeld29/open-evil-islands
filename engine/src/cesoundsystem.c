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

#include "cealloc.h"
#include "celogging.h"
#include "ceoptionmanager.h"
#include "ceeventmanager.h"
#include "cesoundsystem.h"

struct ce_sound_system* ce_sound_system;

static bool ce_sound_system_null_ctor(void)
{
	ce_logging_write("sound system: using null output");
	return true;
}

static bool ce_sound_system_null_write(const void* CE_UNUSED(block))
{
	return true;
}

ce_sound_system_vtable ce_sound_system_null(void)
{
	return (ce_sound_system_vtable){
		0, ce_sound_system_null_ctor, NULL, ce_sound_system_null_write
	};
}

static void ce_sound_system_exit(ce_event* CE_UNUSED(event))
{
	ce_sound_system->done = true;
}

static void ce_sound_system_exec(void* CE_UNUSED(arg))
{
	ce_event_manager_create_queue();

	for (size_t i = 0; !ce_sound_system->done; ++i) {
		ce_semaphore_acquire(ce_sound_system->used_blocks, 1);

		if (!(*ce_sound_system->vtable.write)(ce_sound_system->blocks[i % CE_SOUND_SYSTEM_BLOCK_COUNT])) {
			ce_logging_critical("sound system: could not write block");
		}

		ce_semaphore_release(ce_sound_system->free_blocks, 1);

		ce_event_manager_process_events();
	}
}

static void ce_sound_system_clean(void)
{
	if (NULL != ce_sound_system) {
		if (NULL != ce_sound_system->vtable.dtor) {
			(*ce_sound_system->vtable.dtor)();
		}

		ce_free(ce_sound_system, sizeof(struct ce_sound_system) + ce_sound_system->vtable.size);
	}
}

static bool ce_sound_system_alloc(ce_sound_system_vtable vtable)
{
	ce_sound_system = ce_alloc_zero(sizeof(struct ce_sound_system) + vtable.size);

	ce_sound_system->sample_rate = CE_SOUND_SYSTEM_SAMPLE_RATE;
	ce_sound_system->vtable = vtable;

	if (!(*vtable.ctor)()) {
		ce_sound_system_clean();
		return false;
	}

	return true;
}

void ce_sound_system_init(void)
{
	if (!ce_sound_system_alloc(ce_option_manager->disable_sound ?
			ce_sound_system_null() : ce_sound_system_platform())) {
		ce_sound_system_alloc(ce_sound_system_null());
	}

	ce_sound_system->free_blocks = ce_semaphore_new(CE_SOUND_SYSTEM_BLOCK_COUNT);
	ce_sound_system->used_blocks = ce_semaphore_new(0);

	ce_sound_system->thread = ce_thread_new(ce_sound_system_exec, NULL);

	if (CE_SOUND_SYSTEM_SAMPLE_RATE != ce_sound_system->sample_rate) {
		ce_logging_warning("sound system: sample rate %u Hz not supported "
							"by the implementation/hardware, using %u Hz",
							CE_SOUND_SYSTEM_SAMPLE_RATE, ce_sound_system->sample_rate);
	}
}

void ce_sound_system_term(void)
{
	if (NULL != ce_sound_system) {
		ce_event_manager_post_call(ce_sound_system->thread->id, ce_sound_system_exit);

		ce_semaphore_release(ce_sound_system->used_blocks, 1);
		ce_thread_wait(ce_sound_system->thread);

		ce_thread_del(ce_sound_system->thread);
		ce_semaphore_del(ce_sound_system->used_blocks);
		ce_semaphore_del(ce_sound_system->free_blocks);

		ce_sound_system_clean();
	}
}

void* ce_sound_system_map_block(void)
{
	ce_semaphore_acquire(ce_sound_system->free_blocks, 1);
	return ce_sound_system->blocks[ce_sound_system->next_block++ % CE_SOUND_SYSTEM_BLOCK_COUNT];
}

void ce_sound_system_unmap_block(void)
{
	ce_semaphore_release(ce_sound_system->used_blocks, 1);
}
