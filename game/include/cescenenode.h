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

#ifndef CE_SCENENODE_H
#define CE_SCENENODE_H

#include "cevec3.h"
#include "cequat.h"
#include "ceaabb.h"
#include "cesphere.h"
#include "cevector.h"
#include "cerenderitem.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_scenenode ce_scenenode;

struct ce_scenenode {
	ce_vec3 position;
	ce_quat orientation;
	ce_vec3 world_position;
	ce_quat world_orientation;
	ce_aabb world_bounding_box;
	ce_sphere world_bounding_sphere;
	float dist2;
	ce_renderitem* renderitem;
	ce_scenenode* parent_scenenode;
	ce_vector* child_scenenodes;
};

extern ce_scenenode* ce_scenenode_new(void);
extern void ce_scenenode_del(ce_scenenode* scenenode);

extern ce_scenenode* ce_scenenode_create_child(ce_scenenode* scenenode);
extern void ce_scenenode_add_child(ce_scenenode* scenenode,
									ce_scenenode* child_scenenode);
extern void ce_scenenode_remove_child(ce_scenenode* scenenode,
										ce_scenenode* child_scenenode);

extern void ce_scenenode_update_cascade(ce_scenenode* scenenode);

extern void ce_scenenode_apply_transform(ce_scenenode* scenenode);
extern void ce_scenenode_discard_transform(ce_scenenode* scenenode);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SCENENODE_H */
