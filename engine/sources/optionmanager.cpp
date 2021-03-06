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

#include "optionmanager.hpp"
#include "logging.hpp"
#include "registry.hpp"

#include <boost/filesystem.hpp>

namespace cursedearth
{
    option_manager_t::option_manager_t(const ce_optparse_ptr_t& parser):
        singleton_t<option_manager_t>(this)
    {
        const char *ei_path, *ce_path;

        ce_optparse_get(parser, "ei_path", &ei_path);
        ce_optparse_get(parser, "ce_path", &ce_path);

        if (NULL == ce_path) {
            ce_path = ei_path;
        }

        m_ei_path = ei_path;
        m_ce_path = ce_path;

        ce_optparse_get(parser, "window_width", &window_width);
        ce_optparse_get(parser, "window_height", &window_height);
        ce_optparse_get(parser, "fullscreen", &fullscreen);
        ce_optparse_get(parser, "fullscreen_width", &fullscreen_width);
        ce_optparse_get(parser, "fullscreen_height", &fullscreen_height);
        ce_optparse_get(parser, "fullscreen_bpp", &fullscreen_bpp);
        ce_optparse_get(parser, "fullscreen_rate", &fullscreen_rate);
        ce_optparse_get(parser, "fullscreen_rotation", &fullscreen_rotation);
        ce_optparse_get(parser, "fullscreen_reflection_x", &fullscreen_reflection_x);
        ce_optparse_get(parser, "fullscreen_reflection_y", &fullscreen_reflection_y);
        ce_optparse_get(parser, "list_video_modes", &list_video_modes);
        ce_optparse_get(parser, "list_video_rotations", &list_video_rotations);
        ce_optparse_get(parser, "list_video_reflections", &list_video_reflections);
        ce_optparse_get(parser, "inverse_trackball", &inverse_trackball);
        ce_optparse_get(parser, "inverse_trackball_x", &inverse_trackball_x);
        ce_optparse_get(parser, "inverse_trackball_y", &inverse_trackball_y);
        ce_optparse_get(parser, "enable_terrain_tiling", &m_enable_terrain_tiling);
        ce_optparse_get(parser, "disable_texture_caching", &m_disable_texture_caching);
        ce_optparse_get(parser, "disable_sound", &m_disable_sound);
        ce_optparse_get(parser, "show_axes", &m_show_axes);
        ce_optparse_get(parser, "show_fps", &m_show_fps);

        if (inverse_trackball) {
            inverse_trackball_x = true;
            inverse_trackball_y = true;
        }

        ce_logging_info("option manager: EI path is `%s'", m_ei_path.string().c_str());
        ce_logging_info("option manager: CE path is `%s'", m_ce_path.string().c_str());
        ce_logging_info("option manager: terrain tiling %s", m_enable_terrain_tiling ? "enabled" : "disabled");
        ce_logging_info("option manager: texture caching %s", m_disable_texture_caching ? "disabled" : "enabled");
    }

