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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cealloc.h"
#include "cescenenode.h"

ce_scenenode* ce_scenenode_new(ce_scenenode* parent)
{
	ce_scenenode* scenenode = ce_alloc_zero(sizeof(ce_scenenode));
	scenenode->position = CE_VEC3_ZERO;
	scenenode->orientation = CE_QUAT_IDENTITY;
	scenenode->occluder = true;
	scenenode->culled = true;
	scenenode->renderitems = ce_vector_new();
	scenenode->oqresult = 1;
	scenenode->listener = NULL;
	scenenode->parent = parent;
	scenenode->childs = ce_vector_new();
	ce_gl_gen_queries(1, &scenenode->oqid);
	memset(&scenenode->listener_vtable, '\0',
			sizeof(ce_scenenode_listener_vtable));
	if (NULL != parent) {
		ce_vector_push_back(parent->childs, scenenode);
	}
	return scenenode;
}

void ce_scenenode_del(ce_scenenode* scenenode)
{
	if (NULL != scenenode) {
		ce_scenenode_detach_from_parent(scenenode);
		for (int i = 0; i < scenenode->childs->count; ++i) {
			ce_scenenode* child = scenenode->childs->items[i];
			child->parent = NULL;
			ce_scenenode_del(child);
		}
		ce_vector_del(scenenode->childs);
		ce_gl_delete_queries(1, &scenenode->oqid);
		ce_vector_for_each(scenenode->renderitems, ce_renderitem_del);
		ce_vector_del(scenenode->renderitems);
		if (NULL != scenenode->listener_vtable.destroyed) {
			(*scenenode->listener_vtable.destroyed)(scenenode->listener);
		}
		ce_free(scenenode, sizeof(ce_scenenode));
	}
}

