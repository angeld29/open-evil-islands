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

#ifndef CE_SOUNDINSTANCE_H
#define CE_SOUNDINSTANCE_H

#include <stdbool.h>

#include "cesoundbuffer.h"
#include "cesoundobject.h"
#include "cesoundresource.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	CE_SOUND_INSTANCE_STATE_STOPPED,
	CE_SOUND_INSTANCE_STATE_PAUSED,
	CE_SOUND_INSTANCE_STATE_PLAYING,
};

typedef struct {
	int state;
	float time; // playing time in seconds
	ce_sound_object sound_object;
	ce_sound_resource* sound_resource;
	ce_sound_buffer* sound_buffer;
} ce_sound_instance;

extern ce_sound_instance* ce_sound_instance_new(ce_sound_object sound_object,
											ce_sound_resource* sound_resource);
extern void ce_sound_instance_del(ce_sound_instance* sound_instance);

extern void ce_sound_instance_advance(ce_sound_instance* sound_instance, float elapsed);

static inline bool ce_sound_instance_is_stopped(ce_sound_instance* sound_instance)
{
	return CE_SOUND_INSTANCE_STATE_STOPPED == sound_instance->state;
}

extern void ce_sound_instance_play(ce_sound_instance* sound_instance);
extern void ce_sound_instance_pause(ce_sound_instance* sound_instance);
extern void ce_sound_instance_stop(ce_sound_instance* sound_instance);

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDINSTANCE_H */
