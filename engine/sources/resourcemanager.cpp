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
#include "optionmanager.hpp"
#include "resourcemanager.hpp"

namespace cursedearth
{
    namespace fs = boost::filesystem;

    struct ce_resource_manager* ce_resource_manager;

    const std::vector<std::string> ce_resource_dirs = { "Res" };
    const std::vector<std::string> ce_resource_exts = { ".res" };

    fs::path find_resource_resource(const std::string& name)
    {
        const fs::path root = option_manager_t::instance()->ei_path();
        for (const auto& extension: ce_resource_exts) {
            const fs::path file_name = name + extension;
            for (const auto& directory: ce_resource_dirs) {
                const fs::path file_path = root / directory / file_name;
                if (exists(file_path)) {
                    return file_path;
                }
            }
        }
        return fs::path();
    }

    ce_res_file* ce_resource_manager_open(const std::string& name)
    {
        fs::path path = find_resource_resource(name);
        ce_res_file* res_file = NULL;

        if (!path.empty() && NULL != (res_file = ce_res_file_new_path(path))) {
            ce_logging_info("resource manager: loading `%s'... ok", path.string().c_str());
        } else {
            ce_logging_error("resource manager: loading `%s'... failed", path.string().c_str());
        }

        return res_file;
    }

    void ce_resource_manager_init()
    {
        ce_resource_manager = (struct ce_resource_manager*)ce_alloc_zero(sizeof(struct ce_resource_manager));
        for (const auto& directory: ce_resource_dirs) {
            fs::path path = option_manager_t::instance()->ei_path() / directory;
            ce_logging_info("resource manager: using path `%s'", path.string().c_str());
        }
        ce_resource_manager->database = ce_resource_manager_open("database");
        ce_resource_manager->menus = ce_resource_manager_open("menus");
    }

    void ce_resource_manager_term()
    {
        if (NULL != ce_resource_manager) {
            ce_res_file_del(ce_resource_manager->menus);
            ce_res_file_del(ce_resource_manager->database);
            ce_free(ce_resource_manager, sizeof(struct ce_resource_manager));
        }
    }

    size_t ce_resource_manager_find_data(const std::string& path)
    {
        for (size_t index = 0; index < CE_RESOURCE_DATA_COUNT; ++index) {
            if (path == ce_resource_data_paths[index]) {
                return index;
            }
        }
        return CE_RESOURCE_DATA_COUNT;
    }
}
