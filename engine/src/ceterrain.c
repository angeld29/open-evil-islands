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
	ce_renderitem* renderitem =
		ce_mprrenderitem_new(terrain->mprfile, terrain->tiling,
							sector_x, sector_z, water, terrain->textures);

	ce_mprhlp_get_aabb(&renderitem->aabb, terrain->mprfile, sector_x, sector_z);

	renderitem->position = CE_VEC3_ZERO;
	renderitem->orientation = CE_QUAT_IDENTITY;
	renderitem->bbox.aabb = renderitem->aabb;
	renderitem->bbox.axis = CE_QUAT_IDENTITY;

	ce_scenenode* scenenode = ce_scenenode_new(terrain->scenenode);
	ce_scenenode_add_renderitem(scenenode, renderitem);
}

ce_terrain* ce_terrain_new(ce_mprfile* mprfile, bool tiling,
							const ce_vec3* position,
							const ce_quat* orientation,
							ce_texture* stub_texture,
							ce_texture* textures[],
							ce_scenenode* scenenode)
{
	ce_terrain* terrain = ce_alloc(sizeof(ce_terrain));
	terrain->mprfile = mprfile;
	terrain->tiling = tiling;
	terrain->stub_texture = ce_texture_add_ref(stub_texture);
	terrain->textures = ce_vector_new_reserved(mprfile->texture_count);
	terrain->materials[0] = ce_mprhlp_create_material(mprfile, false);
	terrain->materials[1] = ce_mprhlp_create_material(mprfile, true);
	terrain->scenenode = ce_scenenode_new(scenenode);
	terrain->scenenode->position = *position;
	terrain->scenenode->orientation = *orientation;

	for (int i = 0; i < mprfile->texture_count; ++i) {
		ce_vector_push_back(terrain->textures,
			ce_texture_add_ref(textures[i]));
		ce_texture_wrap(textures[i], CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE);
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
		ce_vector_for_each(terrain->textures, (ce_vector_func1)ce_texture_del);
		ce_scenenode_del(terrain->scenenode);
		ce_material_del(terrain->materials[1]);
		ce_material_del(terrain->materials[0]);
		ce_vector_del(terrain->textures);
		ce_texture_del(terrain->stub_texture);
		ce_mprfile_close(terrain->mprfile);
		ce_free(terrain, sizeof(ce_terrain));
	}
}

void ce_terrain_create_rendergroup(ce_terrain* terrain,
									ce_renderqueue* renderqueue)
{
	ce_renderqueue_add(renderqueue, 0, terrain->materials[0]);
	ce_renderqueue_add(renderqueue, 100, terrain->materials[1]);
}

void ce_terrain_enqueue(ce_terrain* terrain, ce_renderqueue* renderqueue)
{
	ce_rendergroup* rendergroups[2] = { ce_renderqueue_get(renderqueue, 0),
										ce_renderqueue_get(renderqueue, 100) };

	int opaque_count = terrain->mprfile->sector_x_count *
						terrain->mprfile->sector_z_count;

	for (int i = 0; i < terrain->scenenode->childs->count; ++i) {
		ce_scenenode* scenenode = terrain->scenenode->childs->items[i];
		if (!scenenode->culled) {
			ce_rendergroup_add(rendergroups[i >= opaque_count],
				terrain->stub_texture, scenenode->renderitems->items[0]);
		}
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
