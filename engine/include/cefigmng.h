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

#ifndef CE_FIGMNG_H
#define CE_FIGMNG_H

#include <stdbool.h>

#include "cevector.h"
#include "cefigproto.h"
#include "cefigmesh.h"
#include "cefigentity.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	void (*figproto_created)(void* listener, ce_figproto* figproto);
	void (*figmesh_created)(void* listener, ce_figmesh* figmesh);
	void* listener;
} ce_figmng_listener;

typedef struct {
	ce_vector* resfiles;
	ce_vector* figprotos;
	ce_vector* figmeshes;
	ce_vector* figentities;
	ce_vector* listeners;
} ce_figmng;

extern ce_figmng* ce_figmng_new(void);
extern void ce_figmng_del(ce_figmng* figmng);

extern bool ce_figmng_register_resource(ce_figmng* figmng, const char* path);

extern void ce_figmng_add_listener(ce_figmng* figmng,
									ce_figmng_listener* listener);

extern ce_figentity*
ce_figmng_create_figentity(ce_figmng* figmng,
							const char* name,
							const ce_complection* complection,
							const ce_vec3* position,
							const ce_quat* orientation,
							int texture_count,
							ce_texture* textures[],
							ce_scenenode* scenenode);

extern void ce_figmng_remove_figentity(ce_figmng* figmng,
										ce_figentity* figentity);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FIGMNG_H */
