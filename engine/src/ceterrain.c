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

#include "celib.h"
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
	ce_thread_pool* pool;
	ce_thread_mutex* mutex;
	ce_vector* tile_mmpfiles;
} ce_terrain_cookie;

static ce_terrain_cookie* ce_terrain_cookie_new(ce_terrain* terrain,
	bool tiling, ce_texmng* texmng, int thread_count)
{
	ce_terrain_cookie* cookie = ce_alloc(sizeof(ce_terrain_cookie));
	cookie->terrain = terrain;
	cookie->tiling = tiling;
	cookie->texmng = texmng;
	cookie->pool = ce_thread_pool_new(thread_count);
	cookie->mutex = ce_thread_mutex_new();
	cookie->tile_mmpfiles = ce_vector_new_reserved(terrain->mprfile->texture_count);
	return cookie;
}

static void ce_terrain_cookie_del(ce_terrain_cookie* cookie)
{
	if (NULL != cookie) {
		ce_vector_for_each(cookie->tile_mmpfiles, ce_mmpfile_del);
		ce_vector_del(cookie->tile_mmpfiles);
		ce_thread_mutex_del(cookie->mutex);
		ce_thread_pool_del(cookie->pool);
		ce_free(cookie, sizeof(ce_terrain_cookie));
	}
}

static void ce_terrain_load_tile_resources(ce_terrain_cookie* cookie, bool textures)
{
	ce_logging_info("terrain: loading tile %s...",
		textures ? "textures" : "mmp files");

	// mpr name + nnn
	char name[cookie->terrain->mprfile->name->length + 3 + 1];

	for (int i = 0; i < cookie->terrain->mprfile->texture_count; ++i) {
		snprintf(name, sizeof(name), "%s%03d",
				cookie->terrain->mprfile->name->str, i);
		if (textures) {
			ce_texture* texture = ce_texmng_get(cookie->texmng, name);
			ce_texture_wrap(texture, CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE);
			ce_vector_push_back(cookie->terrain->tile_textures,
				ce_texture_add_ref(texture));
		} else {
			ce_mmpfile* mmpfile = ce_texmng_open_mmpfile(cookie->texmng, name);
			ce_mmpfile_convert(mmpfile, CE_MMPFILE_FORMAT_R8G8B8A8);
			ce_vector_push_back(cookie->tile_mmpfiles, mmpfile);
		}
	}
}

typedef struct {
	ce_terrain_cookie* cookie;
	ce_string* name;
	int index;
	int x, z;
	bool water;
	ce_mmpfile* mmpfile;
} ce_terrain_job;

static void ce_terrain_job_ctor(ce_thread_job* base_job, va_list args)
{
	ce_terrain_job* job = (ce_terrain_job*)base_job->impl;

	job->cookie = va_arg(args, ce_terrain_cookie*);
	job->name = ce_string_new_str(va_arg(args, const char*));
	job->index = va_arg(args, int);
	job->x = va_arg(args, int);
	job->z = va_arg(args, int);
	job->water = va_arg(args, int);
	job->mmpfile = NULL;
}

static void ce_terrain_job_dtor(ce_thread_job* base_job)
{
	ce_terrain_job* job = (ce_terrain_job*)base_job->impl;

	ce_mmpfile_del(job->mmpfile);
	ce_string_del(job->name);
}

static void ce_terrain_job_exec(ce_thread_job* base_job)
{
	ce_terrain_job* job = (ce_terrain_job*)base_job->impl;

	ce_thread_mutex_lock(job->cookie->mutex);
	job->mmpfile = ce_texmng_open_mmpfile(job->cookie->texmng, job->name->str);
	ce_thread_mutex_unlock(job->cookie->mutex);

	if (NULL == job->mmpfile || job->mmpfile->version < CE_MMPFILE_VERSION ||
						job->mmpfile->user_info < CE_MPRHLP_MMPFILE_VERSION) {
		// lazy loading tile mmpfiles
		if (ce_vector_empty(job->cookie->tile_mmpfiles)) {
			// FIXME: wake double-checked locking, pthread_once ?
			ce_thread_mutex_lock(job->cookie->mutex);
			if (ce_vector_empty(job->cookie->tile_mmpfiles)) {
				ce_logging_write("terrain: needs to generate "
								"textures for some sectors, please wait...");
				ce_terrain_load_tile_resources(job->cookie, false);
			}
			ce_thread_mutex_unlock(job->cookie->mutex);
		}

		ce_mmpfile_del(job->mmpfile);
		job->mmpfile = ce_mprhlp_generate_mmpfile(job->cookie->terrain->mprfile,
			job->cookie->tile_mmpfiles, job->x, job->z, job->water);

		// force to dxt1?
		ce_mmpfile_convert(job->mmpfile, CE_MMPFILE_FORMAT_DXT1);

		ce_texmng_save_mmpfile(job->cookie->texmng,
								job->name->str, job->mmpfile);
	}
}

