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

#ifndef CE_SOUNDQUERY_H
#define CE_SOUNDQUERY_H

#include <stdbool.h>

#include "cehash.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	bool available;
	int state;
	float time;
	ce_hash_key hash_key;
} ce_sound_query;

extern ce_sound_query* ce_sound_query_new(ce_hash_key hash_key);
extern void ce_sound_query_del(ce_sound_query* sound_query);

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDQUERY_H */
