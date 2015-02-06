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

#include "path.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "resfile.hpp"
#include "resball.hpp"
#include "event.hpp"
#include "optionmanager.hpp"
#include "soundmanager.hpp"

namespace cursedearth
{
    struct ce_sound_manager* ce_sound_manager;

    const char* ce_sound_dirs[] = { "Stream", "Movies", NULL };
    const char* ce_sound_exts[] = { ".wav", ".oga", ".ogv", ".ogg", ".mp3", ".bik", ".flac", NULL };
    const char* ce_sound_resource_dirs[] = { "Res", NULL };
    const char* ce_sound_resource_exts[] = { ".res", NULL };
    const char* ce_sound_resource_names[] = { "sfx", "speech", NULL };

    ce_res_file* ce_sound_manager_open_resource(const char* name)
    {
        std::vector<char> path(ce_option_manager->ei_path->length + 32);
        ce_res_file* res_file = NULL;

        if (NULL != ce_path_find_special1(path.data(), path.size(), ce_option_manager->ei_path->str,
                name, ce_sound_resource_dirs, ce_sound_resource_exts) && NULL != (res_file = ce_res_file_new_path(path.data()))) {
            ce_logging_write("sound manager: loading `%s'... ok", path.data());
        } else {
            ce_logging_error("sound manager: loading `%s'... failed", path.data());
        }

        return res_file;
    }

    void ce_sound_manager_init(void)
    {
        ce_sound_manager = (struct ce_sound_manager*)ce_alloc_zero(sizeof(struct ce_sound_manager));
        ce_sound_manager->res_files = ce_vector_new();
        ce_sound_manager->sound_instances = ce_vector_new();

        std::vector<char> path(ce_option_manager->ei_path->length + 16);
        for (size_t i = 0; NULL != ce_sound_dirs[i]; ++i) {
            ce_path_join(path.data(), path.size(), ce_option_manager->ei_path->str, ce_sound_dirs[i], NULL);
            ce_logging_write("sound manager: using path `%s'", path.data());
        }

        for (size_t i = 0; NULL != ce_sound_resource_names[i]; ++i) {
            ce_res_file* res_file = ce_sound_manager_open_resource(ce_sound_resource_names[i]);
            if (NULL != res_file) {
                ce_vector_push_back(ce_sound_manager->res_files, res_file);
            }
        }
    }

    void ce_sound_manager_term(void)
    {
        if (NULL != ce_sound_manager) {
            ce_vector_for_each(ce_sound_manager->sound_instances, (void(*)(void*))ce_sound_instance_del);
            ce_vector_for_each(ce_sound_manager->res_files, (void(*)(void*))ce_res_file_del);
            ce_vector_del(ce_sound_manager->res_files);
            ce_free(ce_sound_manager, sizeof(struct ce_sound_manager));
        }
    }

    void ce_sound_manager_advance(float /*elapsed*/)
    {
    }

    ce_sound_object ce_sound_manager_create_object(const char* name)
    {
        ce_mem_file* mem_file = NULL;
        for (size_t i = 0; i < ce_sound_manager->res_files->count; ++i) {
            ce_res_file* res_file = (ce_res_file*)ce_sound_manager->res_files->items[i];
            if (NULL != (mem_file = ce_res_ball_extract_mem_file_by_name(res_file, name))) {
                break;
            }
        }

        if (NULL == mem_file) {
            std::vector<char> path(ce_option_manager->ei_path->length + strlen(name) + 32);
            if (NULL == ce_path_find_special1(path.data(), path.size(), ce_option_manager->ei_path->str, name, ce_sound_dirs, ce_sound_exts)) {
                ce_logging_error("sound manager: could not find sound `%s'", name);
                return 0;
            }

            mem_file = ce_mem_file_new_path(path.data());
            if (NULL == mem_file) {
                ce_logging_error("sound manager: could not open file `%s'", path.data());
                return 0;
            }
        }

        ce_sound_resource* sound_resource = ce_sound_resource_new(mem_file);
        if (NULL == sound_resource) {
            ce_logging_error("sound manager: could not create resource `%s'", name);
            ce_mem_file_del(mem_file);
            return 0;
        }

        ce_sound_instance* sound_instance = ce_sound_instance_new(++ce_sound_manager->last_sound_object, sound_resource);
        if (NULL == sound_instance) {
            ce_logging_error("sound manager: could not create instance '%s'", name);
            ce_sound_resource_del(sound_resource);
            return 0;
        }

        ce_vector_push_back(ce_sound_manager->sound_instances, sound_instance);
        return sound_instance->sound_object;
    }

    ce_sound_instance* ce_sound_manager_find_instance(ce_sound_object sound_object)
    {
        for (size_t i = 0; i < ce_sound_manager->sound_instances->count; ++i) {
            ce_sound_instance* sound_instance = (ce_sound_instance*)ce_sound_manager->sound_instances->items[i];
            if (sound_object == sound_instance->sound_object) {
                return sound_instance;
            }
        }
        return nullptr;
    }
}
