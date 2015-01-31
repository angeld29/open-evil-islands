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

#ifndef CE_SOUNDINSTANCE_HPP
#define CE_SOUNDINSTANCE_HPP

#include <stdbool.h>

#include "soundstate.hpp"
#include "soundbundle.hpp"
#include "soundbuffer.hpp"
#include "soundresource.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ce_sound_bundle sound_bundle;
    ce_sound_resource* sound_resource;
    ce_sound_buffer* sound_buffer;
} ce_sound_instance;

extern ce_sound_instance* ce_sound_instance_new(ce_sound_resource* sound_resource);
extern void ce_sound_instance_del(ce_sound_instance* sound_instance);

extern void ce_sound_instance_advance(ce_sound_instance* sound_instance, float elapsed);
extern void ce_sound_instance_change_state(ce_sound_instance* sound_instance, int state);

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDINSTANCE_HPP */
