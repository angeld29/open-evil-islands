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

#ifndef CE_SOUNDMANAGER_H
#define CE_SOUNDMANAGER_H

#include "cevector.h"
#include "cehash.h"
#include "cethread.h"
#include "cetimer.h"
#include "cesoundinstance.h"
#include "cesoundbundle.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct ce_sound_manager {
    ce_thread_id thread_id;
    ce_hash_key last_hash_key;
    ce_vector* res_files;
    ce_hash* sound_instances;
    ce_hash* sound_bundles;
    ce_timer* timer;
    ce_thread* thread;
}* ce_sound_manager;

extern void ce_sound_manager_init(void);
extern void ce_sound_manager_term(void);

extern void ce_sound_manager_advance(float elapsed);

extern ce_hash_key ce_sound_manager_create_object(const char* name);
extern void ce_sound_manager_remove_object(ce_hash_key hash_key);

extern void ce_sound_manager_state_object(ce_hash_key hash_key, int state);

static inline ce_sound_bundle* ce_sound_manager_get_bundle(ce_hash_key hash_key)
{
    return ce_hash_find(ce_sound_manager->sound_bundles, hash_key);
}

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDMANAGER_H */
