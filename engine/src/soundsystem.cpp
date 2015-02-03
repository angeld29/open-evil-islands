/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include "alloc.hpp"
#include "logging.hpp"
#include "event.hpp"
#include "optionmanager.hpp"
#include "soundsystem.hpp"

struct ce_sound_system* ce_sound_system;

static bool ce_sound_system_null_ctor(void)
{
    ce_logging_write("sound system: using null output");
    return true;
}

static bool ce_sound_system_null_write(const void* /*block*/)
{
    return true;
}

ce_sound_system_vtable ce_sound_system_null(void)
{
    return (ce_sound_system_vtable){
        0, ce_sound_system_null_ctor, NULL, ce_sound_system_null_write
    };
}

static void ce_sound_system_exit(ce_event*)
{
    ce_sound_system->done = true;
}

static void ce_sound_system_exec(void*)
{
    for (size_t i = 0; !ce_sound_system->done; ++i) {
        ce_semaphore_acquire(ce_sound_system->used_blocks, 1);

        if (!(*ce_sound_system->vtable.write)(ce_sound_system->blocks[i % CE_SOUND_SYSTEM_BLOCK_COUNT])) {
            ce_logging_critical("sound system: could not write block");
        }

        ce_semaphore_release(ce_sound_system->free_blocks, 1);
        ce_event_manager_process_events(ce_thread_self(), CE_EVENT_FLAG_ALL_EVENTS);
    }
}

static void ce_sound_system_dtor(void)
{
    if (NULL != ce_sound_system) {
        if (NULL != ce_sound_system->vtable.dtor) {
            (*ce_sound_system->vtable.dtor)();
        }

        ce_free(ce_sound_system, sizeof(struct ce_sound_system) + ce_sound_system->vtable.size);
    }
}

static bool ce_sound_system_ctor(ce_sound_system_vtable vtable)
{
    ce_sound_system = ce_alloc_zero(sizeof(struct ce_sound_system) + vtable.size);

    ce_sound_system->samples_per_second = CE_SOUND_SYSTEM_SAMPLES_PER_SECOND;
    ce_sound_system->vtable = vtable;

    if (!(*vtable.ctor)()) {
        ce_sound_system_dtor();
        return false;
    }

    return true;
}

void ce_sound_system_init(void)
{
    if (!ce_sound_system_ctor(ce_option_manager->disable_sound ?
            ce_sound_system_null() : ce_sound_system_platform())) {
        ce_sound_system_ctor(ce_sound_system_null());
    }

    ce_sound_system->free_blocks = ce_semaphore_new(CE_SOUND_SYSTEM_BLOCK_COUNT);
    ce_sound_system->used_blocks = ce_semaphore_new(0);

    ce_sound_system->thread = ce_thread_new(ce_sound_system_exec, NULL);

    ce_sound_format_init(&ce_sound_system->sound_format,
                        CE_SOUND_SYSTEM_BITS_PER_SAMPLE,
                        ce_sound_system->samples_per_second,
                        CE_SOUND_SYSTEM_CHANNEL_COUNT);

    if (CE_SOUND_SYSTEM_SAMPLES_PER_SECOND != ce_sound_system->samples_per_second) {
        ce_logging_warning("sound system: sample rate %u Hz not supported "
                            "by the implementation/hardware, using %u Hz",
            CE_SOUND_SYSTEM_SAMPLES_PER_SECOND, ce_sound_system->samples_per_second);
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

        ce_sound_system_dtor();
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
