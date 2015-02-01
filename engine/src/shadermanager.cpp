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
#include <cassert>

#include "str.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "resourcemanager.hpp"
#include "shadermanager.hpp"

namespace cursedearth
{
struct ce_shader_manager* ce_shader_manager;

void ce_shader_manager_init(void)
{
    ce_shader_manager = ce_alloc_zero(sizeof(struct ce_shader_manager));
    ce_shader_manager->shaders = ce_vector_new();

    if (ce_shader_is_available()) {
        ce_logging_write("shader manager: you have shader support");
    } else {
        ce_logging_warning("shader manager: shader support is not available");
    }
}

void ce_shader_manager_term(void)
{
    if (NULL != ce_shader_manager) {
        ce_vector_for_each(ce_shader_manager->shaders, ce_shader_del);
        ce_vector_del(ce_shader_manager->shaders);
        ce_free(ce_shader_manager, sizeof(struct ce_shader_manager));
    }
}

shader_t* ce_shader_manager_get(const char* resource_paths[])
{
    size_t name_length = 0;
    size_t resource_count = 0;

    for (size_t i = 0; NULL != resource_paths[i]; ++i) {
        name_length += strlen(resource_paths[i]);
        ++resource_count;
    }

    char name[name_length + 1];
    name[0] = '\0';

    for (size_t i = 0; i < resource_count; ++i) {
        ce_strlcat(name, resource_paths[i], sizeof(name));
    }

    // find shader in cache
    for (size_t i = 0; i < ce_shader_manager->shaders->count; ++i) {
        shader_t* shader = ce_shader_manager->shaders->items[i];
        if (0 == strcmp(name, shader->name->str)) {
            return shader;
        }
    }

    shader_info_t shader_infos[resource_count + 1];

    for (size_t i = 0; i < resource_count; ++i) {
        if (NULL != strstr(resource_paths[i], ".vert")) {
            shader_infos[i].shader_type = SHADER_TYPE_VERTEX;
        } else if (NULL != strstr(resource_paths[i], ".frag")) {
            shader_infos[i].shader_type = SHADER_TYPE_FRAGMENT;
        } else {
            shader_infos[i].shader_type = SHADER_TYPE_UNKNOWN;
        }
        shader_infos[i].resource_index = ce_resource_manager_find_data(resource_paths[i]);
        assert(shader_infos[i].resource_index < CE_RESOURCE_DATA_COUNT);
    }

    // add guard element
    shader_infos[resource_count].shader_type = SHADER_TYPE_UNKNOWN;
    shader_infos[resource_count].resource_index = CE_RESOURCE_DATA_COUNT;

    shader_t* shader = ce_shader_new(name, shader_infos);
    if (NULL != shader) {
        ce_vector_push_back(ce_shader_manager->shaders, shader);
        return shader;
    }

    return NULL;
}
}
