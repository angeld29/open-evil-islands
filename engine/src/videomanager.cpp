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

#include <vector>

#include <boost/filesystem.hpp>

#include "exception.hpp"
#include "logging.hpp"
#include "optionmanager.hpp"
#include "videomanager.hpp"

namespace cursedearth
{
    namespace fs = boost::filesystem;

    const std::vector<fs::path> g_video_directories = { "Movies" };
    const std::vector<std::string> g_video_extensions = { ".ogv" , ".ogg", ".bik" };

    video_manager_t::video_manager_t():
        singleton_t<video_manager_t>(this)
    {
        fs::path root = option_manager_t::instance()->ei_path().string().c_str();
        for (const auto& directory: g_video_directories) {
            ce_logging_info("video manager: using path `%s'", (root / directory).string().c_str());
        }
    }

    void video_manager_t::advance(float /*elapsed*/)
    {
    }

    fs::path find_resource(const std::string& name)
    {
        const fs::path root = option_manager_t::instance()->ei_path();
        for (const auto& extension: g_video_extensions) {
            const fs::path file_name = name + extension;
            for (const auto& directory: g_video_directories) {
                const fs::path file_path = root / directory / file_name;
                if (exists(file_path)) {
                    return file_path;
                }
            }
        }
        throw game_error("video manager", boost::format("could not find resource `%1%'") % name);
    }

    video_object_t video_manager_t::make_instance(const std::string& name)
    {
        fs::path file_path = find_resource(name);
        ce_mem_file* mem_file = ce_mem_file_new_path(file_path.string().c_str());
        if (NULL == mem_file) {
            throw game_error("video manager", boost::format("could not open file `%1%'") % file_path);
        }

        ce_video_resource* resource = ce_video_resource_new(mem_file);
        if (NULL == resource) {
            ce_mem_file_del(mem_file);
            throw game_error("video manager", boost::format("could not create resource `%1%'") % file_path);
        }

        const video_object_t object = ++m_last_object;
        video_instance_ptr_t instance = std::make_shared<video_instance_t>(make_sound_object(name), resource);

        m_instances.insert({ object, instance });
        return object;
    }

    video_manager_ptr_t make_video_manager()
    {
        return make_unique<video_manager_t>();
    }
}
