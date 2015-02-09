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

#include "str.hpp"
#include "logging.hpp"
#include "path.hpp"
#include "registry.hpp"
#include "optionmanager.hpp"

namespace cursedearth
{
    option_manager_t::option_manager_t(ce_optparse* parser):
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

        ei_path2 = ce_string_new_str(ei_path);

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
        ce_optparse_get(parser, "terrain_tiling", &terrain_tiling);
        ce_optparse_get(parser, "texture_caching", &texture_caching);
        ce_optparse_get(parser, "disable_sound", &disable_sound);
        ce_optparse_get(parser, "show_axes", &show_axes);
        ce_optparse_get(parser, "show_fps", &show_fps);

        ce_path_normpath(ei_path2->str);

        if (inverse_trackball) {
            inverse_trackball_x = true;
            inverse_trackball_y = true;
        }

        ce_logging_info("option manager: EI path is `%s'", m_ei_path.string().c_str());
        ce_logging_info("option manager: CE path is `%s'", m_ce_path.string().c_str());
        ce_logging_info("option manager: terrain tiling %s", terrain_tiling ? "enabled" : "disabled");
        ce_logging_info("option manager: texture caching %s", texture_caching ? "enabled" : "disabled");
    }

    option_manager_t::~option_manager_t()
    {
        ce_string_del(ei_path2);
    }

    ce_optparse* option_manager_t::make_parser()
    {
        ce_optparse* parser = ce_optparse_new();
        std::vector<char> path(CE_PATH_MAX), help(512);

        if (NULL != ce_registry_get_path_value(path.data(), path.size(), CE_REGISTRY_KEY_CURRENT_USER, "Software\\Nival Interactive\\EvilIslands\\Path Settings", "WORK PATH")) {
            snprintf(help.data(), help.size(), "path to EI directory; using `%s' by default (found in registry)", path.data());
        } else {
            ce_strlcpy(path.data(), ".", path.size());
            ce_strlcpy(help.data(), "path to EI directory; registry value not found, using current directory by default", help.size());
        }

        ce_optparse_add(parser, "ei_path",
            CE_TYPE_STRING, path.data(), false, NULL, "ei-path", help.data());

        ce_optparse_add(parser, "ce_path",
            CE_TYPE_STRING, NULL, false, NULL, "ce-path",
            "reserved for future use: path to CE directory; using EI path by default");

        const int window_width_default = 1024;
        ce_optparse_add(parser, "window_width",
            CE_TYPE_INT, &window_width_default, false, NULL, "window-width",
            "desired window width in window mode");

        const int window_height_default = 768;
        ce_optparse_add(parser, "window_height",
            CE_TYPE_INT, &window_height_default, false, NULL, "window-height",
            "desired window height in window mode");

        ce_optparse_add(parser, "fullscreen",
            CE_TYPE_BOOL, NULL, false, "f", "fullscreen",
            "start program in fullscreen mode");

        ce_optparse_add(parser, "fullscreen_width",
            CE_TYPE_INT, NULL, false, NULL, "fullscreen-width",
            "desired window width in fullscreen mode (max available by default)");

        ce_optparse_add(parser, "fullscreen_height",
            CE_TYPE_INT, NULL, false, NULL, "fullscreen-height",
            "desired window height in fullscreen mode (max available by default)");

        ce_optparse_add(parser, "fullscreen_bpp",
            CE_TYPE_INT, NULL, false, NULL, "fullscreen-bpp",
            "desired bits per pixel (max available by default)");

        ce_optparse_add(parser, "fullscreen_rate",
            CE_TYPE_INT, NULL, false, NULL, "fullscreen-rate",
            "desired refresh rate (max available by default)");

        ce_optparse_add(parser, "fullscreen_rotation",
            CE_TYPE_INT, NULL, false, NULL, "fullscreen-rotation",
            "specify rotation in degrees (90, 180, 270)");

        ce_optparse_add(parser, "fullscreen_reflection_x",
            CE_TYPE_BOOL, NULL, false, NULL, "fullscreen-reflection-x",
            "reflect around x axis");

        ce_optparse_add(parser, "fullscreen_reflection_y",
            CE_TYPE_BOOL, NULL, false, NULL, "fullscreen-reflection-y",
            "reflect around y axis");

        ce_optparse_add(parser, "list_video_modes",
            CE_TYPE_BOOL, NULL, false, NULL, "list-video-modes",
            "display supported video modes in format WIDTHxHEIGHT:BPP@RATE "
            "and exit (output to stdout)");

        ce_optparse_add(parser, "list_video_rotations",
            CE_TYPE_BOOL, NULL, false, NULL, "list-video-rotations",
            "display supported rotations and exit (output to stdout)");

        ce_optparse_add(parser, "list_video_reflections",
            CE_TYPE_BOOL, NULL, false, NULL, "list-video-reflections",
            "display supported reflections and exit (output to stdout)");

        ce_optparse_add(parser, "inverse_trackball",
            CE_TYPE_BOOL, NULL, false, NULL, "inverse-trackball",
            "invert the x and y axes of the mouse movement");

        ce_optparse_add(parser, "inverse_trackball_x",
            CE_TYPE_BOOL, NULL, false, NULL, "inverse-trackball-x",
            "invert the x axis of the mouse movement");

        ce_optparse_add(parser, "inverse_trackball_y",
            CE_TYPE_BOOL, NULL, false, NULL, "inverse-trackball-y",
            "invert the y axis of the mouse movement");

        ce_optparse_add(parser, "terrain_tiling",
            CE_TYPE_BOOL, NULL, false, NULL, "terrain-tiling",
            "enable terrain tiling; may be useful if you have "
            "a prehistoric video adapter; very slow!");

        ce_optparse_add(parser, "texture_caching",
            CE_TYPE_BOOL, NULL, false, NULL, "texture-caching",
            "save generated textures in cache (usually `Textures' directory); "
            "warning: up to 1 GB disk space usage is normal; "
            "very useful if you have a single-core slow CPU");

        ce_optparse_add(parser, "disable_sound",
            CE_TYPE_BOOL, NULL, false, NULL, "disable-sound",
            "turn off all sounds");

        ce_optparse_add(parser, "show_axes",
            CE_TYPE_BOOL, NULL, false, NULL, "show-axes",
            "show x (red), y (green), z (blue) axes");

        ce_optparse_add(parser, "show_fps",
            CE_TYPE_BOOL, NULL, false, NULL, "show-fps",
            "show FPS counter");

        ce_optparse_add_control(parser, "alt+tab", "minimize fullscreen window");
        ce_optparse_add_control(parser, "alt+enter", "toggle fullscreen mode");
        ce_optparse_add_control(parser, "b", "toggle bounding boxes (comprehensive/comprehensive+bones/none)");
        ce_optparse_add_control(parser, "keyboard arrows", "move camera");
        ce_optparse_add_control(parser, "mouse right button + motion", "rotate camera");
        ce_optparse_add_control(parser, "mouse wheel", "zoom camera");

        return parser;
    }
}
