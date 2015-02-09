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

#include <cstdio>
#include <cstring>
#include <vector>

#include "alloc.hpp"
#include "logging.hpp"
#include "path.hpp"
#include "resfile.hpp"
#include "optionmanager.hpp"
#include "mprmanager.hpp"

namespace cursedearth
{
    struct ce_mpr_manager* ce_mpr_manager;

    const char* ce_mpr_dirs[] = { "Maps", NULL };
    const char* ce_mpr_exts[] = { ".mpr", NULL };

    void ce_mpr_manager_init(void)
    {
        std::vector<char> path(option_manager_t::instance()->ei_path2->length + 16);
        for (size_t i = 0; NULL != ce_mpr_dirs[i]; ++i) {
            ce_path_join(path.data(), path.size(), option_manager_t::instance()->ei_path2->str, ce_mpr_dirs[i], NULL);
            ce_logging_info("mpr manager: using path '%s'", path.data());
        }

        ce_mpr_manager = (struct ce_mpr_manager*)ce_alloc_zero(sizeof(struct ce_mpr_manager));
    }

    void ce_mpr_manager_term(void)
    {
        if (NULL != ce_mpr_manager) {
            ce_free(ce_mpr_manager, sizeof(struct ce_mpr_manager));
        }
    }

    ce_mprfile* ce_mpr_manager_open(const char* name)
    {
        std::vector<char> path(option_manager_t::instance()->ei_path2->length + strlen(name) + 32);
        if (NULL == ce_path_find_special1(path.data(), path.size(), option_manager_t::instance()->ei_path2->str, name, ce_mpr_dirs, ce_mpr_exts)) {
            return NULL;
        }

        ce_res_file* res_file = ce_res_file_new_path(path.data());
        if (NULL == res_file) {
            return NULL;
        }

        ce_mprfile* mprfile = ce_mprfile_open(res_file);
        ce_res_file_del(res_file);

        return mprfile;
    }
}
