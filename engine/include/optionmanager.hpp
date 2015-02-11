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
#include "optparse.hpp"

namespace cursedearth
{
    class option_manager_t: public singleton_t<option_manager_t>
    {
    public:
        explicit option_manager_t(const ce_optparse_ptr_t&);

        static ce_optparse_ptr_t make_parser();

        const boost::filesystem::path& ei_path() const { return m_ei_path; }
        const boost::filesystem::path& ce_path() const { return m_ce_path; }

        bool terrain_tiling() const { return m_enable_terrain_tiling; }
        bool texture_caching() const { return !m_disable_texture_caching; }
        bool disable_sound() const { return m_disable_sound; }

        bool show_axes() const { return m_show_axes; }
        bool show_fps() const { return m_show_fps; }

    public:
        int window_width;
        int window_height;
        bool fullscreen;
        int fullscreen_width;
        int fullscreen_height;
        int fullscreen_bpp;
        int fullscreen_rate;
        int fullscreen_rotation;
        bool fullscreen_reflection_x;
        bool fullscreen_reflection_y;
        bool list_video_modes;
        bool list_video_rotations;
        bool list_video_reflections;
        bool inverse_trackball;
        bool inverse_trackball_x;
        bool inverse_trackball_y;

    private:
        boost::filesystem::path m_ei_path;
        boost::filesystem::path m_ce_path;
        bool m_enable_terrain_tiling;
        bool m_disable_texture_caching;
        bool m_disable_sound;
        bool m_show_axes;
        bool m_show_fps;
    };
}

#endif
