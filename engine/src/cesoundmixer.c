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

#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"
#include "ceeventmanager.h"
#include "cesoundsystem.h"
#include "cesoundmixer.h"

struct ce_sound_mixer* ce_sound_mixer;

static void ce_sound_mixer_exit_react(ce_event* event)
{
	ce_unused(event);
	ce_sound_mixer->done = true;
}

static void ce_sound_mixer_add_react(ce_event* event)
{
	ce_ring_buffer* ring_buffer = ((ce_event_ptr*)event->impl)->ptr;
	ce_vector_push_back(ce_sound_mixer->ring_buffers, ring_buffer);
}

static void ce_sound_mixer_remove_react(ce_event* event)
{
	ce_ring_buffer* ring_buffer = ((ce_event_ptr*)event->impl)->ptr;
	ce_vector_remove_all(ce_sound_mixer->ring_buffers, ring_buffer);
}

static void ce_sound_mixer_exec(void* arg)
{
	ce_unused(arg);
	ce_event_manager_create_queue();

	while (!ce_sound_mixer->done) {
		char* block = ce_sound_system_map_block(ce_root.sound_system);

		size_t size = 0;

		// fill tail by silence
		memset(block + size, 0, CE_SOUND_SYSTEM_BLOCK_SIZE - size);

		ce_sound_system_unmap_block(ce_root.sound_system);

		ce_event_manager_process_events();
	}
}

void ce_sound_mixer_init(void)
{
	ce_sound_mixer = ce_alloc_zero(sizeof(struct ce_sound_mixer));
	ce_sound_mixer->ring_buffers = ce_vector_new();
	ce_sound_mixer->thread = ce_thread_new(ce_sound_mixer_exec, NULL);
}

void ce_sound_mixer_term(void)
{
	if (NULL != ce_sound_mixer) {
		ce_event_manager_post_call(ce_thread_get_id(ce_sound_mixer->thread), ce_sound_mixer_exit_react);
		ce_thread_wait(ce_sound_mixer->thread);
		ce_thread_del(ce_sound_mixer->thread);
		if (!ce_vector_empty(ce_sound_mixer->ring_buffers)) {
			ce_logging_warning("sound mixer: some ring buffers have not been removed");
		}
		ce_vector_del(ce_sound_mixer->ring_buffers);
		ce_free(ce_sound_mixer, sizeof(struct ce_sound_mixer));
	}
}

void ce_sound_mixer_add(ce_ring_buffer* ring_buffer)
{
	ce_event_manager_post_raw(ce_thread_get_id(ce_sound_mixer->thread),
		ce_sound_mixer_add_react, &(ce_event_ptr){ring_buffer}, sizeof(ce_event_ptr));
}

void ce_sound_mixer_remove(ce_ring_buffer* ring_buffer)
{
	ce_event_manager_post_raw(ce_thread_get_id(ce_sound_mixer->thread),
		ce_sound_mixer_remove_react, &(ce_event_ptr){ring_buffer}, sizeof(ce_event_ptr));
}
