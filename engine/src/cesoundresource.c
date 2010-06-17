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

ce_sound_resource* ce_sound_resource_new(ce_sound_resource_vtable vtable, ce_memfile* memfile)
{
	size_t size = (*vtable.size_hint)(memfile);
	ce_memfile_rewind(memfile);

	ce_sound_resource* sound_resource = ce_alloc_zero(sizeof(ce_sound_resource) + size);

	sound_resource->memfile = memfile;
	sound_resource->vtable = vtable;
	sound_resource->size = size;

	if (!(*vtable.ctor)(sound_resource)) {
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

ce_sound_resource* ce_sound_resource_new_builtin(ce_memfile* memfile)
{
	size_t index = ce_sound_resource_find_builtin(memfile);
	if (CE_SOUND_RESOURCE_BUILTIN_COUNT != index) {
		return ce_sound_resource_new(ce_sound_resource_builtins[index], memfile);
	}
	return NULL;
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

size_t ce_sound_resource_find_builtin(ce_memfile* memfile)
{
	for (size_t i = 0; i < CE_SOUND_RESOURCE_BUILTIN_COUNT; ++i) {
		ce_memfile_rewind(memfile);
		if ((*ce_sound_resource_builtins[i].test)(memfile)) {
			ce_memfile_rewind(memfile);
			return i;
		}
	}
	ce_memfile_rewind(memfile);
	return CE_SOUND_RESOURCE_BUILTIN_COUNT;
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
