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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cesoundresource.h"

extern const size_t CE_SOUND_RESOURCE_BUILTIN_COUNT;
extern const ce_sound_resource_vtable ce_sound_resource_builtins[];

ce_sound_resource* ce_sound_resource_new(ce_mem_file* mem_file)
{
    ce_sound_probe sound_probe;

    size_t index;
    for (index = 0; index < CE_SOUND_RESOURCE_BUILTIN_COUNT; ++index) {
        ce_mem_file_rewind(mem_file);

        memset(&sound_probe, 0, sizeof(ce_sound_probe));
        sound_probe.mem_file = mem_file;
        sound_probe.name = "unknown";

        if ((*ce_sound_resource_builtins[index].test)(&sound_probe)) {
            break;
        }
    }

    if (CE_SOUND_RESOURCE_BUILTIN_COUNT == index) {
        return NULL;
    }

    size_t size = sound_probe.impl_size + sound_probe.input_buffer_capacity +
                                            sound_probe.output_buffer_capacity;

    ce_sound_resource* sound_resource = ce_alloc_zero(sizeof(ce_sound_resource) + size);

    sound_resource->mem_file = mem_file;
    sound_resource->input_buffer_capacity = sound_probe.input_buffer_capacity;
    sound_resource->output_buffer_capacity = sound_probe.output_buffer_capacity;
    sound_resource->input_buffer = sound_resource->impl + sound_probe.impl_size;
    sound_resource->output_buffer = sound_resource->input_buffer + sound_probe.input_buffer_capacity;
    sound_resource->vtable = ce_sound_resource_builtins[index];
    sound_resource->size = size;

    if (!(*sound_resource->vtable.ctor)(sound_resource, &sound_probe)) {
        // do not take ownership if failed
        sound_resource->mem_file = NULL;
        ce_sound_resource_del(sound_resource);
        return NULL;
    }

    sound_resource->bytes_per_second_inv = 1.0f / sound_resource->sound_format.bytes_per_second;

    ce_logging_debug("sound resource: %s, audio is %u bits per sample, %u Hz, %u channel(s)",
        sound_probe.name,
        sound_resource->sound_format.bits_per_sample,
        sound_resource->sound_format.samples_per_second,
        sound_resource->sound_format.channel_count);

    return sound_resource;
}

void ce_sound_resource_del(ce_sound_resource* sound_resource)
{
    if (NULL != sound_resource) {
        if (NULL != sound_resource->vtable.dtor) {
            (*sound_resource->vtable.dtor)(sound_resource);
        }
        ce_mem_file_del(sound_resource->mem_file);
        ce_free(sound_resource, sizeof(ce_sound_resource) + sound_resource->size);
    }
}

size_t ce_sound_resource_read(ce_sound_resource* sound_resource, void* data, size_t size)
{
    if (0 == sound_resource->output_buffer_size) {
        sound_resource->output_buffer_pos = 0;

        (*sound_resource->vtable.decode)(sound_resource);
    }

    size = ce_min(size_t, size, sound_resource->output_buffer_size);
    memcpy(data, sound_resource->output_buffer +
                sound_resource->output_buffer_pos, size);

    sound_resource->output_buffer_pos += size;
    sound_resource->output_buffer_size -= size;

    sound_resource->granule_pos += size;
    sound_resource->time = sound_resource->granule_pos *
                            sound_resource->bytes_per_second_inv;

    return size;
}

bool ce_sound_resource_reset(ce_sound_resource* sound_resource)
{
    sound_resource->time = 0.0f;
    sound_resource->granule_pos = 0;
    sound_resource->output_buffer_size = 0;
    sound_resource->output_buffer_pos = 0;

    ce_mem_file_rewind(sound_resource->mem_file);

    return (*sound_resource->vtable.reset)(sound_resource);
}
