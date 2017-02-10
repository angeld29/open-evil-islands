/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <vector>

#include <boost/filesystem.hpp>

#include "alloc.hpp"
#include "logging.hpp"
#include "resfile.hpp"
#include "optionmanager.hpp"
#include "texturemanager.hpp"

namespace cursedearth
{
    namespace fs = boost::filesystem;

    struct ce_texture_manager* ce_texture_manager;

    const std::vector<std::string> ce_texture_exts = { ".mmp" };
    const std::vector<std::string> ce_texture_cache_dirs = { "Textures" };
    const std::vector<std::string> ce_texture_resource_dirs = { "Res" };
    const std::vector<std::string> ce_texture_resource_exts = { ".res" };
    const std::vector<std::string> ce_texture_resource_names = { "textures", "redress", "menus" };

    fs::path find_cache_resource(const std::string& name)
    {
        const fs::path root = option_manager_t::instance()->ei_path();
        for (const auto& extension: ce_texture_exts) {
            const fs::path file_name = name + extension;
            for (const auto& directory: ce_texture_cache_dirs) {
                const fs::path file_path = root / directory / file_name;
                if (exists(file_path)) {
                    return file_path;
                }
            }
        }
        return fs::path();
    }

    fs::path find_texture_resource(const std::string& name)
    {
        const fs::path root = option_manager_t::instance()->ei_path();
        for (const auto& extension: ce_texture_resource_exts) {
            const fs::path file_name = name + extension;
            for (const auto& directory: ce_texture_resource_dirs) {
                const fs::path file_path = root / directory / file_name;
                if (exists(file_path)) {
                    return file_path;
                }
            }
        }
        return fs::path();
    }

    void ce_texture_manager_init()
    {
        ce_texture_manager = (struct ce_texture_manager*)ce_alloc_zero(sizeof(struct ce_texture_manager));
        ce_texture_manager->res_files = ce_vector_new();
        ce_texture_manager->textures = ce_vector_new();

        for (const auto& dir: ce_texture_cache_dirs) {
            fs::path path = option_manager_t::instance()->ei_path() / dir;
            ce_logging_info("texture manager: using cache path `%s'", path.string().c_str());
        }

        for (const auto& dir: ce_texture_resource_dirs) {
            fs::path path = option_manager_t::instance()->ei_path() / dir;
            ce_logging_info("texture manager: using path `%s'", path.string().c_str());
        }

        for (const auto& name: ce_texture_resource_names) {
            fs::path path = find_texture_resource(name);
            ce_res_file* res_file;
            if (!path.empty() && NULL != (res_file = ce_res_file_new_path(path))) {
                ce_vector_push_back(ce_texture_manager->res_files, res_file);
                ce_logging_info("texture manager: loading `%s'... ok", path.string().c_str());
            } else {
                ce_logging_error("texture manager: loading `%s'... failed", path.string().c_str());
            }
        }
    }

    void ce_texture_manager_term()
    {
        if (NULL != ce_texture_manager) {
            ce_vector_for_each(ce_texture_manager->textures, (void(*)(void*))ce_texture_del);
            ce_vector_del(ce_texture_manager->textures);
            ce_vector_for_each(ce_texture_manager->res_files, (void(*)(void*))ce_res_file_del);
            ce_vector_del(ce_texture_manager->res_files);
            ce_free(ce_texture_manager, sizeof(struct ce_texture_manager));
        }
    }

    ce_mmpfile* ce_texture_manager_open_mmpfile_from_cache(const std::string& name)
    {
        fs::path path = find_cache_resource(name);
        if (!path.empty()) {
            ce_mem_file* mem_file = ce_mem_file_new_path(path);
            if (NULL != mem_file) {
                ce_mmpfile* mmpfile = ce_mmpfile_new_mem_file(mem_file);
                ce_mem_file_del(mem_file);
                return mmpfile;
            }
        }

        return NULL;
    }

    ce_mmpfile* ce_texture_manager_open_mmpfile_from_resources(const std::string& name)
    {
        std::string file_name = name + ce_texture_exts[0];

        // find in resources
        for (size_t i = 0; i < ce_texture_manager->res_files->count; ++i) {
            ce_res_file* res_file = (ce_res_file*)ce_texture_manager->res_files->items[i];
            size_t index = ce_res_file_node_index(res_file, file_name.c_str());
            if (res_file->node_count != index) {
                ce_mmpfile* mmpfile = ce_mmpfile_new_res_file(res_file, index);
                return mmpfile;
            }
        }

        return NULL;
    }

    ce_mmpfile* ce_texture_manager_open_mmpfile(const std::string& name)
    {
        ce_mmpfile* mmpfile = ce_texture_manager_open_mmpfile_from_cache(name.c_str());
        if (NULL == mmpfile) {
            mmpfile = ce_texture_manager_open_mmpfile_from_resources(name.c_str());
        }
        return mmpfile;
    }

    void ce_texture_manager_save_mmpfile(const std::string& name, ce_mmpfile* mmpfile)
    {
        std::string file_name = name + ce_texture_exts[0];
        fs::path path = option_manager_t::instance()->ei_path() / ce_texture_cache_dirs[0] / file_name;
        ce_mmpfile_save(mmpfile, path);
    }

    ce_texture* ce_texture_manager_get(const std::string& name)
    {
        std::string base_name = name.substr(0, name.find_last_of("."));

        // find texture in cache
        for (size_t i = 0; i < ce_texture_manager->textures->count; ++i) {
            ce_texture* texture = (ce_texture*)ce_texture_manager->textures->items[i];
            if (base_name == texture->name->str) {
                return texture;
            }
        }

        // load texture from resources
        ce_mmpfile* mmpfile = ce_texture_manager_open_mmpfile(name.c_str());
        if (NULL != mmpfile) {
            ce_texture* texture = ce_texture_new(base_name.c_str(), mmpfile);
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
}
