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
#include <float.h>
#include <math.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"
#include "cetexturemanager.h"
#include "cefighlp.h"
#include "cefigentity.h"

static void ce_figentity_scenenode_about_to_update(void* listener)
{
	ce_figentity* figentity = listener;

	ce_figbone_advance(figentity->figbone, ce_root.animation_fps * ce_root.timer->elapsed);
	ce_figbone_update(figentity->figbone, figentity->figmesh->figproto->fignode,
											figentity->scenenode->renderitems);

	ce_vec3_copy(&figentity->scenenode->position, &figentity->position);
	ce_quat_copy(&figentity->scenenode->orientation, &figentity->orientation);

	figentity->scenenode->position.y += figentity->height_correction;
}

static void ce_figentity_enqueue(ce_figentity* figentity, ce_fignode* fignode)
{
	ce_renderlayer_add(figentity->renderlayers->items[fignode->index],
		figentity->scenenode->renderitems->items[fignode->index]);

	for (size_t i = 0; i < fignode->childs->count; ++i) {
		ce_figentity_enqueue(figentity, fignode->childs->items[i]);
	}
}

static float ce_figentity_find_min_y(ce_figentity* figentity, ce_fignode* fignode)
{
	float y = FLT_MAX;

	// what about small piece of hard-code? :)
	if (strstr(fignode->name->str, "leg") ||
			(fignode->name->length >= 2 && 'l' == fignode->name->str[1] &&
			('l' == fignode->name->str[0] || 'r' == fignode->name->str[0]))) {
		ce_renderitem* renderitem = figentity->scenenode->renderitems->items[fignode->index];
		y = renderitem->world_position.y - renderitem->world_bbox.aabb.radius;
	}

	for (size_t i = 0; i < fignode->childs->count; ++i) {
		y = fminf(y, ce_figentity_find_min_y(figentity, fignode->childs->items[i]));
	}

	return y;
}

static void ce_figentity_scenenode_updated(void* listener)
{
	ce_figentity* figentity = listener;

	if (NULL != figentity->figmesh->figproto->adb_file) {
		// WARNING: begin of experimental code

		// fix figure height relative to root node
		// it's a difference between root and legs

		ce_renderitem* root_renderitem = figentity->scenenode->renderitems->items
										[figentity->figmesh->figproto->fignode->index];

		float y = ce_figentity_find_min_y(figentity, figentity->figmesh->figproto->fignode);

		if (y <= root_renderitem->world_position.y) {
			y = root_renderitem->world_position.y - y;
		} else {
			// flying creatures ???
			y = 1.0f;
		}

		figentity->scenenode->world_position.y += y;
		figentity->scenenode->world_bbox.aabb.origin.y += y;
		for (size_t i = 0; i < figentity->scenenode->renderitems->count; ++i) {
			ce_renderitem* renderitem = figentity->scenenode->renderitems->items[i];
			if (renderitem->visible) {
				renderitem->world_position.y += y;
				renderitem->world_bbox.aabb.origin.y += y;
			}
		}

		// WARNING: end of experimental code
	}

	ce_figentity_enqueue(figentity, figentity->figmesh->figproto->fignode);
}

static void ce_figentity_create_renderlayers(ce_figentity* figentity,
												const char* parts[],
												ce_fignode* fignode)
{
	// FIXME: to be reversed...
	size_t index = fignode->figfile->texture_number - 1;
	if (index > 1) {
		index = 0;
	}

	ce_vector_push_back(figentity->renderlayers,
		ce_rendergroup_get(fignode->rendergroup,
							figentity->textures->items[index]));

	if (NULL != parts[0]) {
		ce_renderitem* renderitem = figentity->scenenode->renderitems->items[fignode->index];
		renderitem->visible = false;
		for (size_t i = 0; NULL != parts[i]; ++i) {
			if (0 == strcmp(parts[i], fignode->name->str)) {
				renderitem->visible = true;
			}
		}
	}

	for (size_t i = 0; i < fignode->childs->count; ++i) {
		ce_figentity_create_renderlayers(figentity, parts, fignode->childs->items[i]);
	}
}

ce_figentity* ce_figentity_new(ce_figmesh* figmesh,
								const ce_vec3* position,
								const ce_quat* orientation,
								const char* parts[],
								const char* textures[],
								ce_scenenode* scenenode)
{
	ce_figentity* figentity = ce_alloc_zero(sizeof(ce_figentity));
	figentity->figmesh = ce_figmesh_add_ref(figmesh);
	figentity->figbone = ce_figbone_new(figmesh->figproto->fignode,
										&figmesh->complection, NULL);
	figentity->textures = ce_vector_new_reserved(2);
	figentity->renderlayers = ce_vector_new();
	figentity->scenenode = ce_scenenode_new(scenenode);
	figentity->scenenode->occlusion = ce_occlusion_new();
	figentity->scenenode->listener = (ce_scenenode_listener)
		{NULL, NULL, ce_figentity_scenenode_about_to_update,
		ce_figentity_scenenode_updated, NULL, figentity};

	ce_vec3_copy(&figentity->position, position);
	ce_quat_copy(&figentity->orientation, orientation);

	for (size_t i = 0; NULL != textures[i]; ++i) {
		ce_vector_push_back(figentity->textures, ce_texture_manager_get(textures[i]));
		if (NULL == figentity->textures->items[i]) {
			figentity->textures->items[i] = ce_texture_manager_get("default0");
			ce_logging_error("figure entity: could not find texture '%s' for '%s', "
				"force to 'default0'", textures[i], figmesh->figproto->name->str);
		}
		ce_texture_add_ref(figentity->textures->items[i]);
		ce_texture_wrap(figentity->textures->items[i], CE_TEXTURE_WRAP_REPEAT);
	}

	for (size_t i = 0; i < figmesh->renderitems->count; ++i) {
		ce_scenenode_add_renderitem(figentity->scenenode,
			ce_renderitem_clone(figmesh->renderitems->items[i]));
	}

	ce_figentity_create_renderlayers(figentity, parts,
									figentity->figmesh->figproto->fignode);

	return figentity;
}

void ce_figentity_del(ce_figentity* figentity)
{
	if (NULL != figentity) {
		ce_scenenode_del(figentity->scenenode);
		ce_vector_del(figentity->renderlayers);
		ce_vector_for_each(figentity->textures, ce_texture_del);
		ce_vector_del(figentity->textures);
		ce_figbone_del(figentity->figbone);
		ce_figmesh_del(figentity->figmesh);
		ce_free(figentity, sizeof(ce_figentity));
	}
}

void ce_figentity_fix_height(ce_figentity* figentity, float height)
{
	figentity->height_correction = height;
}

int ce_figentity_get_animation_count(ce_figentity* figentity)
{
	return figentity->figmesh->figproto->fignode->anmfiles->count;
}

const char* ce_figentity_get_animation_name(ce_figentity* figentity, int index)
{
	ce_fignode* fignode = figentity->figmesh->figproto->fignode;
	ce_anmfile* anmfile = fignode->anmfiles->items[index];
	return anmfile->name->str;
}

bool ce_figentity_play_animation(ce_figentity* figentity, const char* name)
{
	return ce_figbone_play_animation(figentity->figbone,
		figentity->figmesh->figproto->fignode, name);
}

void ce_figentity_stop_animation(ce_figentity* figentity)
{
	ce_figbone_stop_animation(figentity->figbone,
		figentity->figmesh->figproto->fignode);
}
