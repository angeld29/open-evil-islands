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

#include "celogging.h"
#include "cealloc.h"
#include "cemprhlp.h"
#include "cemprrenderitem.h"
#include "ceterrain.h"

static void ce_terrain_create_sector(ce_terrain* terrain, bool tiling,
	ce_texmng* texmng, int sector_x, int sector_z, bool water)
{
	// mpr name + xxxzzz[w]
	char name[terrain->mprfile->name->length + 3 + 3 + 1 + 1];

	if (tiling) {
		ce_vector_push_back(terrain->sector_textures,
			ce_texture_add_ref(ce_texmng_get(texmng, "default0")));
	} else {
		snprintf(name, sizeof(name), water ? "%s%03d%03dw" :
			"%s%03d%03d", terrain->mprfile->name->str, sector_x, sector_z);

		ce_texture* texture = ce_texmng_get(texmng, name);
		if (NULL == texture) {
			// lazy loading tile mmpfiles
			if (ce_vector_empty(terrain->tile_resources)) {
				ce_logging_write("terrain: needs to generate "
					"textures for some sectors, please wait... ");
				ce_logging_write("terrain: loading tile mmp files...");
				// mpr name + nnn
				char name[terrain->mprfile->name->length + 3 + 1];
				for (int i = 0; i < terrain->mprfile->texture_count; ++i) {
					snprintf(name, sizeof(name), "%s%03d",
						terrain->mprfile->name->str, i);
					ce_vector_push_back(terrain->tile_resources,
						ce_mmpfile_convert_del(ce_texmng_open_mmpfile(texmng,
							name), CE_MMPFILE_FORMAT_R8G8B8A8));
				}
			}

			ce_logging_write("terrain: generating texture '%s'...", name);
			ce_mmpfile* mmpfile = ce_mprhlp_generate_mmpfile(terrain->mprfile,
				terrain->tile_resources, sector_x, sector_z, water);

			mmpfile = ce_mmpfile_convert_del(mmpfile, CE_MMPFILE_FORMAT_DXT1);
			ce_texmng_save_mmpfile(texmng, name, mmpfile);

			ce_texmng_put(texmng, texture = ce_texture_new(name, mmpfile));
			ce_mmpfile_del(mmpfile);
		}

		ce_texture_wrap(texture, CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE);
		ce_vector_push_back(terrain->sector_textures,
			ce_texture_add_ref(texture));
	}

	ce_renderitem* renderitem =
		ce_mprrenderitem_new(terrain->mprfile, tiling,
							sector_x, sector_z, water,
							terrain->tile_resources);

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
							ce_texmng* texmng,
							ce_scenenode* scenenode)
{
	ce_terrain* terrain = ce_alloc(sizeof(ce_terrain));
	terrain->mprfile = mprfile;
	terrain->tile_resources = ce_vector_new_reserved(mprfile->texture_count);
	terrain->sector_textures =
		ce_vector_new_reserved(2 * mprfile->sector_x_count *
									mprfile->sector_z_count);
	terrain->materials[0] = ce_mprhlp_create_material(mprfile, false);
	terrain->materials[1] = ce_mprhlp_create_material(mprfile, true);
	terrain->scenenode = ce_scenenode_new(scenenode);
	terrain->scenenode->position = *position;
	terrain->scenenode->orientation = *orientation;

	ce_logging_write("terrain: loading '%s'...", mprfile->name->str);

	if (tiling) { // load tile textures immediately
		ce_logging_write("terrain: loading tile textures...");
		// mpr name + nnn
		char name[terrain->mprfile->name->length + 3 + 1];
		for (int i = 0; i < mprfile->texture_count; ++i) {
			snprintf(name, sizeof(name), "%s%03d", mprfile->name->str, i);
			ce_vector_push_back(terrain->tile_resources,
				ce_texture_add_ref(ce_texmng_get(texmng, name)));
			ce_texture_wrap(terrain->tile_resources->items[i],
				CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE);
		}
	} else {
		ce_logging_write("terrain: loading sector textures...");
	}

	// opaque
	for (int z = 0; z < mprfile->sector_z_count; ++z) {
		for (int x = 0; x < mprfile->sector_x_count; ++x) {
			ce_terrain_create_sector(terrain, tiling, texmng, x, z, false);
		}
	}

	// transparent
	for (int z = 0; z < mprfile->sector_z_count; ++z) {
		for (int x = 0; x < mprfile->sector_x_count; ++x) {
			ce_mprsector* sector = terrain->mprfile->sectors + z *
									terrain->mprfile->sector_x_count + x;
			// do not add empty geometry
			if (NULL != sector->water_allow) {
				ce_terrain_create_sector(terrain, tiling, texmng, x, z, true);
			}
		}
	}

	if (!tiling) {
		ce_vector_for_each(terrain->tile_resources, ce_mmpfile_del);
		ce_vector_clear(terrain->tile_resources);
	}

	return terrain;
}

void ce_terrain_del(ce_terrain* terrain)
{
	if (NULL != terrain) {
		ce_vector_for_each(terrain->sector_textures, ce_texture_del);
		ce_vector_for_each(terrain->tile_resources, ce_texture_del);
		ce_scenenode_del(terrain->scenenode);
		ce_material_del(terrain->materials[1]);
		ce_material_del(terrain->materials[0]);
		ce_vector_del(terrain->sector_textures);
		ce_vector_del(terrain->tile_resources);
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
				terrain->sector_textures->items[i],
				scenenode->renderitems->items[0]);
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
