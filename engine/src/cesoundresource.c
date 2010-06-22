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

#include <stdio.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cesoundresource.h"

extern const size_t CE_SOUND_RESOURCE_BUILTIN_COUNT;
extern const ce_sound_resource_vtable ce_sound_resource_builtins[];

ce_sound_resource* ce_sound_resource_new(ce_memfile* memfile)
{
	ce_sound_probe sound_probe = {.memfile = memfile};

	size_t index;
	for (index = 0; index < CE_SOUND_RESOURCE_BUILTIN_COUNT; ++index) {
		ce_memfile_rewind(memfile);
		if ((*ce_sound_resource_builtins[index].test)(&sound_probe)) {
			break;
		}
	}

	if (CE_SOUND_RESOURCE_BUILTIN_COUNT == index) {
		return NULL;
	}

	ce_sound_resource* sound_resource = ce_alloc_zero(sizeof(ce_sound_resource) + sound_probe.size);

	sound_resource->memfile = memfile;
	sound_resource->vtable = ce_sound_resource_builtins[index];
	sound_resource->size = sound_probe.size;

	if (!(*sound_resource->vtable.ctor)(sound_resource, &sound_probe)) {
		// do not take ownership if failed
		sound_resource->memfile = NULL;
		ce_sound_resource_del(sound_resource);
		return NULL;
	}

	sound_resource->sample_size = sound_resource->channel_count *
								(sound_resource->bits_per_sample / 8);
	sound_resource->bytes_per_sec_inv = 1.0f / (sound_resource->sample_rate *
												sound_resource->sample_size);

	return sound_resource;
}

void ce_sound_resource_del(ce_sound_resource* sound_resource)
{
	if (NULL != sound_resource) {
		if (NULL != sound_resource->vtable.dtor) {
			(*sound_resource->vtable.dtor)(sound_resource);
		}
		ce_memfile_close(sound_resource->memfile);
		ce_free(sound_resource, sizeof(ce_sound_resource) + sound_resource->size);
	}
}

size_t ce_sound_resource_read(ce_sound_resource* sound_resource, void* data, size_t size)
{
	size = (*sound_resource->vtable.read)(sound_resource, data, size);
	sound_resource->granule_pos += size;
	sound_resource->time = sound_resource->granule_pos *
							sound_resource->bytes_per_sec_inv;
	return size;
}

bool ce_sound_resource_reset(ce_sound_resource* sound_resource)
{
	sound_resource->time = 0.0f;
	sound_resource->granule_pos = 0;
	ce_memfile_rewind(sound_resource->memfile);
	return (*sound_resource->vtable.reset)(sound_resource);
}
