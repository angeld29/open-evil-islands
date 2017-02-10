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

#include "soundmanager.hpp"
#include "optionmanager.hpp"
#include "resball.hpp"

#include <boost/filesystem.hpp>

namespace cursedearth
{
    namespace fs = boost::filesystem;

    const std::vector<std::string> ce_sound_dirs = { "Stream", "Movies" };
    const std::vector<std::string> ce_sound_exts = { ".wav", ".oga", ".ogv", ".ogg", ".mp3", ".bik", ".flac" };
    const std::vector<std::string> ce_sound_resource_dirs = { "Res" };
    const std::vector<std::string> ce_sound_resource_exts = { ".res" };
    const std::vector<std::string> ce_sound_resource_names = { "sfx", "speech" };

    fs::path find_sound(const std::string& name)
    {
        const fs::path root = option_manager_t::instance()->ei_path();
        for (const auto& extension: ce_sound_exts) {
            const fs::path file_name = name + extension;
            for (const auto& directory: ce_sound_dirs) {
                const fs::path file_path = root / directory / file_name;
                if (exists(file_path)) {
                    return file_path;
                }
            }
        }
        return fs::path();
    }

    fs::path find_sound_resource(const std::string& name)
    {
        const fs::path root = option_manager_t::instance()->ei_path();
        for (const auto& extension: ce_sound_resource_exts) {
            const fs::path file_name = name + extension;
            for (const auto& directory: ce_sound_resource_dirs) {
                const fs::path file_path = root / directory / file_name;
                if (exists(file_path)) {
                    return file_path;
                }
            }
        }
        return fs::path();
    }

    ce_res_file* ce_sound_manager_open_resource(const std::string& name)
    {
        fs::path path = find_sound_resource(name);
        ce_res_file* res_file = NULL;
        if (!path.empty() && NULL != (res_file = ce_res_file_new_path(path))) {
            ce_logging_info("sound manager: loading `%s'... ok", path.string().c_str());
        } else {
            ce_logging_error("sound manager: loading `%s'... failed", path.string().c_str());
        }
        return res_file;
    }

    sound_manager_t::sound_manager_t():
        singleton_t<sound_manager_t>(this)
    {
        for (const auto& directory: ce_sound_dirs) {
            fs::path path = option_manager_t::instance()->ei_path() / directory;
            ce_logging_info("sound manager: using path `%s'", path.string().c_str());
        }

        for (const auto& name: ce_sound_resource_names) {
            if (ce_res_file* res_file = ce_sound_manager_open_resource(name)) {
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
            if (NULL != (mem_file = ce_res_ball_extract_mem_file_by_name(file, name))) {
                break;
            }
        }

        if (NULL == mem_file) {
            fs::path path = find_sound(name);
            if (path.empty()) {
                ce_logging_error("sound manager: could not find sound `%s'", name.c_str());
                return 0;
            }

            mem_file = ce_mem_file_new_path(path);
            if (NULL == mem_file) {
                ce_logging_error("sound manager: could not open file `%s'", path.string().c_str());
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
