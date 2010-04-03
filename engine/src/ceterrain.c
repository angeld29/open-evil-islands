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

#include <stdbool.h>
#include <math.h>

#include "cealloc.h"
#include "cemprhlp.h"
#include "cemprrenderitem.h"
#include "ceterrain.h"

static void ce_terrain_create_sector(ce_terrain* terrain,
									int sector_x, int sector_z,
									bool water)
{
	ce_renderlayer* renderlayer = ce_renderlayer_new(
		ce_mprhlp_create_material(terrain->mprfile, water,
									terrain->stub_texture));
	ce_renderlayer_add_renderitem(renderlayer,
		ce_mprrenderitem_new(terrain->mprfile,
							sector_x, sector_z,
							water, terrain->textures));

	ce_scenenode* scenenode = ce_scenenode_new(terrain->scenenode);
	ce_scenenode_add_renderlayer(scenenode, renderlayer);
}

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

	for (int i = 0; i < mprfile->texture_count; ++i) {
		ce_vector_push_back(terrain->textures,
			ce_texture_add_ref(textures[i]));
	}

	// opaque
	for (int z = 0; z < mprfile->sector_z_count; ++z) {
		for (int x = 0; x < mprfile->sector_x_count; ++x) {
			ce_terrain_create_sector(terrain, x, z, false);
		}
	}

	// transparent
	for (int z = 0; z < mprfile->sector_z_count; ++z) {
		for (int x = 0; x < mprfile->sector_x_count; ++x) {
			ce_mprsector* sector = terrain->mprfile->sectors + z *
									terrain->mprfile->sector_x_count + x;
			// do not add empty geometry
			if (NULL != sector->water_allow) {
				ce_terrain_create_sector(terrain, x, z, true);
			}
		}
	}

	return terrain;
}

void ce_terrain_del(ce_terrain* terrain)
{
	if (NULL != terrain) {
		ce_scenenode_del(terrain->scenenode);
		ce_vector_for_each(terrain->textures, (ce_vector_func1)ce_texture_del);
		ce_vector_del(terrain->textures);
		ce_texture_del(terrain->stub_texture);
		ce_mprfile_close(terrain->mprfile);
		ce_free(terrain, sizeof(ce_terrain));
	}
}

ce_scenenode* ce_terrain_find_scenenode(ce_terrain* terrain, float x, float z)
{
	// FIXME: opengl hard-code
	z = fabsf(z);

	int sector_x = (int)x / (CE_MPRFILE_VERTEX_SIDE - 1);
	int sector_z = (int)z / (CE_MPRFILE_VERTEX_SIDE - 1);

	return terrain->scenenode->childs->items[sector_z *
		terrain->mprfile->sector_x_count + sector_x];
}
