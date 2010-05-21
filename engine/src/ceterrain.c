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

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cestring.h"
#include "cethread.h"
#include "ceroot.h"
#include "cemprhlp.h"
#include "cemprrenderitem.h"
#include "ceterrain.h"

static void ce_terrain_sector_scenenode_updated(void* listener)
{
	ce_terrain_sector* sector = listener;
	ce_renderlayer_add(sector->renderlayer, sector->renderitem);
}

static ce_terrain_sector* ce_terrain_sector_new(void)
{
	ce_terrain_sector* sector = ce_alloc(sizeof(ce_terrain_sector));
	return sector;
}

static void ce_terrain_sector_del(ce_terrain_sector* sector)
{
	if (NULL != sector) {
		ce_texture_del(sector->texture);
		ce_free(sector, sizeof(ce_terrain_sector));
	}
}

typedef struct {
	ce_terrain* terrain;
	ce_texmng* texmng;
	ce_rendergroup* rendergroups[CE_MPRFILE_MATERIAL_COUNT];
	ce_thread_pool* pool;
	ce_thread_mutex* mutex;
	ce_thread_once* once;
	ce_vector* tile_mmpfiles;
	ce_vector* portions;
	int queued_portion_count;
	int completed_portion_count;
} ce_terrain_cookie;

static ce_terrain_cookie* ce_terrain_cookie_new(ce_terrain* terrain,
												ce_texmng* texmng,
												ce_renderqueue* renderqueue)
{
	ce_terrain_cookie* cookie = ce_alloc(sizeof(ce_terrain_cookie));
	cookie->terrain = terrain;
	cookie->texmng = texmng;
	cookie->rendergroups[CE_MPRFILE_MATERIAL_LAND] =
		ce_renderqueue_get(renderqueue, 0,
							terrain->materials[CE_MPRFILE_MATERIAL_LAND]);
	cookie->rendergroups[CE_MPRFILE_MATERIAL_WATER] =
		ce_renderqueue_get(renderqueue, 100,
							terrain->materials[CE_MPRFILE_MATERIAL_WATER]);
	cookie->pool = ce_thread_pool_new(ce_root.thread_count);
	cookie->mutex = ce_thread_mutex_new();
	cookie->once = ce_thread_once_new();
	cookie->tile_mmpfiles = ce_vector_new_reserved(terrain->mprfile->texture_count);
	cookie->portions = ce_vector_new();
	cookie->queued_portion_count = 0;
	cookie->completed_portion_count = 0;
	return cookie;
}

static void ce_terrain_cookie_del(ce_terrain_cookie* cookie)
{
	if (NULL != cookie) {
		assert(ce_vector_empty(cookie->portions) && "internal error");
		ce_vector_del(cookie->portions);
		ce_vector_for_each(cookie->tile_mmpfiles, ce_mmpfile_del);
		ce_vector_del(cookie->tile_mmpfiles);
		ce_thread_once_del(cookie->once);
		ce_thread_mutex_del(cookie->mutex);
		ce_thread_pool_del(cookie->pool);
		ce_free(cookie, sizeof(ce_terrain_cookie));
	}
}

typedef struct {
	ce_terrain_cookie* cookie;
	ce_string* name;
	int index;
	int x, z;
	bool water;
	ce_mmpfile* mmpfile;
} ce_terrain_portion;

static ce_terrain_portion* ce_terrain_portion_new(ce_terrain_cookie* cookie,
	const char* name, int index, int x, int z, bool water)
{
	ce_terrain_portion* portion = ce_alloc(sizeof(ce_terrain_portion));
	portion->cookie = cookie;
	portion->name = ce_string_new_str(name);
	portion->index = index;
	portion->x = x;
	portion->z = z;
	portion->water = water;
	portion->mmpfile = NULL;
	return portion;
}

static void ce_terrain_portion_del(ce_terrain_portion* portion)
{
	if (NULL != portion) {
		ce_mmpfile_del(portion->mmpfile);
		ce_string_del(portion->name);
		ce_free(portion, sizeof(ce_terrain_portion));
	}
}

typedef void (*ce_terrain_load_tile_func)(ce_terrain_cookie*, const char*);

static void ce_terrain_load_tile_resources(ce_terrain_cookie* cookie,
	const char* info, ce_terrain_load_tile_func func)
{
	ce_logging_info("terrain: loading tile %s...", info);

	// mpr name + nnn
	char name[cookie->terrain->mprfile->name->length + 3 + 1];

	for (int i = 0; i < cookie->terrain->mprfile->texture_count; ++i) {
		snprintf(name, sizeof(name), "%s%03d",
				cookie->terrain->mprfile->name->str, i);
		(*func)(cookie, name);
	}
}

