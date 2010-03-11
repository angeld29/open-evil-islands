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
		ce_free(node, sizeof(ce_figentity_node));
	}
}

static ce_figentity_node* ce_figentity_node_new(ce_figmesh_node* mesh_node,
												const char* texture_names[],
												ce_scenenode* parent_scenenode)
{
	ce_figentity_node* node = ce_alloc_zero(sizeof(ce_figentity_node));
	if (NULL == node) {
		ce_logging_error("figentity node: could not allocate memory");
		return NULL;
	}

	node->mesh_node = mesh_node;
	node->anm_index = -1;

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
	if (-1 != node->anm_index) {
		ce_vector* anmfiles = node->mesh_node->proto_node->anmfiles;
		ce_anmfile* anmfile = ce_vector_at(anmfiles, node->anm_index);

		assert(anmfile->rotation_frame_count ==
				anmfile->translation_frame_count);

		const float anm_fps = 15.0f;
		const float anm_frame_count = anmfile->rotation_frame_count;

		node->anm_frame += anm_fps * elapsed;
		if (node->anm_frame >= anm_frame_count) {
			node->anm_frame = 0.0f;
		}

		float anm_frame_prev;
		float anm_frame_coef = modff(node->anm_frame, &anm_frame_prev);
		float anm_frame_next = anm_frame_prev + 1.0f;
		if (anm_frame_next >= anm_frame_count) {
			anm_frame_next = 0.0f;
		}

		ce_quat_init_array(&node->scenenode->orientation,
							anmfile->rotations + (int)anm_frame_prev * 4);

		if (NULL != anmfile->morphs) {
			assert(anmfile->rotation_frame_count ==
					anmfile->morph_frame_count);

			ce_renderitem_update(node->renderitem, anmfile->morphs +
				(int)anm_frame_prev * anmfile->morph_vertex_count,
				node->renderitem, anmfile->morphs + (int)anm_frame_next *
				anmfile->morph_vertex_count, anm_frame_coef);
		}
	}

	for (int i = 0, n = ce_vector_count(node->child_nodes); i < n; ++i) {
		ce_figentity_node* child_node = ce_vector_at(node->child_nodes, i);
		ce_figentity_node_advance(child_node, elapsed);
	}
}

static bool ce_figentity_node_play_animation(ce_figentity_node* node,
												const char* anm_name)
{
	bool ok = false;

	ce_vector* anmfiles = node->mesh_node->proto_node->anmfiles;
	for (int i = 0, n = ce_vector_count(anmfiles); i < n; ++i) {
		ce_anmfile* anmfile = ce_vector_at(anmfiles, i);
		if (0 == ce_string_cmp_cstr(anmfile->name, anm_name)) {
			node->anm_index = i;
			node->anm_frame = 0.0f;
			ok = true;
			break;
		}
	}

	for (int i = 0, n = ce_vector_count(node->child_nodes); i < n; ++i) {
		ce_figentity_node* child_node = ce_vector_at(node->child_nodes, i);
		ok = ce_figentity_node_play_animation(child_node, anm_name) && ok;
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
	if (NULL == figentity) {
		ce_logging_error("figentity: could not allocate memory");
		return NULL;
	}

	figentity->figmesh = ce_figmesh_copy(figmesh);

	if (NULL == (figentity->scenenode =
					ce_scenenode_create_child(parent_scenenode))) {
		ce_figentity_del(figentity);
		return NULL;
	}

	if (NULL == (figentity->root_node = ce_figentity_node_new(
				figmesh->root_node, texture_names, figentity->scenenode))) {
		ce_figentity_del(figentity);
		return NULL;
	}

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

bool ce_figentity_play_animation(ce_figentity* figentity, const char* anm_name)
{
	return ce_figentity_node_play_animation(figentity->root_node, anm_name);
}
