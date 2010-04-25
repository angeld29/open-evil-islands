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
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "cefighlp.h"
#include "cefigentity.h"

static void ce_figentity_scenenode_about_to_update(void* listener,
													float anmfps, float elapsed)
{
	ce_figentity* figentity = listener;
	ce_figbone_advance(figentity->figbone, anmfps * elapsed);
	ce_figbone_update(figentity->figbone,
		figentity->figmesh->figproto->fignode,
		figentity->scenenode->renderitems);
}

static void ce_figentity_enqueue(ce_figentity* figentity, ce_fignode* fignode)
{
	ce_renderlayer_add(figentity->renderlayers->items[fignode->index],
		figentity->scenenode->renderitems->items[fignode->index]);

	for (int i = 0; i < fignode->childs->count; ++i) {
		ce_figentity_enqueue(figentity, fignode->childs->items[i]);
	}
}

static void ce_figentity_scenenode_updated(void* listener)
{
	ce_figentity* figentity = listener;
	ce_figentity_enqueue(figentity, figentity->figmesh->figproto->fignode);
}

static void ce_figentity_create_renderlayers(ce_figentity* figentity,
												ce_fignode* fignode)
{
	// FIXME: to be reversed...
	int index = fignode->figfile->texture_number - 1;
	if (index > 1) {
		index = 0;
	}

	ce_vector_push_back(figentity->renderlayers,
		ce_rendergroup_get(fignode->rendergroup,
							figentity->textures->items[index]));

	for (int i = 0; i < fignode->childs->count; ++i) {
		ce_figentity_create_renderlayers(figentity, fignode->childs->items[i]);
	}
}

ce_figentity* ce_figentity_new(ce_figmesh* figmesh,
								const ce_vec3* position,
								const ce_quat* orientation,
								int texture_count,
								ce_texture* textures[],
								ce_scenenode* scenenode)
{
	ce_scenenode_listener_vtable listener_vtable = {
		NULL, NULL, ce_figentity_scenenode_about_to_update,
		ce_figentity_scenenode_updated, NULL
	};

	ce_figentity* figentity = ce_alloc(sizeof(ce_figentity));
	figentity->figmesh = ce_figmesh_add_ref(figmesh);
	figentity->figbone = ce_figbone_new(figmesh->figproto->fignode,
										&figmesh->complection, NULL);
	figentity->textures = ce_vector_new_reserved(texture_count);
	figentity->renderlayers = ce_vector_new();
	figentity->scenenode = ce_scenenode_new(scenenode);
	figentity->scenenode->position = *position;
	figentity->scenenode->orientation = *orientation;
	figentity->scenenode->occlusion = ce_occlusion_new();
	figentity->scenenode->listener_vtable = listener_vtable;
	figentity->scenenode->listener = figentity;

	for (int i = 0; i < texture_count; ++i) {
		ce_vector_push_back(figentity->textures,
			ce_texture_add_ref(textures[i]));
		ce_texture_wrap(textures[i], CE_TEXTURE_WRAP_MODE_REPEAT);
	}

	ce_figentity_create_renderlayers(figentity, figentity->figmesh->figproto->fignode);

	for (int i = 0; i < figmesh->renderitems->count; ++i) {
		ce_scenenode_add_renderitem(figentity->scenenode,
			ce_renderitem_clone(figmesh->renderitems->items[i]));
	}

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
