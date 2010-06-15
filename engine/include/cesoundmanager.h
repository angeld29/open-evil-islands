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
#include "cestring.h"
#include "cesoundinstance.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ce_sound_id last_sound_id;
	ce_vector* soundinstances;
} ce_soundmanager;

extern ce_soundmanager* ce_soundmanager_new(void);
extern void ce_soundmanager_del(ce_soundmanager* soundmanager);

extern void ce_soundmanager_advance(ce_soundmanager* soundmanager, float elapsed);

extern ce_sound_id ce_soundmanager_create(ce_soundmanager* soundmanager, const char* name);
extern ce_soundinstance* ce_soundmanager_find(ce_soundmanager* soundmanager, ce_sound_id sound_id);

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDMANAGER_H */
