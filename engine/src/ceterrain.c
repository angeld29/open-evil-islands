/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceoptionmanager.h"
#include "ceeventmanager.h"
#include "cerendersystem.h"
#include "cetexturemanager.h"
#include "cemprhlp.h"
#include "cemprrenderitem.h"
#include "ceterrain.h"

static void ce_scenenode_updated(void* listener)
{
	ce_terrain_sector* sector = listener;
	ce_renderlayer_add(sector->renderlayer, sector->renderitem);
}

static void ce_terrain_load_tile_mmpfiles(ce_terrain* terrain)
{
	ce_logging_debug("ce_terrain_load_tile_mmpfiles");
	char name[terrain->mprfile->name->length + 3 + 1];
	for (int i = 0; i < terrain->mprfile->texture_count; ++i) {
		snprintf(name, sizeof(name), "%s%03d", terrain->mprfile->name->str, i);
		ce_mmpfile* mmpfile = ce_texture_manager_open_mmpfile(name);
		ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
		ce_vector_push_back(terrain->tile_mmpfiles, mmpfile);
	}
}

static void ce_terrain_load_tile_textures(ce_terrain* terrain)
{
	ce_logging_debug("ce_terrain_load_tile_textures");
	char name[terrain->mprfile->name->length + 3 + 1];
	for (int i = 0; i < terrain->mprfile->texture_count; ++i) {
		snprintf(name, sizeof(name), "%s%03d", terrain->mprfile->name->str, i);
		ce_texture* texture = ce_texture_add_ref(ce_texture_manager_get(name));
		ce_texture_wrap(texture, CE_TEXTURE_WRAP_CLAMP_TO_EDGE);
		ce_vector_push_back(terrain->tile_textures, texture);
	}
}

typedef struct {
	ce_terrain_sector* sector;
} ce_test;

static void ce_terrain_sector_process(ce_event* event)
{
	ce_terrain_sector* sector = ((ce_test*)event->impl)->sector;

	ce_logging_debug("ce_terrain_sector_process %d %d", sector->x, sector->z);

	//ce_logging_debug("%p", sector);

	if (ce_option_manager->terrain_tiling) {
		sector->texture = ce_texture_add_ref(ce_texture_manager_get("default0"));
	} else {
		sector->texture = ce_texture_new(sector->name->str, sector->mmpfile);
		ce_mmpfile_del(sector->mmpfile);
	}

	ce_texture_wrap(sector->texture, CE_TEXTURE_WRAP_CLAMP_TO_EDGE);

	sector->renderlayer = ce_rendergroup_get(sector->
		terrain->rendergroups[sector->water], sector->texture);

	//ce_texmng_put(ce_texture_add_ref(sector->texture));

	if (ce_option_manager->terrain_tiling) {
		// tile textures are necessary for geometry creation if tiling
		ce_once_exec(sector->terrain->tile_once,
			ce_terrain_load_tile_textures, sector->terrain);
	}

	sector->renderitem = ce_mprrenderitem_new(sector->terrain->mprfile,
		sector->x, sector->z, sector->water, sector->terrain->tile_textures);

	ce_mprhlp_get_aabb(&sector->renderitem->aabb,
		sector->terrain->mprfile, sector->x, sector->z, sector->water);

	sector->renderitem->position = CE_VEC3_ZERO;
	sector->renderitem->orientation = CE_QUAT_IDENTITY;
	sector->renderitem->bbox.aabb = sector->renderitem->aabb;
	sector->renderitem->bbox.axis = CE_QUAT_IDENTITY;

	ce_scenenode* scenenode = ce_scenenode_new(sector->terrain->scenenode);
	scenenode->listener = (ce_scenenode_listener)
						{NULL, NULL, NULL, ce_scenenode_updated, NULL, sector};

	ce_scenenode_add_renderitem(scenenode, sector->renderitem);
}

static void ce_terrain_sector_exec(ce_terrain_sector* sector)
{
	ce_logging_debug("ce_terrain_sector_exec %d %d", sector->x, sector->z);

	if (!ce_option_manager->terrain_tiling) {
		// lazy loading tile mmp files
		ce_once_exec(sector->terrain->tile_once,
			ce_terrain_load_tile_mmpfiles, sector->terrain);

		if (NULL == sector->mmpfile ||
				sector->mmpfile->version < CE_MMPFILE_VERSION ||
				sector->mmpfile->user_info < CE_MPRHLP_MMPFILE_VERSION) {
		}

		sector->mmpfile = ce_mprhlp_generate_mmpfile(sector->terrain->mprfile,
			sector->terrain->tile_mmpfiles, sector->x, sector->z, sector->water);

		// force to dxt1?
		ce_mmpfile_convert(sector->mmpfile, CE_MMPFILE_FORMAT_DXT1);

		//ce_texmng_save_mmpfile(sector->name->str, sector->mmpfile);
	}

	//ce_logging_debug("%p", sector);

	ce_event_manager_post_raw(ce_render_system->thread_id,
		ce_terrain_sector_process, (ce_test[]){{sector}}, sizeof(ce_test));
}

