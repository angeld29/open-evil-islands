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

#include "cevec3.h"
#include "cequat.h"
#include "cefigmesh.h"
#include "cescenenode.h"
#include "cetexture.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	const ce_figmesh_node* mesh_node;
	ce_texture* texture;
	ce_renderitem* renderitem;
	ce_scenenode* scenenode;
	ce_vector* child_nodes;
	int anm_index;
	float anm_frame;
} ce_figentity_node;

typedef struct {
	ce_figmesh* figmesh;
	ce_scenenode* scenenode;
	ce_figentity_node* root_node;
} ce_figentity;

extern ce_figentity* ce_figentity_new(ce_figmesh* figmesh,
										const ce_vec3* position,
										const ce_quat* orientation,
										const char* texture_names[],
										ce_scenenode* parent_scenenode);
extern void ce_figentity_del(ce_figentity* figentity);

extern void ce_figentity_advance(ce_figentity* figentity, float elapsed);

extern bool ce_figentity_play_animation(ce_figentity* figentity,
											const char* anm_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FIGENTITY_H */
