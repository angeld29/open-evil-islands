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

#ifndef CE_FIGENTITY_H
#define CE_FIGENTITY_H

#include <stdbool.h>

#include "cevec3.h"
#include "cequat.h"
#include "cevector.h"
#include "cefigmesh.h"
#include "cefigbone.h"
#include "cetexture.h"
#include "cescenenode.h"
#include "cerendergroup.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_figmesh* figmesh;
	ce_figbone* figbone;
	ce_vector* textures;
	ce_scenenode* scenenode;
} ce_figentity;

extern ce_figentity* ce_figentity_new(ce_figmesh* figmesh,
										const ce_vec3* position,
										const ce_quat* orientation,
										int texture_count,
										ce_texture* textures[],
										ce_scenenode* scenenode);
extern void ce_figentity_del(ce_figentity* figentity);

extern void ce_figentity_advance(ce_figentity* figentity,
									float fps, float elapsed);

extern void ce_figentity_update(ce_figentity* figentity, bool force);

extern void ce_figentity_enqueue(ce_figentity* figentity,
								ce_rendergroup* rendergroup);

extern int ce_figentity_get_animation_count(ce_figentity* figentity);
extern const char*
ce_figentity_get_animation_name(ce_figentity* figentity, int index);

extern bool
ce_figentity_play_animation(ce_figentity* figentity, const char* name);
extern void ce_figentity_stop_animation(ce_figentity* figentity);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FIGENTITY_H */
