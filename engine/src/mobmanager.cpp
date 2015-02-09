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
#include "optionmanager.hpp"
#include "mobmanager.hpp"

namespace cursedearth
{
    struct ce_mob_manager* ce_mob_manager;

    const char* ce_mob_dirs[] = { "Maps", NULL };
    const char* ce_mob_exts[] = { ".mob", NULL };

    void ce_mob_manager_init(void)
    {
        std::vector<char> path(option_manager_t::instance()->ei_path2->length + 16);
        for (size_t i = 0; NULL != ce_mob_dirs[i]; ++i) {
            ce_path_join(path.data(), path.size(), option_manager_t::instance()->ei_path2->str, ce_mob_dirs[i], NULL);
            ce_logging_info("mob manager: using path `%s'", path.data());
        }

        ce_mob_manager = (struct ce_mob_manager*)ce_alloc_zero(sizeof(struct ce_mob_manager));
    }

    void ce_mob_manager_term(void)
    {
        if (NULL != ce_mob_manager) {
            ce_free(ce_mob_manager, sizeof(struct ce_mob_manager));
        }
    }

    ce_mob_file* ce_mob_manager_open(const char* name)
    {
        std::vector<char> path(option_manager_t::instance()->ei_path2->length + strlen(name) + 32);
        if (NULL != ce_path_find_special1(path.data(), path.size(), option_manager_t::instance()->ei_path2->str, name, ce_mob_dirs, ce_mob_exts)) {
            return ce_mob_file_open(path.data());
        }
        return NULL;
    }
}