ce_terrain_sector* ce_terrain_sector_new(ce_terrain* terrain,
										const char* name,
										int x, int z, bool water)
{
	ce_terrain_sector* sector = ce_alloc_zero(sizeof(ce_terrain_sector));
	sector->name = ce_string_new_str(name);
	sector->x = x;
	sector->z = z;
	sector->water = water;
	sector->terrain = terrain;
	ce_thread_pool_enqueue(ce_terrain_sector_exec, sector);
	return sector;
}

void ce_terrain_sector_del(ce_terrain_sector* sector)
{
	if (NULL != sector) {
		ce_texture_del(sector->texture);
		ce_string_del(sector->name);
		ce_free(sector, sizeof(ce_terrain_sector));
	}
}

ce_terrain* ce_terrain_new(ce_mprfile* mprfile,
							ce_renderqueue* renderqueue,
							const ce_vec3* position,
							const ce_quat* orientation,
							ce_scenenode* scenenode)
{
	ce_logging_info("terrain: loading '%s'...", mprfile->name->str);

	ce_terrain* terrain = ce_alloc_zero(sizeof(ce_terrain));
	terrain->mprfile = mprfile;
	terrain->materials[CE_MPRFILE_MATERIAL_LAND] = ce_mprhlp_create_material(mprfile, false);
	terrain->materials[CE_MPRFILE_MATERIAL_WATER] = ce_mprhlp_create_material(mprfile, true);
	terrain->rendergroups[CE_MPRFILE_MATERIAL_LAND] =
		ce_renderqueue_get(renderqueue, 0, terrain->materials[CE_MPRFILE_MATERIAL_LAND]);
	terrain->rendergroups[CE_MPRFILE_MATERIAL_WATER] =
		ce_renderqueue_get(renderqueue, 100, terrain->materials[CE_MPRFILE_MATERIAL_WATER]);
	terrain->tile_mmpfiles = ce_vector_new_reserved(mprfile->texture_count);
	terrain->tile_textures = ce_vector_new_reserved(mprfile->texture_count);
	terrain->tile_once = ce_once_new();
	terrain->sectors = ce_vector_new_reserved(2 * mprfile->sector_x_count *
													mprfile->sector_z_count);
	terrain->scenenode = ce_scenenode_new(scenenode);
	terrain->scenenode->position = *position;
	terrain->scenenode->orientation = *orientation;

	char name[terrain->mprfile->name->length + 3 + 3 + 1 + 1];
	for (int i = 0; i < CE_MPRFILE_MATERIAL_COUNT; ++i) {
		bool water = CE_MPRFILE_MATERIAL_WATER == i;
		for (int z = 0; z < terrain->mprfile->sector_z_count; ++z) {
			for (int x = 0; x < terrain->mprfile->sector_x_count; ++x) {
				ce_mprsector* mpr_sector = terrain->mprfile->sectors + z *
											terrain->mprfile->sector_x_count + x;
				if (water && NULL == mpr_sector->water_allow) {
					// do not add empty geometry
					continue;
				}
				snprintf(name, sizeof(name), water ? "%s%03d%03dw" :
					"%s%03d%03d", terrain->mprfile->name->str, x, z);
				ce_vector_push_back(terrain->sectors,
					ce_terrain_sector_new(terrain, name, x, z, water));
			}
		}
	}

	ce_logging_info("terrain: %d jobs queued", terrain->queued_job_count);
	ce_logging_info("terrain: done loading '%s'", mprfile->name->str);

	return terrain;
}

void ce_terrain_del(ce_terrain* terrain)
{
	if (NULL != terrain) {
		ce_scenenode_del(terrain->scenenode);
		ce_vector_for_each(terrain->sectors, ce_terrain_sector_del);
		ce_vector_del(terrain->sectors);
		ce_once_del(terrain->tile_once);
		ce_vector_for_each(terrain->tile_textures, ce_texture_del);
		ce_vector_del(terrain->tile_textures);
		ce_vector_for_each(terrain->tile_mmpfiles, ce_mmpfile_del);
		ce_vector_del(terrain->tile_mmpfiles);
		ce_material_del(terrain->materials[CE_MPRFILE_MATERIAL_WATER]);
		ce_material_del(terrain->materials[CE_MPRFILE_MATERIAL_LAND]);
		ce_mprfile_close(terrain->mprfile);
		ce_free(terrain, sizeof(ce_terrain));
	}
}

ce_scenenode* ce_terrain_find_scenenode(ce_terrain* terrain, float x, float z)
{
	// FIXME: opengl hard-code
	z = fabsf(z);

	size_t sector_x = (int)x / (CE_MPRFILE_VERTEX_SIDE - 1);
	size_t sector_z = (int)z / (CE_MPRFILE_VERTEX_SIDE - 1);

	return terrain->scenenode->childs->items[sector_z *
		terrain->mprfile->sector_x_count + sector_x];
}
