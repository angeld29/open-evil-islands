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

#include "cealloc.h"
#include "cescenenode.h"

ce_scenenode* ce_scenenode_new(ce_scenenode* parent)
{
	ce_scenenode* scenenode = ce_alloc_zero(sizeof(ce_scenenode));
	scenenode->position = CE_VEC3_ZERO;
	scenenode->orientation = CE_QUAT_IDENTITY;
	scenenode->renderlayers = ce_vector_new();
	scenenode->parent = parent;
	scenenode->childs = ce_vector_new();
	if (NULL != parent) {
		ce_vector_push_back(parent->childs, scenenode);
	}
	return scenenode;
}

void ce_scenenode_del(ce_scenenode* scenenode)
{
	if (NULL != scenenode) {
		if (NULL != scenenode->parent) {
			ce_scenenode_detach_child(scenenode->parent, scenenode);
		}
		for (int i = 0; i < scenenode->childs->count; ++i) {
			ce_scenenode* child = scenenode->childs->items[i];
			child->parent = NULL;
			ce_scenenode_del(child);
		}
		ce_vector_del(scenenode->childs);
		ce_vector_for_each(scenenode->renderlayers,
							(ce_vector_func1)ce_renderlayer_del);
		ce_vector_del(scenenode->renderlayers);
		ce_free(scenenode, sizeof(ce_scenenode));
	}
}

void ce_scenenode_detach_child(ce_scenenode* scenenode, ce_scenenode* child)
{
	int index = ce_vector_find(scenenode->childs, child);
	if (-1 != index) {
		child->parent = NULL;
		ce_vector_remove(scenenode->childs, index);
	}
}

void ce_scenenode_add_renderlayer(ce_scenenode* scenenode,
									ce_renderlayer* renderlayer)
{
	ce_vector_push_back(scenenode->renderlayers, renderlayer);
}

static void ce_scenenode_update_transform(ce_scenenode* scenenode)
{
	if (NULL == scenenode->parent) {
		// absolute state == relative state
		scenenode->world_position = scenenode->position;
		scenenode->world_orientation = scenenode->orientation;
	} else {
		ce_vec3_rot(&scenenode->world_position,
					&scenenode->position,
					&scenenode->parent->world_orientation);
		ce_vec3_add(&scenenode->world_position,
					&scenenode->world_position,
					&scenenode->parent->world_position);
		ce_quat_mul(&scenenode->world_orientation,
					&scenenode->parent->world_orientation,
					&scenenode->orientation);
	}
}

static void ce_scenenode_update_bounds(ce_scenenode* scenenode)
{
	ce_bbox_clear(&scenenode->world_bbox);

	for (int i = 0; i < scenenode->renderlayers->count; ++i) {
		ce_renderlayer* renderlayer = scenenode->renderlayers->items[i];
		for (int j = 0; j < renderlayer->renderitems->count; ++j) {
			ce_renderitem* renderitem = renderlayer->renderitems->items[j];
			ce_bbox_merge(&scenenode->world_bbox, &renderitem->bbox);
		}
	}

	// FIXME: wrong if correct bbox merging will be implemented
	scenenode->world_bbox.axis = scenenode->world_orientation;

	ce_vec3_rot(&scenenode->world_bbox.aabb.origin,
				&scenenode->world_bbox.aabb.origin,
				&scenenode->world_orientation);
	ce_vec3_add(&scenenode->world_bbox.aabb.origin,
				&scenenode->world_bbox.aabb.origin,
				&scenenode->world_position);

	for (int i = 0; i < scenenode->childs->count; ++i) {
		ce_scenenode* child = scenenode->childs->items[i];
		ce_bbox_merge(&scenenode->world_bbox, &child->world_bbox);
	}
}

void ce_scenenode_update(ce_scenenode* scenenode)
{
	ce_scenenode_update_transform(scenenode);
	ce_scenenode_update_bounds(scenenode);
}

void ce_scenenode_update_cascade(ce_scenenode* scenenode)
{
	ce_scenenode_update_transform(scenenode);
	for (int i = 0; i < scenenode->childs->count; ++i) {
		ce_scenenode_update_cascade(scenenode->childs->items[i]);
	}
	ce_scenenode_update_bounds(scenenode);
}

void ce_scenenode_render(ce_scenenode* scenenode,
						ce_rendersystem* rendersystem)
{
	ce_rendersystem_apply_transform(rendersystem,
									&scenenode->world_position,
									&scenenode->world_orientation,
									&CE_VEC3_UNIT_SCALE);
	for (int i = 0; i < scenenode->renderlayers->count; ++i) {
		ce_renderlayer* renderlayer = scenenode->renderlayers->items[i];
		ce_rendersystem_apply_material(rendersystem, renderlayer->material);
		for (int j = 0; j < renderlayer->renderitems->count; ++j) {
			ce_renderitem* renderitem = renderlayer->renderitems->items[j];
			ce_rendersystem_apply_transform(rendersystem,
											&renderitem->position,
											&renderitem->orientation,
											&CE_VEC3_UNIT_SCALE);
			ce_renderitem_render(renderitem);
			ce_rendersystem_discard_transform(rendersystem);
		}
		ce_rendersystem_discard_material(rendersystem, renderlayer->material);
	}
	ce_rendersystem_discard_transform(rendersystem);
}

static void ce_scenenode_draw_bbox(ce_rendersystem* rendersystem,
									const ce_bbox* bbox)
{
	ce_rendersystem_apply_transform(rendersystem,
		&bbox->aabb.origin, &bbox->axis, &bbox->aabb.extents);
	ce_rendersystem_draw_wire_cube(rendersystem, 1.0f, &CE_COLOR_BLUE);
	ce_rendersystem_discard_transform(rendersystem);
}

void ce_scenenode_draw_bboxes(ce_scenenode* scenenode,
							ce_rendersystem* rendersystem,
							bool comprehensive_only)
{
	ce_scenenode_draw_bbox(rendersystem, &scenenode->world_bbox);

	if (!comprehensive_only) {
		ce_rendersystem_apply_transform(rendersystem,
										&scenenode->world_position,
										&scenenode->world_orientation,
										&CE_VEC3_UNIT_SCALE);
		for (int i = 0; i < scenenode->renderlayers->count; ++i) {
			ce_renderlayer* renderlayer = scenenode->renderlayers->items[i];
			for (int j = 0; j < renderlayer->renderitems->count; ++j) {
				ce_renderitem* renderitem = renderlayer->renderitems->items[j];
				ce_scenenode_draw_bbox(rendersystem, &renderitem->bbox);
			}
		}
		ce_rendersystem_discard_transform(rendersystem);
	}
}
