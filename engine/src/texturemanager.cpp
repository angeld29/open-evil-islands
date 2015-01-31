/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

static const char* ce_texture_exts[] = {".mmp", NULL};
static const char* ce_texture_cache_dirs[] = {"Textures", NULL};
static const char* ce_texture_resource_dirs[] = {"Res", NULL};
static const char* ce_texture_resource_exts[] = {".res", NULL};
static const char* ce_texture_resource_names[] = {"textures", "redress", "menus", NULL};

void ce_texture_manager_init(void)
{
    ce_texture_manager = ce_alloc_zero(sizeof(struct ce_texture_manager));
    ce_texture_manager->res_files = ce_vector_new();
    ce_texture_manager->textures = ce_vector_new();

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

    for (size_t i = 0; NULL != ce_texture_resource_names[i]; ++i) {
        ce_res_file* res_file;
        if (NULL != ce_path_find_special1(path, sizeof(path),
                ce_option_manager->ei_path->str, ce_texture_resource_names[i],
                ce_texture_resource_dirs, ce_texture_resource_exts) &&
                NULL != (res_file = ce_res_file_new_path(path))) {
            ce_vector_push_back(ce_texture_manager->res_files, res_file);
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
        ce_vector_for_each(ce_texture_manager->res_files, ce_res_file_del);
        ce_vector_del(ce_texture_manager->res_files);
        ce_free(ce_texture_manager, sizeof(struct ce_texture_manager));
    }
}

ce_mmpfile* ce_texture_manager_open_mmpfile_from_cache(const char* name)
{
    char path[ce_option_manager->ei_path->length + strlen(name) + 32];

    if (NULL != ce_path_find_special1(path, sizeof(path), ce_option_manager->
            ei_path->str, name, ce_texture_cache_dirs, ce_texture_exts)) {
        ce_mem_file* mem_file = ce_mem_file_new_path(path);
        if (NULL != mem_file) {
            ce_mmpfile* mmpfile = ce_mmpfile_new_mem_file(mem_file);
            ce_mem_file_del(mem_file);
            return mmpfile;
        }
    }

    return NULL;
}

ce_mmpfile* ce_texture_manager_open_mmpfile_from_resources(const char* name)
{
    char file_name[strlen(name) + 8];
    ce_path_append_ext(file_name, sizeof(file_name), name, ce_texture_exts[0]);

    // find in resources
    for (size_t i = 0; i < ce_texture_manager->res_files->count; ++i) {
        ce_res_file* res_file = ce_texture_manager->res_files->items[i];
        size_t index = ce_res_file_node_index(res_file, file_name);
        if (res_file->node_count != index) {
            ce_mmpfile* mmpfile = ce_mmpfile_new_res_file(res_file, index);
            return mmpfile;
        }
    }

    return NULL;
}

ce_mmpfile* ce_texture_manager_open_mmpfile(const char* name)
{
    ce_mmpfile* mmpfile = ce_texture_manager_open_mmpfile_from_cache(name);
    if (NULL == mmpfile) {
        mmpfile = ce_texture_manager_open_mmpfile_from_resources(name);
    }
    return mmpfile;
}

void ce_texture_manager_save_mmpfile(const char* name, ce_mmpfile* mmpfile)
{
    char file_name[strlen(name) + 8];
    ce_path_append_ext(file_name, sizeof(file_name), name, ce_texture_exts[0]);

    char path[ce_option_manager->ei_path->length + strlen(file_name) + 32];
    ce_path_join(path, sizeof(path), ce_option_manager->
        ei_path->str, ce_texture_cache_dirs[0], file_name, NULL);

    ce_mmpfile_save(mmpfile, path);
}

ce_texture* ce_texture_manager_get(const char* name)
{
    char base_name[strlen(name) + 1];
    ce_path_remove_ext(base_name, name);

    // find texture in cache
    for (size_t i = 0; i < ce_texture_manager->textures->count; ++i) {
        ce_texture* texture = ce_texture_manager->textures->items[i];
        if (0 == ce_strcasecmp(base_name, texture->name->str)) {
            return texture;
        }
    }

    // load texture from resources
    ce_mmpfile* mmpfile = ce_texture_manager_open_mmpfile(name);
    if (NULL != mmpfile) {
        ce_texture* texture = ce_texture_new(base_name, mmpfile);
        ce_mmpfile_del(mmpfile);
        ce_texture_manager_put(texture);
        return texture;
    }

    return NULL;
}

void ce_texture_manager_put(ce_texture* texture)
{
    ce_vector_push_back(ce_texture_manager->textures, texture);
}
