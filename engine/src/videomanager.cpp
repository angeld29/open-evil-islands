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
#include "videomanager.hpp"

namespace cursedearth
{
    struct ce_video_manager* ce_video_manager;

    const char* ce_video_dirs[] = { "Movies", NULL };
    const char* ce_video_exts[] = { ".ogv", ".ogg", ".bik", NULL };

    void ce_video_manager_init(void)
    {
        ce_video_manager = (struct ce_video_manager*)ce_alloc_zero(sizeof(struct ce_video_manager));
        ce_video_manager->video_instances = ce_vector_new();

        std::vector<char> path(ce_option_manager->ei_path->length + 16);
        for (size_t i = 0; NULL != ce_video_dirs[i]; ++i) {
            ce_path_join(path.data(), path.size(), ce_option_manager->ei_path->str, ce_video_dirs[i], NULL);
            ce_logging_write("video manager: using path `%s'", path.data());
        }
    }

    void ce_video_manager_term(void)
    {
        if (NULL != ce_video_manager) {
            ce_vector_for_each(ce_video_manager->video_instances, (void(*)(void*))ce_video_instance_del);
            ce_vector_del(ce_video_manager->video_instances);
            ce_free(ce_video_manager, sizeof(struct ce_video_manager));
        }
    }

    void ce_video_manager_advance(float /*elapsed*/)
    {
    }

    ce_video_instance* ce_video_manager_create_instance(const char* name)
    {
        std::vector<char> path(ce_option_manager->ei_path->length + strlen(name) + 32);
        if (NULL == ce_path_find_special1(path.data(), path.size(), ce_option_manager->ei_path->str, name, ce_video_dirs, ce_video_exts)) {
            ce_logging_error("video manager: could not find path `%s'", path.data());
            return NULL;
        }

        ce_mem_file* mem_file = ce_mem_file_new_path(path.data());
        if (NULL == mem_file) {
            ce_logging_error("video manager: could not open file `%s'", path.data());
            return NULL;
        }

        ce_video_resource* video_resource = ce_video_resource_new(mem_file);
        if (NULL == video_resource) {
            ce_logging_error("video manager: could not find decoder for `%s'", path.data());
            ce_mem_file_del(mem_file);
            return NULL;
        }

        ce_video_instance* video_instance = ce_video_instance_new(++ce_video_manager->last_video_object, ce_sound_object_new(name), video_resource);
        if (NULL == video_instance) {
            ce_logging_error("video manager: could not create instance `%s'", path.data());
            ce_video_resource_del(video_resource);
            return NULL;
        }

        ce_vector_push_back(ce_video_manager->video_instances, video_instance);
        return video_instance;
    }

    ce_video_instance* ce_video_manager_find_instance(ce_video_object video_object)
    {
        for (size_t i = 0; i < ce_video_manager->video_instances->count; ++i) {
            ce_video_instance* video_instance = (ce_video_instance*)ce_video_manager->video_instances->items[i];
            if (video_object == video_instance->video_object) {
                return video_instance;
            }
        }
        return NULL;
    }
}
