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

#include "cealloc.h"
#include "cefighlp.h"
#include "cefigentity.h"

typedef struct {
	ce_figentity* figentity;
} ce_figentity_listener;

static void ce_figentity_listener_ctor(
	ce_scenenode_listener* base_listener, va_list args)
{
	ce_figentity_listener* listener = (ce_figentity_listener*)base_listener->impl;

	listener->figentity = va_arg(args, ce_figentity*);
}

static void ce_figentity_listener_about_to_update(
	ce_scenenode_listener* base_listener, float anmfps, float elapsed)
{
	ce_figentity_listener* listener = (ce_figentity_listener*)base_listener->impl;

	ce_figbone_advance(listener->figentity->figbone, anmfps * elapsed);
	ce_figbone_update(listener->figentity->figbone,
		listener->figentity->figmesh->figproto->fignode,
		listener->figentity->scenenode->renderitems);
}

static const ce_scenenode_listener_vtable ce_figentity_listener_vtable = {
	ce_figentity_listener_ctor, NULL, NULL, NULL,
	ce_figentity_listener_about_to_update, NULL, NULL
};

ce_figentity* ce_figentity_new(ce_figmesh* figmesh,
								const ce_vec3* position,
								const ce_quat* orientation,
								int texture_count,
								ce_texture* textures[],
								ce_scenenode* scenenode)
{
	ce_figentity* figentity = ce_alloc(sizeof(ce_figentity));
	figentity->figmesh = ce_figmesh_add_ref(figmesh);
	figentity->figbone = ce_figbone_new(figmesh->figproto->fignode,
										&figmesh->complection, NULL);
	figentity->textures = ce_vector_new_reserved(texture_count);
	figentity->scenenode = ce_scenenode_new(scenenode);
	figentity->scenenode->position = *position;
	figentity->scenenode->orientation = *orientation;

	ce_scenenode_create_listener(figentity->scenenode,
		ce_figentity_listener_vtable, sizeof(ce_figentity_listener), figentity);

	for (int i = 0; i < texture_count; ++i) {
		ce_vector_push_back(figentity->textures,
			ce_texture_add_ref(textures[i]));
		ce_texture_wrap(textures[i], CE_TEXTURE_WRAP_MODE_REPEAT);
	}

	for (int i = 0; i < figmesh->renderitems->count; ++i) {
		ce_scenenode_add_renderitem(figentity->scenenode,
			ce_renderitem_clone(figmesh->renderitems->items[i]));
	}

	return figentity;
}

void ce_figentity_del(ce_figentity* figentity)
{
	if (NULL != figentity) {
		ce_vector_for_each(figentity->textures, ce_texture_del);
		ce_scenenode_del(figentity->scenenode);
		ce_vector_del(figentity->textures);
		ce_figbone_del(figentity->figbone);
		ce_figmesh_del(figentity->figmesh);
		ce_free(figentity, sizeof(ce_figentity));
	}
}

static void ce_figentity_enqueue_nodes(ce_figentity* figentity,
										ce_fignode* fignode,
										ce_renderqueue* renderqueue)
{
	// FIXME: to be reversed...
	int index = fignode->figfile->texture_number - 1;
	if (index > 1) {
		index = 0;
	}

	ce_rendergroup* rendergroup =
		ce_renderqueue_get(renderqueue, fignode->figfile->group);

	ce_rendergroup_add(rendergroup, figentity->textures->items[index],
		figentity->scenenode->renderitems->items[fignode->index]);

	for (int i = 0; i < fignode->childs->count; ++i) {
		ce_figentity_enqueue_nodes(figentity,
			fignode->childs->items[i], renderqueue);
	}
}

void ce_figentity_enqueue(ce_figentity* figentity, ce_renderqueue* renderqueue)
{
	if (!figentity->scenenode->culled) {
		ce_figentity_enqueue_nodes(figentity,
			figentity->figmesh->figproto->fignode, renderqueue);
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
