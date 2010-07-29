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
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cesoundmixer.h"
#include "cesoundinstance.h"

ce_sound_instance* ce_sound_instance_new(ce_sound_resource* sound_resource)
{
	ce_sound_instance* sound_instance = ce_alloc_zero(sizeof(ce_sound_instance));
	sound_instance->sound_resource = sound_resource;
	sound_instance->sound_buffer = ce_sound_mixer_create_buffer();
	sound_instance->sound_buffer->sound_format = sound_resource->sound_format;
	return sound_instance;
}

void ce_sound_instance_del(ce_sound_instance* sound_instance)
{
	if (NULL != sound_instance) {
		ce_sound_mixer_destroy_buffer(sound_instance->sound_buffer);
		ce_sound_resource_del(sound_instance->sound_resource);
		ce_free(sound_instance, sizeof(ce_sound_instance));
	}
}

void ce_sound_instance_advance(ce_sound_instance* sound_instance, float CE_UNUSED(elapsed))
{
	if (CE_SOUND_STATE_PLAYING == sound_instance->state) {
		size_t size = ce_sound_buffer_available_size_for_write(sound_instance->sound_buffer);
		if (0 != size) {
			char buffer[size];
			size = ce_sound_resource_read(sound_instance->sound_resource, buffer, size);
			ce_sound_buffer_write(sound_instance->sound_buffer, buffer, size);
			sound_instance->time = sound_instance->sound_resource->time;
			if (0 == size) {
				ce_sound_instance_change_state(sound_instance, CE_SOUND_STATE_STOPPED);
			}
		}
	}
}

void ce_sound_instance_change_state(ce_sound_instance* sound_instance, int state)
{
	sound_instance->state = state;

	switch (state) {
	case CE_SOUND_STATE_STOPPED:
		sound_instance->time = 0.0f;
		ce_sound_resource_reset(sound_instance->sound_resource);
		break;
	}
}
