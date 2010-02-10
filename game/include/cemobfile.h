/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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

#ifndef CE_MOBFILE_H
#define CE_MOBFILE_H

#include <stdint.h>

#include "cevector.h"
#include "cestring.h"
#include "cevec3.h"
#include "cequat.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_vector* parts;
	char owner;
	uint32_t id;
	uint32_t type;
    ce_string* name;
	ce_string* model_name;
	ce_string* parent_name;
	ce_string* primary_texture;
	ce_string* secondary_texture;
	ce_string* comment;
	ce_vec3 position;
	ce_quat rotation;
	char quest;
	char shadow;
	uint32_t parent_id;
	ce_string* quest_info;
	float strength;
	float dexterity;
	float tallness;
} ce_mobobject_object;

typedef struct {
	ce_string* script;
	ce_vector* objects;
} ce_mobfile;

extern ce_mobfile* ce_mobfile_open(const char* path);
extern void ce_mobfile_close(ce_mobfile* mob);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MOBFILE_H */
