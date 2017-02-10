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
#include "mobmanager.hpp"

namespace cursedearth
{
    namespace fs = boost::filesystem;

    struct ce_mob_manager* ce_mob_manager;

    const std::vector<std::string> ce_mob_dirs = { "Maps" };
    const std::vector<std::string> ce_mob_exts = { ".mob" };

    void ce_mob_manager_init(void)
    {
        for (const auto& dir: ce_mob_dirs) {
            fs::path path = option_manager_t::instance()->ei_path() / dir;
            ce_logging_info("mob manager: using path `%s'", path.string().c_str());
        }
        ce_mob_manager = (struct ce_mob_manager*)ce_alloc_zero(sizeof(struct ce_mob_manager));
    }

    void ce_mob_manager_term(void)
    {
        if (NULL != ce_mob_manager) {
            ce_free(ce_mob_manager, sizeof(struct ce_mob_manager));
        }
    }

    fs::path find_mob_resource(const std::string& name)
    {
        const fs::path root = option_manager_t::instance()->ei_path();
        for (const auto& extension: ce_mob_exts) {
            const fs::path file_name = name + extension;
            for (const auto& directory: ce_mob_dirs) {
                const fs::path file_path = root / directory / file_name;
                if (exists(file_path)) {
                    return file_path;
                }
            }
        }
        return fs::path();
    }

    ce_mob_file* ce_mob_manager_open(const std::string& name)
    {
        fs::path path = find_mob_resource(name);
        if (!path.empty()) {
            return ce_mob_file_open(path);
        }
        return NULL;
    }
}