void ce_scenenode_detach_from_parent(ce_scenenode* scenenode)
{
	if (NULL != scenenode->parent) {
		ce_scenenode_detach_child(scenenode->parent, scenenode);
		scenenode->parent = NULL;
		if (NULL != scenenode->listener_vtable.detached) {
			(*scenenode->listener_vtable.detached)(scenenode->listener);
		}
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

void ce_scenenode_add_renderitem(ce_scenenode* scenenode,
								ce_renderitem* renderitem)
{
	ce_vector_push_back(scenenode->renderitems, renderitem);
}

int ce_scenenode_count_visible_cascade(ce_scenenode* scenenode)
{
	if (scenenode->culled) {
		return 0;
	}

	if (0 == scenenode->oqresult) {
		return 0;
	}

	int count = 1;
	for (int i = 0; i < scenenode->childs->count; ++i) {
		count += ce_scenenode_count_visible_cascade(scenenode->childs->items[i]);
	}

	return count;
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

	for (int i = 0; i < scenenode->renderitems->count; ++i) {
		ce_renderitem* renderitem = scenenode->renderitems->items[i];
		ce_vec3_rot(&renderitem->world_position,
					&renderitem->position,
					&scenenode->world_orientation);
		ce_vec3_add(&renderitem->world_position,
					&renderitem->world_position,
					&scenenode->world_position);
		ce_quat_mul(&renderitem->world_orientation,
					&scenenode->world_orientation,
					&renderitem->orientation);
	}
}

static void ce_scenenode_update_bounds(ce_scenenode* scenenode)
{
	ce_bbox_clear(&scenenode->world_bbox);

	for (int i = 0; i < scenenode->renderitems->count; ++i) {
		ce_renderitem* renderitem = scenenode->renderitems->items[i];
		renderitem->world_bbox.aabb = renderitem->bbox.aabb;

		ce_vec3_rot(&renderitem->world_bbox.aabb.origin,
					&renderitem->world_bbox.aabb.origin,
					&scenenode->world_orientation);
		ce_vec3_add(&renderitem->world_bbox.aabb.origin,
					&renderitem->world_bbox.aabb.origin,
					&scenenode->world_position);
		ce_quat_mul(&renderitem->world_bbox.axis,
					&scenenode->world_orientation,
					&renderitem->bbox.axis);

		ce_bbox_merge(&scenenode->world_bbox, &renderitem->world_bbox);
	}

	for (int i = 0; i < scenenode->childs->count; ++i) {
		ce_scenenode* child = scenenode->childs->items[i];
		ce_bbox_merge(&scenenode->world_bbox, &child->world_bbox);
	}
}

void ce_scenenode_update_cascade(ce_scenenode* scenenode,
	const ce_frustum* frustum, float anmfps, float elapsed, bool force)
{
	// try to cull scene node BEFORE update for performance reasons
	// rendering defects are possible, such as culling partially visible objects
	scenenode->culled = !(force || ce_frustum_test_bbox(frustum, &scenenode->world_bbox));

	if (!scenenode->culled) {
		if (!scenenode->occluder && ce_gl_is_query(scenenode->oqid)) {
			GLint oqavailable;
			ce_gl_get_query_object_iv(scenenode->oqid,
				CE_GL_QUERY_RESULT_AVAILABLE, &oqavailable);
			if (0 != oqavailable) {
				ce_gl_get_query_object_iv(scenenode->oqid,
					CE_GL_QUERY_RESULT, &scenenode->oqresult);
			}
		}

		if (force || scenenode->occluder || 0 != scenenode->oqresult) {
			if (NULL != scenenode->listener_vtable.about_to_update) {
				(*scenenode->listener_vtable.about_to_update)
						(scenenode->listener, anmfps, elapsed);
			}

			ce_scenenode_update_transform(scenenode);
			for (int i = 0; i < scenenode->childs->count; ++i) {
				ce_scenenode_update_cascade(scenenode->childs->items[i],
											frustum, anmfps, elapsed, force);
			}
			ce_scenenode_update_bounds(scenenode);

			if (NULL != scenenode->listener_vtable.updated) {
				(*scenenode->listener_vtable.updated)(scenenode->listener);
			}
		}
	}
}

static void ce_scenenode_draw_bbox(ce_rendersystem* rendersystem,
									const ce_bbox* bbox)
{
	ce_rendersystem_apply_transform(rendersystem,
		&bbox->aabb.origin, &bbox->axis, &bbox->aabb.extents);
	ce_rendersystem_draw_wire_cube(rendersystem);
	ce_rendersystem_discard_transform(rendersystem);
}

static void ce_scenenode_draw_bbox2(ce_rendersystem* rendersystem,
									const ce_bbox* bbox)
{
	ce_rendersystem_apply_transform(rendersystem,
		&bbox->aabb.origin, &bbox->axis, &bbox->aabb.extents);
	ce_rendersystem_draw_solid_cube(rendersystem);
	ce_rendersystem_discard_transform(rendersystem);
}

void ce_scenenode_occlude_cascade(ce_scenenode* scenenode,
										ce_rendersystem* rendersystem)
{
	if (!scenenode->culled) {
		// TODO: check it
		//ce_gl_get_query_iv(CE_GL_SAMPLES_PASSED,
		//	CE_GL_QUERY_COUNTER_BITS, &oqresult);

		if (!scenenode->occluder && !ce_vector_empty(scenenode->renderitems)) {
			ce_gl_begin_query(CE_GL_SAMPLES_PASSED, scenenode->oqid);
			ce_scenenode_draw_bbox2(rendersystem, &scenenode->world_bbox);
			ce_gl_end_query(CE_GL_SAMPLES_PASSED);
		}

		for (int i = 0; i < scenenode->childs->count; ++i) {
			ce_scenenode_occlude_cascade(scenenode->childs->items[i], rendersystem);
		}
	}
}

void ce_scenenode_draw_bboxes_cascade(ce_scenenode* scenenode,
								ce_rendersystem* rendersystem,
								bool comprehensive_only)
{
	ce_rendersystem_apply_color(rendersystem, &CE_COLOR_BLUE);

	if (!scenenode->culled) {
		ce_scenenode_draw_bbox(rendersystem, &scenenode->world_bbox);

		if (!comprehensive_only) {
			for (int i = 0; i < scenenode->renderitems->count; ++i) {
				ce_renderitem* renderitem = scenenode->renderitems->items[i];
				ce_scenenode_draw_bbox(rendersystem, &renderitem->world_bbox);
			}
		}

		for (int i = 0; i < scenenode->childs->count; ++i) {
			ce_scenenode_draw_bboxes_cascade(scenenode->childs->items[i],
											rendersystem, comprehensive_only);
		}
	}
}
