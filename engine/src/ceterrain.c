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
#include "cethread.h"
#include "cestring.h"
#include "cemprhlp.h"
#include "cemprrenderitem.h"
#include "ceterrain.h"

typedef struct {
	ce_terrain* terrain;
	bool tiling;
	ce_texmng* texmng;
	ce_thread_pool* thread_pool;
	ce_vector* tile_mmpfiles;
	ce_vector* sector_mmpfiles;
	ce_string* sector_name;
	int index;
	int x, z;
	bool water;
} ce_terrain_cookie;

static ce_terrain_cookie* ce_terrain_cookie_new(ce_terrain* terrain,
												bool tiling, ce_texmng* texmng)
{
	ce_terrain_cookie* cookie = ce_alloc(sizeof(ce_terrain_cookie));
	cookie->terrain = terrain;
	cookie->tiling = tiling;
	cookie->texmng = texmng;
	cookie->thread_pool = ce_thread_pool_new(4);
	cookie->tile_mmpfiles = ce_vector_new_reserved(terrain->mprfile->texture_count);
	cookie->sector_mmpfiles = ce_vector_new_reserved(2 *
		terrain->mprfile->sector_x_count * terrain->mprfile->sector_z_count);
	// mpr name + xxxzzz[w]
	cookie->sector_name =
		ce_string_new_reserved(terrain->mprfile->name->length + 3 + 3 + 1 + 1);
	return cookie;
}

static void ce_terrain_cookie_del(ce_terrain_cookie* cookie)
{
	if (NULL != cookie) {
		ce_string_del(cookie->sector_name);
		ce_vector_for_each(cookie->sector_mmpfiles, ce_mmpfile_del);
		ce_vector_for_each(cookie->tile_mmpfiles, ce_mmpfile_del);
		ce_vector_del(cookie->sector_mmpfiles);
		ce_vector_del(cookie->tile_mmpfiles);
		ce_thread_pool_del(cookie->thread_pool);
		ce_free(cookie, sizeof(ce_terrain_cookie));
	}
}

typedef void (*ce_terrain_process_func)(ce_terrain_cookie* cookie);

static void ce_terrain_process(ce_terrain_cookie* cookie,
							ce_terrain_process_func process,
							const char* message)
{
	ce_logging_info("terrain: %s...", message);

	cookie->index = 0;

	for (int i = 0; i < 2; ++i) {
		cookie->water = 1 == i;
		for (int z = 0; z < cookie->terrain->mprfile->sector_z_count; ++z) {
			for (int x = 0; x < cookie->terrain->mprfile->sector_x_count; ++x) {
				ce_mprsector* sector = cookie->terrain->mprfile->sectors + z *
								cookie->terrain->mprfile->sector_x_count + x;

				if (cookie->water && NULL == sector->water_allow) {
					// do not add empty geometry
					continue;
				}

				ce_string_assign_f(cookie->sector_name,
					cookie->water ? "%s%03d%03dw" : "%s%03d%03d",
					cookie->terrain->mprfile->name->str, x, z);

				cookie->x = x;
				cookie->z = z;

				(*process)(cookie);

				++cookie->index;
			}
		}
	}
}

static void ce_terrain_load_tile_mmpfiles(ce_terrain_cookie* cookie)
{
	ce_logging_info("terrain: loading tile mmp files...");

	// mpr name + nnn
	char name[cookie->terrain->mprfile->name->length + 3 + 1];

	for (int i = 0; i < cookie->terrain->mprfile->texture_count; ++i) {
		snprintf(name, sizeof(name), "%s%03d",
				cookie->terrain->mprfile->name->str, i);
		ce_mmpfile* mmpfile = ce_texmng_open_mmpfile(cookie->texmng, name);
		ce_vector_push_back(cookie->tile_mmpfiles, mmpfile);
		ce_thread_pool_enqueue(cookie->thread_pool,
			ce_mmpfile_create_job_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8));
	}

	ce_thread_pool_wait(cookie->thread_pool);
}

static void ce_terrain_load_sector_mmpfile(ce_terrain_cookie* cookie)
{
	ce_mmpfile* mmpfile =
		ce_texmng_open_mmpfile(cookie->texmng, cookie->sector_name->str);
	if (NULL == mmpfile || mmpfile->version < CE_MMPFILE_VERSION ||
							mmpfile->user_info < CE_MPRHLP_MMPFILE_VERSION) {
		ce_mmpfile_del(mmpfile);

		// lazy loading tile mmpfiles
		if (ce_vector_empty(cookie->tile_mmpfiles)) {
			ce_logging_write("terrain: needs to generate "
							"textures for some sectors, please wait...");
			ce_terrain_load_tile_mmpfiles(cookie);
		}

		mmpfile = ce_mprhlp_create_mmpfile(cookie->terrain->mprfile,
												cookie->tile_mmpfiles);

		ce_mprhlp_mmpfile_data data = {
			mmpfile, cookie->terrain->mprfile, cookie->tile_mmpfiles,
			cookie->x, cookie->z, cookie->water
		};

		ce_thread_pool_enqueue(cookie->thread_pool,
			ce_mprhlp_create_job_generate(&data));
	}

	ce_vector_push_back(cookie->sector_mmpfiles, mmpfile);
}

