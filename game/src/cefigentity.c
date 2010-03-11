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
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "celogging.h"
#include "cealloc.h"
#include "ceroot.h"
#include "cefigentity.h"

static void ce_figentity_node_del(ce_figentity_node* node)
{
	if (NULL != node) {
		if (NULL != node->child_nodes) {
			for (int i = 0, n = ce_vector_count(node->child_nodes); i < n; ++i) {
				ce_figentity_node_del(ce_vector_at(node->child_nodes, i));
			}
			ce_vector_del(node->child_nodes);
		}
		ce_renderitem_del(node->renderitem);
		ce_texture_del(node->texture);
		ce_anmstate_del(node->anmstate);
		ce_free(node, sizeof(ce_figentity_node));
	}
}

static ce_figentity_node* ce_figentity_node_new(const ce_figmesh_node* mesh_node,
												const char* texture_names[],
												ce_scenenode* parent_scenenode)
{
	ce_figentity_node* node = ce_alloc_zero(sizeof(ce_figentity_node));

	node->mesh_node = mesh_node;
	node->anmstate = ce_anmstate_new();

	// TODO: to be reversed...
	int texture_number = mesh_node->proto_node->figfile->texture_number - 1;
	if (texture_number > 1) {
		texture_number = 0;
	}

	if (NULL == (node->texture = ce_texmng_get_texture(ce_root_get_texmng(),
			texture_names[texture_number]))) {
		ce_figentity_node_del(node);
		return NULL;
	}

	if (mesh_node->proto_node->has_morphing &&
			NULL == (node->renderitem =
						ce_renderitem_clone(mesh_node->renderitem))) {
		ce_figentity_node_del(node);
		return NULL;
	}

	if (NULL == (node->scenenode =
					ce_scenenode_create_child(parent_scenenode)) ||
			NULL == (node->child_nodes = ce_vector_new())) {
		ce_figentity_node_del(node);
		return NULL;
	}

	node->scenenode->texture = node->texture;
	node->scenenode->renderitem = mesh_node->proto_node->has_morphing ?
								node->renderitem : mesh_node->renderitem;

	ce_vec3_copy(&node->scenenode->position, &mesh_node->bone);

	for (int i = 0, n = ce_vector_count(mesh_node->child_nodes); i < n; ++i) {
		ce_figentity_node* child_node = ce_figentity_node_new(
			ce_vector_at(mesh_node->child_nodes, i), texture_names, node->scenenode);
		if (NULL == child_node) {
			ce_figentity_node_del(node);
			return NULL;
		}
		ce_vector_push_back(node->child_nodes, child_node);
	}

	return node;
}

static void ce_figentity_node_advance(ce_figentity_node* node, float elapsed)
{
	if (NULL != node->anmstate->anmfile) {
		const float fps = 15.0f;
		ce_anmstate_advance(node->anmstate, fps, elapsed);

		// update bone
		ce_quat q1, q2;
		ce_quat_slerp(&node->scenenode->orientation,
			ce_quat_init_array(&q1, node->anmstate->anmfile->rotations +
									(int)node->anmstate->prev_frame * 4),
			ce_quat_init_array(&q2, node->anmstate->anmfile->rotations +
									(int)node->anmstate->next_frame * 4),
			node->anmstate->coef);

		// update morph
		if (NULL != node->renderitem) {
			ce_renderitem_update(node->renderitem,
								node->mesh_node->proto_node->figfile,
								node->anmstate);
		}
	}

	for (int i = 0; i < node->child_nodes->count; ++i) {
		ce_figentity_node_advance(node->child_nodes->items[i], elapsed);
	}
}

static bool ce_figentity_node_play_animation(ce_figentity_node* node,
												const char* name)
{
	bool ok = ce_anmstate_play_animation(node->anmstate,
										node->mesh_node->proto_node->anmfiles,
										name);
	for (int i = 0; i < node->child_nodes->count; ++i) {
		ok = ce_figentity_node_play_animation(node->child_nodes->items[i], name) && ok;
	}
	return ok;
}

ce_figentity* ce_figentity_new(ce_figmesh* figmesh,
								const ce_vec3* position,
								const ce_quat* orientation,
								const char* texture_names[],
								ce_scenenode* parent_scenenode)
{
	ce_figentity* figentity = ce_alloc_zero(sizeof(ce_figentity));

	figentity->figmesh = ce_figmesh_copy(figmesh);
	figentity->scenenode = ce_scenenode_create_child(parent_scenenode);
	figentity->root_node =
		ce_figentity_node_new(figmesh->root_node,
								texture_names,
								figentity->scenenode);

	ce_vec3_copy(&figentity->scenenode->position, position);
	ce_quat_copy(&figentity->scenenode->orientation, orientation);

	return figentity;
}

void ce_figentity_del(ce_figentity* figentity)
{
	if (NULL != figentity) {
		ce_figentity_node_del(figentity->root_node);
		ce_scenenode_del(figentity->scenenode);
		ce_figmesh_del(figentity->figmesh);
		ce_free(figentity, sizeof(ce_figentity));
	}
}

void ce_figentity_advance(ce_figentity* figentity, float elapsed)
{
	ce_figentity_node_advance(figentity->root_node, elapsed);
}

bool ce_figentity_play_animation(ce_figentity* figentity, const char* name)
{
	return ce_figentity_node_play_animation(figentity->root_node, name);
}