static void ce_terrain_load_tile_mmpfile(ce_terrain_cookie* cookie, const char* name)
{
	ce_mmpfile* mmpfile = ce_texmng_open_mmpfile(cookie->texmng, name);
	ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
	ce_vector_push_back(cookie->tile_mmpfiles, mmpfile);
}

static void ce_terrain_load_tile_texture(ce_terrain_cookie* cookie, const char* name)
{
	ce_texture* texture = ce_texture_add_ref(ce_texmng_get(cookie->texmng, name));
	ce_texture_wrap(texture, CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE);
	ce_vector_push_back(cookie->terrain->tile_textures, texture);
}

static void ce_terrain_load_tile_mmpfiles(ce_terrain_cookie* cookie)
{
	ce_logging_write("terrain: needs to generate textures "
						"for some sectors, please wait...");
	ce_terrain_load_tile_resources(cookie, "mmp files", ce_terrain_load_tile_mmpfile);
}

static void ce_terrain_load_tile_textures(ce_terrain_cookie* cookie)
{
	ce_terrain_load_tile_resources(cookie, "textures", ce_terrain_load_tile_texture);
}

static void ce_terrain_process_portion(ce_terrain_portion* portion)
{
	ce_thread_mutex_lock(portion->cookie->mutex);

	// TODO: describe thread-safe texmng operations
	portion->mmpfile = ce_texmng_open_mmpfile(portion->cookie->texmng,
												portion->name->str);

	ce_thread_mutex_unlock(portion->cookie->mutex);

	if (NULL == portion->mmpfile ||
			portion->mmpfile->version < CE_MMPFILE_VERSION ||
			portion->mmpfile->user_info < CE_MPRHLP_MMPFILE_VERSION) {
		// lazy loading tile mmp files
		ce_thread_once_exec(portion->cookie->once,
			ce_terrain_load_tile_mmpfiles, portion->cookie);

		ce_mmpfile_del(portion->mmpfile);
		portion->mmpfile =
			ce_mprhlp_generate_mmpfile(portion->cookie->terrain->mprfile,
										portion->cookie->tile_mmpfiles,
										portion->x, portion->z, portion->water);

		// force to dxt1?
		ce_mmpfile_convert(portion->mmpfile, CE_MMPFILE_FORMAT_DXT1);

		ce_texmng_save_mmpfile(portion->cookie->texmng,
								portion->name->str, portion->mmpfile);
	}

	ce_thread_mutex_lock(portion->cookie->mutex);

	ce_vector_push_back(portion->cookie->portions, portion);
	++portion->cookie->completed_portion_count;

	ce_thread_mutex_unlock(portion->cookie->mutex);
}

static void ce_terrain_load_portions(ce_terrain_cookie* cookie)
{
	ce_thread_mutex_lock(cookie->mutex);

	for (int i = 0; i < cookie->portions->count; ++i) {
		ce_terrain_portion* portion = cookie->portions->items[i];
		ce_terrain_sector* sector = cookie->terrain->sectors->items[portion->index];

		sector->texture = ce_texture_new(portion->name->str, portion->mmpfile);
		ce_texture_wrap(sector->texture, CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE);

		ce_rendergroup* rendergroup = cookie->rendergroups[portion->water];
		sector->renderlayer = ce_rendergroup_get(rendergroup, sector->texture);

		ce_texmng_put(cookie->texmng, ce_texture_add_ref(sector->texture));

		ce_terrain_portion_del(portion);
	}

	ce_vector_clear(cookie->portions);

	ce_thread_mutex_unlock(cookie->mutex);
}

static void ce_terrain_create_sector(ce_terrain_cookie* cookie,
	const char* name, int index, int x, int z, bool water)
{
	ce_terrain_sector* sector = ce_terrain_sector_new();
	ce_vector_push_back(cookie->terrain->sectors, sector);

	if (ce_root.terrain_tiling) {
		sector->texture = ce_texture_add_ref(ce_texmng_get(cookie->texmng, "default0"));
		ce_texture_wrap(sector->texture, CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE);

		ce_rendergroup* rendergroup = cookie->rendergroups[water];
		sector->renderlayer = ce_rendergroup_get(rendergroup, sector->texture);
	} else {
		// enqueue mmp file loading or generation
		ce_thread_pool_enqueue(cookie->pool, ce_terrain_process_portion,
			ce_terrain_portion_new(cookie, name, index, x, z, water));
		++cookie->queued_portion_count;

		// load the portions as soon as processed to avoid extra memory usage
		ce_terrain_load_portions(cookie);
	}

	// create geometry
	sector->renderitem = ce_mprrenderitem_new(cookie->terrain->mprfile,
		x, z, water, cookie->terrain->tile_textures);

	ce_mprhlp_get_aabb(&sector->renderitem->aabb,
						cookie->terrain->mprfile, x, z, water);

	sector->renderitem->position = CE_VEC3_ZERO;
	sector->renderitem->orientation = CE_QUAT_IDENTITY;
	sector->renderitem->bbox.aabb = sector->renderitem->aabb;
	sector->renderitem->bbox.axis = CE_QUAT_IDENTITY;

	ce_scenenode* scenenode = ce_scenenode_new(cookie->terrain->scenenode);
	scenenode->listener = (ce_scenenode_listener) {NULL, NULL, NULL,
		ce_terrain_sector_scenenode_updated, NULL, sector};

	ce_scenenode_add_renderitem(scenenode, sector->renderitem);
}