static void ce_terrain_job_post(ce_thread_job* base_job, va_list args)
{
	ce_terrain_job* job = (ce_terrain_job*)base_job->impl;
	ce_unused(args);

	// all operations with textures must be performed in initial thread
	ce_texture* texture = ce_texture_new(job->name->str, job->mmpfile);
	ce_texture_wrap(texture, CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE);

	ce_texmng_put(job->cookie->texmng, ce_texture_add_ref(texture));
	job->cookie->terrain->sector_textures->items[job->index] = texture;
}

static const ce_thread_job_vtable ce_terrain_job_vtable = {
	ce_terrain_job_ctor, ce_terrain_job_dtor,
	ce_terrain_job_exec, ce_terrain_job_post
};

static void ce_terrain_create_sector(ce_terrain_cookie* cookie,
	const char* name, int index, int x, int z, bool water)
{
	if (cookie->tiling) {
		ce_texture* texture = ce_texmng_get(cookie->texmng, "default0");
		ce_texture_wrap(texture, CE_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE);
		cookie->terrain->sector_textures->items[index] = ce_texture_add_ref(texture);
	} else {
		// enqueue mmp file loading or generation
		ce_thread_pool_enqueue(cookie->pool,
			ce_thread_job_new(ce_terrain_job_vtable, sizeof(ce_terrain_job),
				cookie, name, index, x, z, (int)water));
	}

	// create geometry
	ce_renderitem* renderitem =
		ce_mprrenderitem_new(cookie->terrain->mprfile,
			cookie->tiling, x, z, water, cookie->terrain->tile_textures);

	ce_mprhlp_get_aabb(&renderitem->aabb, cookie->terrain->mprfile, x, z);

	renderitem->position = CE_VEC3_ZERO;
	renderitem->orientation = CE_QUAT_IDENTITY;
	renderitem->bbox.aabb = renderitem->aabb;
	renderitem->bbox.axis = CE_QUAT_IDENTITY;

	ce_scenenode* scenenode = ce_scenenode_new(cookie->terrain->scenenode);
	ce_scenenode_add_renderitem(scenenode, renderitem);
}

static void ce_terrain_create_sectors(ce_terrain_cookie* cookie)
{
	ce_logging_info("terrain: creating sectors...");

	// mpr name + xxxzzz[w]
	char name[cookie->terrain->mprfile->name->length + 3 + 3 + 1 + 1];

	for (int i = 0, j = 0; i < 2; ++i) {
		bool water = 1 == i;
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

ce_terrain* ce_terrain_new(ce_mprfile* mprfile, bool tiling,
							ce_texmng* texmng, int thread_count,
							const ce_vec3* position,
							const ce_quat* orientation,
							ce_scenenode* scenenode)
{
	ce_logging_write("terrain: loading '%s'...", mprfile->name->str);

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

	ce_vector_resize(terrain->sector_textures, 2 *
		mprfile->sector_x_count * mprfile->sector_z_count);

	ce_terrain_cookie* cookie =
		ce_terrain_cookie_new(terrain, tiling, texmng, thread_count);

	if (tiling) {
		// load tile textures immediately, because
		// they are necessary for geometry creation
		ce_terrain_load_tile_resources(cookie, true);
	}

	ce_terrain_create_sectors(cookie);

	ce_logging_write("terrain: waiting for all tasks to complete...");
	ce_logging_write("terrain: this may take some time, please wait...");

	for (;;) {
		ce_thread_pool_wait_completed(cookie->pool);
		if (!ce_thread_pool_has_completed(cookie->pool)) {
			break;
		}
		ce_thread_job* job = ce_thread_pool_take_completed(cookie->pool);
		ce_thread_job_post(job);
		ce_thread_job_del(job);
	}

	ce_terrain_cookie_del(cookie);

	ce_logging_write("terrain: done loading '%s'", mprfile->name->str);

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
