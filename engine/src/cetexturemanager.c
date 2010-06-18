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
#include <string.h>
#include <assert.h>

#include "cealloc.h"
#include "celogging.h"
#include "cestr.h"
#include "cepath.h"
#include "ceresfile.h"
#include "ceoptionmanager.h"
#include "cetexturemanager.h"

struct ce_texture_manager* ce_texture_manager;

static const char* ce_texture_cache_dirs[] = {"Textures", NULL};
static const char* ce_texture_cache_exts[] = {".mmp", NULL};
static const char* ce_texture_resource_dirs[] = {"Res", NULL};
static const char* ce_texture_resource_exts[] = {".res", NULL};
static const char* ce_texture_resource_names[] = {"textures", "redress", "menus", NULL};

void ce_texture_manager_init(void)
{
	char path[ce_option_manager->ei_path->length + 32];

	for (size_t i = 0; NULL != ce_texture_cache_dirs[i]; ++i) {
		ce_path_join(path, sizeof(path),
			ce_option_manager->ei_path->str, ce_texture_cache_dirs[i], NULL);
		ce_logging_write("texture manager: using cache path '%s'", path);
	}

	for (size_t i = 0; NULL != ce_texture_resource_dirs[i]; ++i) {
		ce_path_join(path, sizeof(path),
			ce_option_manager->ei_path->str, ce_texture_resource_dirs[i], NULL);
		ce_logging_write("texture manager: using path '%s'", path);
	}

	ce_texture_manager = ce_alloc_zero(sizeof(struct ce_texture_manager));
	ce_texture_manager->resfiles = ce_vector_new();
	ce_texture_manager->textures = ce_vector_new();

	for (size_t i = 0; NULL != ce_texture_resource_names[i]; ++i) {
		ce_resfile* resfile;
		if (NULL != ce_path_find_special1(path, sizeof(path),
				ce_option_manager->ei_path->str, ce_texture_resource_names[i],
				ce_texture_resource_dirs, ce_texture_resource_exts) &&
				NULL != (resfile = ce_resfile_open_file(path))) {
			ce_vector_push_back(ce_texture_manager->resfiles, resfile);
			ce_logging_write("texture manager: loading '%s'... ok", path);
		} else {
			ce_logging_error("texture manager: loading '%s'... failed", path);
		}
	}
}

void ce_texture_manager_term(void)
{
	if (NULL != ce_texture_manager) {
		ce_vector_for_each(ce_texture_manager->textures, ce_texture_del);
		ce_vector_del(ce_texture_manager->textures);
		ce_vector_for_each(ce_texture_manager->resfiles, ce_resfile_close);
		ce_vector_del(ce_texture_manager->resfiles);
		ce_free(ce_texture_manager, sizeof(struct ce_texture_manager));
	}
}

ce_mmpfile* ce_texture_manager_open_mmpfile(const char* name)
{
	char path[ce_option_manager->ei_path->length + strlen(name) + 32];

	// try to load from cache dir
	if (NULL != ce_path_find_special1(path, sizeof(path),
					ce_option_manager->ei_path->str, name,
					ce_texture_cache_dirs, ce_texture_cache_exts)) {
		ce_memfile* memfile = ce_memfile_open_path(path);
		if (NULL != memfile) {
			ce_mmpfile* mmpfile = ce_mmpfile_new_memfile(memfile);
			ce_memfile_close(memfile);
			if (NULL != mmpfile) {
				return mmpfile;
			}
		}
	}

	char file_name[strlen(name) + 8];
	ce_path_append_ext(file_name, sizeof(file_name),
						name, ce_texture_cache_exts[0]);

	// find in resources
	for (size_t i = 0; i < ce_texture_manager->resfiles->count; ++i) {
		ce_resfile* resfile = ce_texture_manager->resfiles->items[i];
		int index = ce_resfile_node_index(resfile, file_name);
		if (-1 != index) {
			return ce_mmpfile_new_resfile(resfile, index);
		}
	}

	return NULL;
}

void ce_texture_manager_save_mmpfile(const char* name, ce_mmpfile* mmpfile)
{
	char file_name[strlen(name) + 8];
	ce_path_append_ext(file_name, sizeof(file_name),
						name, ce_texture_cache_exts[0]);

	char path[ce_option_manager->ei_path->length + strlen(file_name) + 32];
	ce_path_join(path, sizeof(path), ce_option_manager->
		ei_path->str, ce_texture_cache_dirs[0], file_name, NULL);

	ce_mmpfile_save(mmpfile, path);
}

ce_texture* ce_texture_manager_get(const char* name)
{
	char file_name[strlen(name) + 8];
	ce_path_append_ext(file_name, sizeof(file_name),
						name, ce_texture_cache_exts[0]);

	// find texture in cache
	for (size_t i = 0; i < ce_texture_manager->textures->count; ++i) {
		ce_texture* texture = ce_texture_manager->textures->items[i];
		if (0 == ce_strcasecmp(file_name, texture->name->str)) {
			return texture;
		}
	}

	// load texture from resources
	ce_mmpfile* mmpfile = ce_texture_manager_open_mmpfile(name);
	if (NULL != mmpfile) {
		ce_texture* texture = ce_texture_new(file_name, mmpfile);
		ce_mmpfile_del(mmpfile);
		ce_vector_push_back(ce_texture_manager->textures, texture);
		return texture;
	}

	return NULL;
}

void ce_texture_manager_put(ce_texture* texture)
{
	ce_vector_push_back(ce_texture_manager->textures, texture);
}
