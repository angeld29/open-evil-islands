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

#include "lib.hpp"
#include "str.hpp"
#include "path.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "optionmanager.hpp"
#include "resourcemanager.hpp"

namespace cursedearth
{
struct ce_resource_manager* ce_resource_manager;

static const char* ce_resource_dirs[] = {"Res", NULL};
static const char* ce_resource_exts[] = {".res", NULL};

static ce_res_file* ce_resource_manager_open(const char* name)
{
    char path[ce_option_manager->ei_path->length + 32];
    ce_res_file* res_file = NULL;

    if (NULL != ce_path_find_special1(path, sizeof(path),
            ce_option_manager->ei_path->str, name,
            ce_resource_dirs, ce_resource_exts) &&
            NULL != (res_file = ce_res_file_new_path(path))) {
        ce_logging_write("resource manager: loading '%s'... ok", path);
    } else {
        ce_logging_error("resource manager: loading '%s'... failed", path);
    }

    return res_file;
}

void ce_resource_manager_init(void)
{
    char path[ce_option_manager->ei_path->length + 32];
    ce_path_join(path, sizeof(path), ce_option_manager->ei_path->str, ce_resource_dirs[0], NULL);

    ce_resource_manager = ce_alloc_zero(sizeof(struct ce_resource_manager));
    ce_resource_manager->path = ce_string_new_str(path);

    for (size_t i = 0; NULL != ce_resource_dirs[i]; ++i) {
        ce_path_join(path, sizeof(path), ce_option_manager->ei_path->str, ce_resource_dirs[i], NULL);
        ce_logging_write("resource manager: using path '%s'", path);
    }

    ce_resource_manager->database = ce_resource_manager_open("database");
    ce_resource_manager->menus = ce_resource_manager_open("menus");
}

void ce_resource_manager_term(void)
{
    if (NULL != ce_resource_manager) {
        ce_res_file_del(ce_resource_manager->menus);
        ce_res_file_del(ce_resource_manager->database);
        ce_string_del(ce_resource_manager->path);
        ce_free(ce_resource_manager, sizeof(struct ce_resource_manager));
    }
}

size_t ce_resource_manager_find_data(const char* path)
{
    for (size_t index = 0; index < CE_RESOURCE_DATA_COUNT; ++index) {
        if (0 == strcmp(path, ce_resource_data_paths[index])) {
            return index;
        }
    }
    return CE_RESOURCE_DATA_COUNT;
}
}
