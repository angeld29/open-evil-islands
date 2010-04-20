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

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include "cevec3.h"
#include "cequat.h"
#include "cebbox.h"
#include "cevector.h"
#include "cefrustum.h"
#include "cerenderitem.h"
#include "cerendersystem.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_scenenode_listener ce_scenenode_listener;

typedef struct {
	void (*ctor)(ce_scenenode_listener* listener, va_list args);
	void (*dtor)(ce_scenenode_listener* listener);
	void (*attached)(ce_scenenode_listener* listener);
	void (*detached)(ce_scenenode_listener* listener);
	void (*about_to_update)(ce_scenenode_listener* listener,
								float anmfps, float elapsed);
	void (*updated)(ce_scenenode_listener* listener);
	void (*destroyed)(ce_scenenode_listener* listener);
} ce_scenenode_listener_vtable;

struct ce_scenenode_listener {
	ce_scenenode_listener_vtable vtable;
	size_t size;
	char impl[];
};

typedef struct ce_scenenode ce_scenenode;

struct ce_scenenode {
	ce_vec3 position;
	ce_quat orientation;
	ce_vec3 world_position;
	ce_quat world_orientation;
	ce_bbox world_bbox;
	bool culled;
	ce_vector* renderitems;
	ce_scenenode_listener* listener;
	ce_scenenode* parent;
	ce_vector* childs;
};

extern ce_scenenode* ce_scenenode_new(ce_scenenode* parent);
extern void ce_scenenode_del(ce_scenenode* scenenode);

extern void ce_scenenode_create_listener(ce_scenenode* scenenode,
	ce_scenenode_listener_vtable vtable, size_t size, ...);

extern void ce_scenenode_detach_from_parent(ce_scenenode* scenenode);
extern void ce_scenenode_detach_child(ce_scenenode* scenenode,
										ce_scenenode* child);

extern void ce_scenenode_add_renderitem(ce_scenenode* scenenode,
										ce_renderitem* renderitem);

extern void ce_scenenode_cull_cascade(ce_scenenode* scenenode,
										const ce_frustum* frustum);

extern int ce_scenenode_count_visible_cascade(ce_scenenode* scenenode);

extern void ce_scenenode_update(ce_scenenode* scenenode,
								float anmfps, float elapsed, bool force);
extern void ce_scenenode_update_cascade(ce_scenenode* scenenode,
										float anmfps, float elapsed, bool force);

extern void ce_scenenode_draw_bboxes(ce_scenenode* scenenode,
									ce_rendersystem* rendersystem,
									bool comprehensive_only);
extern void ce_scenenode_draw_bboxes_cascade(ce_scenenode* scenenode,
											ce_rendersystem* rendersystem,
											bool comprehensive_only);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SCENENODE_H */
