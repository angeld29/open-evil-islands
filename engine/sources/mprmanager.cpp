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
#include "mprmanager.hpp"

namespace cursedearth
{
    namespace fs = boost::filesystem;

    struct ce_mpr_manager* ce_mpr_manager;

    const std::vector<std::string> ce_mpr_dirs = { "Maps" };
    const std::vector<std::string> ce_mpr_exts = { ".mpr" };

    void ce_mpr_manager_init()
    {
        for (const auto& dir: ce_mpr_dirs) {
            fs::path path = option_manager_t::instance()->ei_path() / dir;
            ce_logging_info("mpr manager: using path '%s'", path.string().c_str());
        }
        ce_mpr_manager = (struct ce_mpr_manager*)ce_alloc_zero(sizeof(struct ce_mpr_manager));
    }

    void ce_mpr_manager_term()
    {
        if (NULL != ce_mpr_manager) {
            ce_free(ce_mpr_manager, sizeof(struct ce_mpr_manager));
        }
    }

    fs::path find_mpr_resource(const std::string& name)
    {
        const fs::path root = option_manager_t::instance()->ei_path();
        for (const auto& extension: ce_mpr_exts) {
            const fs::path file_name = name + extension;
            for (const auto& directory: ce_mpr_dirs) {
                const fs::path file_path = root / directory / file_name;
                if (exists(file_path)) {
                    return file_path;
                }
            }
        }
        return fs::path();
    }

    ce_mprfile* ce_mpr_manager_open(const std::string& name)
    {
        fs::path path = find_mpr_resource(name);
        if (path.empty()) {
            return NULL;
        }

        ce_res_file* res_file = ce_res_file_new_path(path);
        if (NULL == res_file) {
            return NULL;
        }

        ce_mprfile* mprfile = ce_mprfile_open(res_file);
        ce_res_file_del(res_file);

        return mprfile;
    }
}
