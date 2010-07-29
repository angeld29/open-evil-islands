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

#ifndef CE_SOUNDMANAGER_H
#define CE_SOUNDMANAGER_H

#include "cevector.h"
#include "cehash.h"
#include "cethread.h"
#include "cetimer.h"
#include "cesoundinstance.h"
#include "cesoundquery.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct ce_sound_manager {
	ce_hash_key last_hash_key;
	ce_vector* res_files;
	ce_hash* sound_instances;
	ce_hash* sound_queries;
	ce_timer* timer;
	ce_thread* thread;
}* ce_sound_manager;

extern void ce_sound_manager_init(void);
extern void ce_sound_manager_term(void);

extern void ce_sound_manager_advance(float elapsed);

extern ce_hash_key ce_sound_manager_create_instance(const char* name);
extern void ce_sound_manager_remove_instance(ce_hash_key hash_key);
extern void ce_sound_manager_change_instance_state(ce_hash_key hash_key, int state);

static inline ce_sound_query* ce_sound_manager_get_query(ce_hash_key hash_key)
{
	return NULL;
	return ce_hash_find(ce_sound_manager->sound_queries, hash_key);
}

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDMANAGER_H */
