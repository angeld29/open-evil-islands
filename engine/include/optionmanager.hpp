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

#include "string.hpp"
#include "optparse.hpp"

namespace cursedearth
{
    extern struct ce_option_manager {
        ce_string* ei_path;
        ce_string* ce_path;
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
        bool terrain_tiling;
        bool texture_caching;
        bool disable_sound;
        bool show_axes;
        bool show_fps;
    }* ce_option_manager;

    void ce_option_manager_init(ce_optparse* optparse);
    void ce_option_manager_term(void);

    ce_optparse* ce_option_manager_create_option_parser(void);
}

#endif