static void ce_terrain_create_sector(ce_terrain_cookie* cookie)
{
	ce_renderitem* renderitem =
		ce_mprrenderitem_new(cookie->terrain->mprfile, cookie->tiling,
			cookie->x, cookie->z, cookie->water, cookie->terrain->tile_textures);

	ce_mprhlp_get_aabb(&renderitem->aabb,
		cookie->terrain->mprfile, cookie->x, cookie->z);

	renderitem->position = CE_VEC3_ZERO;
	renderitem->orientation = CE_QUAT_IDENTITY;
	renderitem->bbox.aabb = renderitem->aabb;
	renderitem->bbox.axis = CE_QUAT_IDENTITY;

	ce_scenenode* scenenode = ce_scenenode_new(cookie->terrain->scenenode);
	ce_scenenode_add_renderitem(scenenode, renderitem);
}

static void ce_terrain_load_tile_textures(ce_terrain_cookie* cookie)
{
	ce_logging_info("terrain: loading tile textures...");

	// mpr name + nnn
	char name[cookie->terrain->mprfile->name->length + 3 + 1];

	for (int i = 0; i < cookie->terrain->mprfile->texture_count; ++i) {
		snprintf(name, sizeof(name), "%s%03d",
				cookie->terrain->mprfile->name->str, i);
		ce_texture* texture = ce_texmng_get(cookie->texmng, name);
		ce_texture_wrap(texture, CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE);
		ce_vector_push_back(cookie->terrain->tile_textures,
			ce_texture_add_ref(texture));
	}
}

static void ce_terrain_load_sector_texture(ce_terrain_cookie* cookie)
{
	ce_texture* texture;

	if (cookie->tiling) {
		texture = ce_texmng_get(cookie->texmng, "default0");
	} else {
		ce_mmpfile* mmpfile = cookie->sector_mmpfiles->items[cookie->index];
		texture = ce_texture_new(cookie->sector_name->str, mmpfile);
		if (!ce_vector_empty(cookie->tile_mmpfiles)) {
			// some mmp files are generated, needs to save
			ce_texmng_save_mmpfile(cookie->texmng,
				cookie->sector_name->str, mmpfile);
		}
		ce_texmng_put(cookie->texmng, texture);
	}

	ce_texture_wrap(texture, CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE);
	ce_vector_push_back(cookie->terrain->sector_textures,
		ce_texture_add_ref(texture));
}

ce_terrain* ce_terrain_new(ce_mprfile* mprfile,
							bool tiling, ce_texmng* texmng,
							const ce_vec3* position,
							const ce_quat* orientation,
							ce_scenenode* scenenode)
{
	ce_terrain* terrain = ce_alloc(sizeof(ce_terrain));
	terrain->mprfile = mprfile;
	terrain->tile_textures = ce_vector_new_reserved(mprfile->texture_count);
	terrain->sector_textures = ce_vector_new_reserved(2 *
		mprfile->sector_x_count * mprfile->sector_z_count);
	terrain->materials[0] = ce_mprhlp_create_material(mprfile, false);
	terrain->materials[1] = ce_mprhlp_create_material(mprfile, true);
	terrain->scenenode = ce_scenenode_new(scenenode);
	terrain->scenenode->position = *position;
	terrain->scenenode->orientation = *orientation;

	ce_logging_write("terrain: loading '%s'...", mprfile->name->str);

	ce_terrain_cookie* cookie = ce_terrain_cookie_new(terrain, tiling, texmng);

	tiling ? ce_terrain_load_tile_textures(cookie) :
			ce_terrain_process(cookie, ce_terrain_load_sector_mmpfile,
											"loading sector mmp files");

	ce_terrain_process(cookie, ce_terrain_create_sector, "creating sectors");

	ce_logging_info("terrain: waiting for other tasks...");
	ce_thread_pool_wait(cookie->thread_pool);

	ce_terrain_process(cookie, ce_terrain_load_sector_texture,
									"loading sector textures");

	ce_terrain_cookie_del(cookie);

	return terrain;
}

void ce_terrain_del(ce_terrain* terrain)
{
	if (NULL != terrain) {
		ce_vector_for_each(terrain->sector_textures, ce_texture_del);
		ce_vector_for_each(terrain->tile_textures, ce_texture_del);
		ce_scenenode_del(terrain->scenenode);
		ce_material_del(terrain->materials[1]);
		ce_material_del(terrain->materials[0]);
		ce_vector_del(terrain->sector_textures);
		ce_vector_del(terrain->tile_textures);
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
