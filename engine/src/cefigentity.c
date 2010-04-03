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

/*static void ce_figentity_assign_renderitems(ce_figentity* figentity,
											ce_fignode* fignode)
{
	// FIXME: to be reversed...
	int index = fignode->figfile->texture_number - 1;
	if (index > 1) {
		index = 0;
	}

	ce_renderlayer* renderlayer =
		figentity->scenenode->renderlayers->items[index];

	ce_renderlayer_add_renderitem(renderlayer,
		figentity->renderitems->items[fignode->index]);

	for (int i = 0; i < fignode->childs->count; ++i) {
		ce_figentity_assign_renderitems(figentity, fignode->childs->items[i]);
	}
}*/

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
	figentity->scenenode = ce_scenenode_new(scenenode);
	figentity->scenenode->position = *position;
	figentity->scenenode->orientation = *orientation;

	for (int i = 0; i < figmesh->renderitems->count; ++i) {
		ce_scenenode_add_renderitem(figentity->scenenode,
			ce_renderitem_clone(figmesh->renderitems->items[i]));
	}

	/*for (int i = 0; i < texture_count; ++i) {
		ce_scenenode_add_renderlayer(figentity->scenenode,
			ce_renderlayer_new(ce_fighlp_create_material(textures[i])));
	}*/

	//ce_figentity_assign_renderitems(figentity, figmesh->figproto->fignode);

	return figentity;
}

void ce_figentity_del(ce_figentity* figentity)
{
	if (NULL != figentity) {
		ce_scenenode_del(figentity->scenenode);
		ce_figbone_del(figentity->figbone);
		ce_figmesh_del(figentity->figmesh);
		ce_free(figentity, sizeof(ce_figentity));
	}
}

void ce_figentity_advance(ce_figentity* figentity, float fps, float elapsed)
{
	ce_figbone_advance(figentity->figbone, fps, elapsed);
}

void ce_figentity_update(ce_figentity* figentity, bool force)
{
	if (!force && figentity->scenenode->culled) {
		return;
	}

	ce_figbone_update(figentity->figbone,
		figentity->figmesh->figproto->fignode,
		figentity->scenenode->renderitems);
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
