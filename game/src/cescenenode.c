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

#include "celogging.h"
#include "cealloc.h"
#include "cescenenode.h"

ce_scenenode* ce_scenenode_new(void)
{
	ce_scenenode* scenenode = ce_alloc_zero(sizeof(ce_scenenode));
	if (NULL == scenenode) {
		ce_logging_error("scenenode: could not allocate memory");
		return NULL;
	}

	if (NULL == (scenenode->child_scenenodes = ce_vector_new())) {
		ce_scenenode_del(scenenode);
		return NULL;
	}

	ce_vec3_zero(&scenenode->position);
	ce_quat_identity(&scenenode->orientation);

	return scenenode;
}

void ce_scenenode_del(ce_scenenode* scenenode)
{
	if (NULL != scenenode) {
		if (NULL != scenenode->parent_scenenode) {
			ce_scenenode_remove_child(scenenode->parent_scenenode, scenenode);
		}
		if (NULL != scenenode->child_scenenodes) {
			for (int i = 0, n = ce_vector_count(scenenode->child_scenenodes); i < n; ++i) {
				ce_scenenode* child_scenenode =
					ce_vector_at(scenenode->child_scenenodes, i);
				child_scenenode->parent_scenenode = NULL;
				ce_scenenode_del(child_scenenode);
			}
			ce_vector_del(scenenode->child_scenenodes);
		}
		ce_free(scenenode, sizeof(ce_scenenode));
	}
}

ce_scenenode* ce_scenenode_create_child(ce_scenenode* scenenode)
{
	ce_scenenode* child_scenenode = ce_scenenode_new();
	if (NULL != child_scenenode) {
		child_scenenode->parent_scenenode = scenenode;
		ce_vector_push_back(scenenode->child_scenenodes, child_scenenode);
	}
	return child_scenenode;
}

void ce_scenenode_add_child(ce_scenenode* scenenode,
							ce_scenenode* child_scenenode)
{
	child_scenenode->parent_scenenode = scenenode;
	ce_vector_push_back(scenenode->child_scenenodes, child_scenenode);
}

void ce_scenenode_remove_child(ce_scenenode* scenenode,
								ce_scenenode* child_scenenode)
{
	for (int i = 0, n = ce_vector_count(scenenode->child_scenenodes); i < n; ++i) {
		if (child_scenenode == ce_vector_at(scenenode->child_scenenodes, i)) {
			child_scenenode->parent_scenenode = NULL;
			ce_vector_remove(scenenode->child_scenenodes, i);
			break;
		}
	}
}

static void ce_scenenode_update_transform(ce_scenenode* scenenode)
{
	if (NULL == scenenode->parent_scenenode) {
		// absolute state == relative state
		ce_vec3_copy(&scenenode->world_position, &scenenode->position);
		ce_quat_copy(&scenenode->world_orientation, &scenenode->orientation);
	} else {
		ce_vec3_rot(&scenenode->world_position,
					&scenenode->position,
					&scenenode->parent_scenenode->world_orientation);
		ce_vec3_add(&scenenode->world_position,
					&scenenode->world_position,
					&scenenode->parent_scenenode->world_position);
		ce_quat_mul(&scenenode->world_orientation,
					&scenenode->orientation,
					&scenenode->parent_scenenode->world_orientation);
	}
}

static void ce_scenenode_update_bounds(ce_scenenode* scenenode)
{
	if (NULL == scenenode->renderitem) {
		ce_aabb_init_zero(&scenenode->world_bounding_box);
		ce_sphere_init_zero(&scenenode->world_bounding_sphere);
	} else {
		ce_aabb_transform(&scenenode->world_bounding_box,
							&scenenode->renderitem->bounding_box,
							&scenenode->world_position,
							&scenenode->world_orientation);
		ce_sphere_transform(&scenenode->world_bounding_sphere,
							&scenenode->renderitem->bounding_sphere,
							&scenenode->world_position,
							&scenenode->world_orientation);
	}

	for (int i = 0, n = ce_vector_count(scenenode->child_scenenodes); i < n; ++i) {
		ce_scenenode* child_scenenode =
			ce_vector_at(scenenode->child_scenenodes, i);
		ce_aabb_merge(&scenenode->world_bounding_box,
						&scenenode->world_bounding_box,
						&child_scenenode->world_bounding_box);
		ce_sphere_merge(&scenenode->world_bounding_sphere,
						&scenenode->world_bounding_sphere,
						&child_scenenode->world_bounding_sphere);
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
	for (int i = 0, n = ce_vector_count(scenenode->child_scenenodes); i < n; ++i) {
		ce_scenenode_update_cascade(ce_vector_at(scenenode->child_scenenodes, i));
	}
	ce_scenenode_update_bounds(scenenode);
}