static void ce_terrain_create_sectors(ce_terrain_cookie* cookie)
{
	ce_logging_write("terrain: creating sectors...");
	ce_logging_write("terrain: this may take some time, please wait...");

	// mpr name + xxxzzz[w]
	char name[cookie->terrain->mprfile->name->length + 3 + 3 + 1 + 1];

	for (int i = 0, j = 0; i < CE_MPRFILE_MATERIAL_COUNT; ++i) {
		bool water = CE_MPRFILE_MATERIAL_WATER == i;
		for (int z = 0; z < cookie->terrain->mprfile->sector_z_count; ++z) {
			for (int x = 0; x < cookie->terrain->mprfile->sector_x_count; ++x) {
				ce_mprsector* sector = cookie->terrain->mprfile->sectors + z *
								cookie->terrain->mprfile->sector_x_count + x;

				if (water && NULL == sector->water_allow) {
					// do not add empty geometry
					continue;
				}

				snprintf(name, sizeof(name), water ? "%s%03d%03dw" :
					"%s%03d%03d", cookie->terrain->mprfile->name->str, x, z);

				ce_terrain_create_sector(cookie, name, j++, x, z, water);
			}
		}
	}
}

ce_terrain* ce_terrain_new(ce_mprfile* mprfile, ce_texmng* texmng,
							ce_renderqueue* renderqueue,
							const ce_vec3* position,
							const ce_quat* orientation,
							ce_scenenode* scenenode)
{
	ce_logging_write("terrain: loading '%s'...", mprfile->name->str);

	ce_terrain* terrain = ce_alloc(sizeof(ce_terrain));
	terrain->mprfile = mprfile;
	terrain->tile_textures = ce_vector_new_reserved(mprfile->texture_count);
	terrain->materials[CE_MPRFILE_MATERIAL_LAND] =
						ce_mprhlp_create_material(mprfile, false);
	terrain->materials[CE_MPRFILE_MATERIAL_WATER] =
						ce_mprhlp_create_material(mprfile, true);
	terrain->sectors = ce_vector_new_reserved(2 * mprfile->sector_x_count *
													mprfile->sector_z_count);
	terrain->scenenode = ce_scenenode_new(scenenode);
	terrain->scenenode->position = *position;
	terrain->scenenode->orientation = *orientation;

	ce_terrain_cookie* cookie = ce_terrain_cookie_new(terrain, texmng, renderqueue);

	if (ce_root.terrain_tiling) {
		// load tile textures immediately, because
		// they are necessary for geometry creation
		ce_terrain_load_tile_textures(cookie);
	}

	ce_terrain_create_sectors(cookie);

	ce_logging_write("terrain: waiting for all tasks to complete...");

	ce_thread_mutex_lock(cookie->mutex);

	while (!ce_vector_empty(cookie->portions) ||
			cookie->completed_portion_count != cookie->queued_portion_count) {
		ce_thread_mutex_unlock(cookie->mutex);

		ce_terrain_load_portions(cookie);
		ce_thread_pool_wait_one(cookie->pool);

		ce_thread_mutex_lock(cookie->mutex);
	}

	ce_thread_mutex_unlock(cookie->mutex);

	ce_terrain_cookie_del(cookie);

	ce_logging_write("terrain: done loading '%s'", mprfile->name->str);

	return terrain;
}

void ce_terrain_del(ce_terrain* terrain)
{
	if (NULL != terrain) {
		ce_scenenode_del(terrain->scenenode);
		ce_vector_for_each(terrain->sectors, ce_terrain_sector_del);
		ce_vector_del(terrain->sectors);
		ce_material_del(terrain->materials[CE_MPRFILE_MATERIAL_WATER]);
		ce_material_del(terrain->materials[CE_MPRFILE_MATERIAL_LAND]);
		ce_vector_for_each(terrain->tile_textures, ce_texture_del);
		ce_vector_del(terrain->tile_textures);
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