    ce_optparse_ptr_t option_manager_t::make_parser()
    {
        ce_optparse_ptr_t parser = std::make_shared<ce_optparse>();

        boost::filesystem::path ei_path = find_path_in_registry(registry_key_t::current_user, "Software\\Nival Interactive\\EvilIslands\\Path Settings", "WORK PATH");
        std::string ei_help;

        if (exists(ei_path)) {
            ei_help = str(boost::format("path to EI directory; using %1% by default (found in registry)") % ei_path);
        } else {
            if (!ei_path.empty()) {
                ce_logging_warning("option manager: EI path `%s' found in registry but it doesn't exist", ei_path.string().c_str());
            }
            ei_path = boost::filesystem::current_path();
            ei_help = "path to EI directory; registry value not found, using current directory by default";
        }

        ce_optparse_add(parser, "ei_path", CE_TYPE_STRING, ei_path.string().c_str(), false, NULL, "ei-path", ei_help.c_str());
        ce_optparse_add(parser, "ce_path", CE_TYPE_STRING, NULL, false, NULL, "ce-path", "path to CE directory; using EI path by default");

        const int window_width_default = 1024;
        ce_optparse_add(parser, "window_width", CE_TYPE_INT, &window_width_default, false, NULL, "window-width", "desired window width in window mode");

        const int window_height_default = 768;
        ce_optparse_add(parser, "window_height", CE_TYPE_INT, &window_height_default, false, NULL, "window-height", "desired window height in window mode");

        ce_optparse_add(parser, "fullscreen", CE_TYPE_BOOL, NULL, false, "f", "fullscreen", "start program in fullscreen mode");

        ce_optparse_add(parser, "fullscreen_width", CE_TYPE_INT, NULL, false, NULL, "fullscreen-width", "desired window width in fullscreen mode (max available by default)");
        ce_optparse_add(parser, "fullscreen_height", CE_TYPE_INT, NULL, false, NULL, "fullscreen-height", "desired window height in fullscreen mode (max available by default)");

        ce_optparse_add(parser, "fullscreen_bpp", CE_TYPE_INT, NULL, false, NULL, "fullscreen-bpp", "desired bits per pixel (max available by default)");
        ce_optparse_add(parser, "fullscreen_rate", CE_TYPE_INT, NULL, false, NULL, "fullscreen-rate", "desired refresh rate (max available by default)");
        ce_optparse_add(parser, "fullscreen_rotation", CE_TYPE_INT, NULL, false, NULL, "fullscreen-rotation", "specify rotation in degrees (90, 180, 270)");

        ce_optparse_add(parser, "fullscreen_reflection_x", CE_TYPE_BOOL, NULL, false, NULL, "fullscreen-reflection-x", "reflect around x axis");
        ce_optparse_add(parser, "fullscreen_reflection_y", CE_TYPE_BOOL, NULL, false, NULL, "fullscreen-reflection-y", "reflect around y axis");

        ce_optparse_add(parser, "list_video_modes", CE_TYPE_BOOL, NULL, false, NULL, "list-video-modes", "dump to stdout supported video modes (WIDTHxHEIGHT:BPP@RATE)");
        ce_optparse_add(parser, "list_video_rotations", CE_TYPE_BOOL, NULL, false, NULL, "list-video-rotations", "dump to stdout supported rotations");
        ce_optparse_add(parser, "list_video_reflections", CE_TYPE_BOOL, NULL, false, NULL, "list-video-reflections", "dump to stdout supported reflections");

        ce_optparse_add(parser, "inverse_trackball", CE_TYPE_BOOL, NULL, false, NULL, "inverse-trackball", "invert the x and y axes of the mouse movement");
        ce_optparse_add(parser, "inverse_trackball_x", CE_TYPE_BOOL, NULL, false, NULL, "inverse-trackball-x", "invert the x axis of the mouse movement");
        ce_optparse_add(parser, "inverse_trackball_y", CE_TYPE_BOOL, NULL, false, NULL, "inverse-trackball-y", "invert the y axis of the mouse movement");

        ce_optparse_add(parser, "enable_terrain_tiling", CE_TYPE_BOOL, NULL, false, NULL, "enable-terrain-tiling",
            "tile terrain; may be useful if you have a prehistoric video adapter; very slow!");

        ce_optparse_add(parser, "disable_texture_caching", CE_TYPE_BOOL, NULL, false, NULL, "disable-texture-caching",
            "do not save generated textures in cache (usually `Textures' directory, up to 1 GB disk space usage is normal); very slow if you have a prehistoric CPU!");

        ce_optparse_add(parser, "disable_sound", CE_TYPE_BOOL, NULL, false, NULL, "disable-sound", "turn off all sounds");
        ce_optparse_add(parser, "show_axes", CE_TYPE_BOOL, NULL, false, NULL, "show-axes", "show x (red), y (green), z (blue) axes");
        ce_optparse_add(parser, "show_fps", CE_TYPE_BOOL, NULL, false, NULL, "show-fps", "show FPS counter");

        ce_optparse_add_control(parser, "alt+tab", "minimize fullscreen window");
        ce_optparse_add_control(parser, "alt+enter", "toggle fullscreen mode");
        ce_optparse_add_control(parser, "b", "toggle bounding boxes (comprehensive/comprehensive+bones/none)");
        ce_optparse_add_control(parser, "keyboard arrows", "move camera");
        ce_optparse_add_control(parser, "mouse right button + motion", "rotate camera");
        ce_optparse_add_control(parser, "mouse wheel", "zoom camera");

        return parser;
    }
}
