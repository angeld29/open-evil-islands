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

#ifndef CE_OPTIONMANAGER_HPP
#define CE_OPTIONMANAGER_HPP

#include <boost/filesystem/path.hpp>

#include "singleton.hpp"
#include "string.hpp"
#include "optparse.hpp"

namespace cursedearth
{
    class option_manager_t: public singleton_t<option_manager_t>
    {
    public:
        explicit option_manager_t(ce_optparse*);
        ~option_manager_t();

        static ce_optparse* make_parser();

        const boost::filesystem::path& ei_path() const { return m_ei_path; }
        const boost::filesystem::path& ce_path() const { return m_ce_path; }

    public:
        ce_string* ei_path2;
        int window_width = 1024;
        int window_height = 768;
        bool fullscreen = false;
        int fullscreen_width = 1024;
        int fullscreen_height = 768;
        int fullscreen_bpp;
        int fullscreen_rate;
        int fullscreen_rotation = 0;
        bool fullscreen_reflection_x = false;
        bool fullscreen_reflection_y = false;
        bool list_video_modes = false;
        bool list_video_rotations = false;
        bool list_video_reflections = false;
        bool inverse_trackball = false;
        bool inverse_trackball_x = false;
        bool inverse_trackball_y = false;
        bool terrain_tiling = false;
        bool texture_caching = false;
        bool disable_sound = false;
        bool show_axes = false;
        bool show_fps = false;

    private:
        boost::filesystem::path m_ei_path;
        boost::filesystem::path m_ce_path;
    };
}

#endif
