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

#include <cstring>

#include "path.hpp"
#include "logging.hpp"
#include "resfile.hpp"
#include "resball.hpp"
#include "optionmanager.hpp"
#include "soundmanager.hpp"

namespace cursedearth
{
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
            ce_logging_info("sound manager: loading `%s'... ok", path.data());
        } else {
            ce_logging_error("sound manager: loading `%s'... failed", path.data());
        }

        return res_file;
    }

    sound_manager_t::sound_manager_t():
        singleton_t<sound_manager_t>(this)
    {
        std::vector<char> path(ce_option_manager->ei_path->length + 16);
        for (size_t i = 0; NULL != ce_sound_dirs[i]; ++i) {
            ce_path_join(path.data(), path.size(), ce_option_manager->ei_path->str, ce_sound_dirs[i], NULL);
            ce_logging_info("sound manager: using path `%s'", path.data());
        }

        for (size_t i = 0; NULL != ce_sound_resource_names[i]; ++i) {
            if (ce_res_file* res_file = ce_sound_manager_open_resource(ce_sound_resource_names[i])) {
                m_files.push_back(res_file);
            }
        }
    }

    sound_manager_t::~sound_manager_t()
    {
        for (const auto& file: m_files) {
            ce_res_file_del(file);
        }
    }

    void sound_manager_t::advance(float /*elapsed*/)
    {
    }

    sound_object_t sound_manager_t::make_instance(const std::string& name)
    {
        ce_mem_file* mem_file = NULL;
        for (const auto& file: m_files) {
            if (NULL != (mem_file = ce_res_ball_extract_mem_file_by_name(file, name.c_str()))) {
                break;
            }
        }

        if (NULL == mem_file) {
            std::vector<char> path(ce_option_manager->ei_path->length + strlen(name.c_str()) + 32);
            if (NULL == ce_path_find_special1(path.data(), path.size(), ce_option_manager->ei_path->str, name.c_str(), ce_sound_dirs, ce_sound_exts)) {
                ce_logging_error("sound manager: could not find sound `%s'", name.c_str());
                return 0;
            }

            mem_file = ce_mem_file_new_path(path.data());
            if (NULL == mem_file) {
                ce_logging_error("sound manager: could not open file `%s'", path.data());
                return 0;
            }
        }

        ce_sound_resource* resource = ce_sound_resource_new(mem_file);
        if (NULL == resource) {
            ce_logging_error("sound manager: could not create resource `%s'", name.c_str());
            ce_mem_file_del(mem_file);
            return 0;
        }

        const sound_object_t object = ++m_last_object;
        sound_instance_ptr_t instance = std::make_shared<sound_instance_t>(resource);

        m_instances.insert({ object, instance });
        return object;
    }
}
