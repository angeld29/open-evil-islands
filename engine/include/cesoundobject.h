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

#ifndef CE_SOUNDOBJECT_H
#define CE_SOUNDOBJECT_H

#include <stdbool.h>

#include "cehash.h"
#include "cesoundstate.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef ce_hash_key ce_sound_object;

extern ce_sound_object ce_sound_object_new(const char* name);
extern void ce_sound_object_del(ce_sound_object sound_object);

extern bool ce_sound_object_is_valid(ce_sound_object sound_object);

extern void ce_sound_object_stop(ce_sound_object sound_object);
extern void ce_sound_object_pause(ce_sound_object sound_object);
extern void ce_sound_object_play(ce_sound_object sound_object);

extern int ce_sound_object_get_state(ce_sound_object sound_object);
extern float ce_sound_object_get_time(ce_sound_object sound_object);

static inline bool ce_sound_object_is_stopped(ce_sound_object sound_object)
{
    return CE_SOUND_STATE_STOPPED == ce_sound_object_get_state(sound_object);
}

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDOBJECT_H */
