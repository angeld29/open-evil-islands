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
#include "ceterrain.h"

ce_terrain* ce_terrain_new(ce_mprfile* mprfile,
							const ce_vec3* position,
							const ce_quat* orientation,
							ce_texture* stub_texture,
							ce_texture* textures[],
							ce_scenenode* scenenode)
{
	ce_terrain* terrain = ce_alloc(sizeof(ce_terrain));
	terrain->mprfile = mprfile;
	terrain->stub_texture = ce_texture_add_ref(stub_texture);
	terrain->textures = ce_vector_new_reserved(mprfile->texture_count);
	terrain->scenenode = ce_scenenode_new(scenenode);
	terrain->scenenode->position = *position;
	terrain->scenenode->orientation = *orientation;

	for (int i = 0, n = mprfile->texture_count; i < n; ++i) {
		ce_vector_push_back(terrain->textures, ce_texture_add_ref(textures[i]));
	}

	if (!ce_terrain_create(terrain)) {
		terrain->mprfile = NULL;
		ce_terrain_del(terrain);
		return NULL;
	}

	return terrain;
}

void ce_terrain_del(ce_terrain* terrain)
{
	if (NULL != terrain) {
		ce_scenenode_del(terrain->scenenode);
		for (int i = 0; i < terrain->textures->count; ++i) {
			ce_texture_del(terrain->textures->items[i]);
		}
		ce_vector_del(terrain->textures);
		ce_texture_del(terrain->stub_texture);
		ce_mprfile_close(terrain->mprfile);
		ce_free(terrain, sizeof(ce_terrain));
	}
}
