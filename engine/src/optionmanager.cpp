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
#include "alloc.hpp"
#include "logging.hpp"
#include "thread.hpp"
#include "path.hpp"
#include "registry.hpp"
#include "optionmanager.hpp"

namespace cursedearth
{
    struct ce_option_manager* ce_option_manager;

    void ce_option_manager_init(ce_optparse* optparse)
    {
        const char *ei_path, *ce_path;

        ce_optparse_get(optparse, "ei_path", &ei_path);
        ce_optparse_get(optparse, "ce_path", &ce_path);

        if (NULL == ce_path) {
            ce_path = ei_path;
        }

        ce_option_manager = (struct ce_option_manager*)ce_alloc_zero(sizeof(struct ce_option_manager));
        ce_option_manager->ei_path = ce_string_new_str(ei_path);
        ce_option_manager->ce_path = ce_string_new_str(ce_path);
        ce_optparse_get(optparse, "window_width", &ce_option_manager->window_width);
        ce_optparse_get(optparse, "window_height", &ce_option_manager->window_height);
        ce_optparse_get(optparse, "fullscreen", &ce_option_manager->fullscreen);
        ce_optparse_get(optparse, "fullscreen_width", &ce_option_manager->fullscreen_width);
        ce_optparse_get(optparse, "fullscreen_height", &ce_option_manager->fullscreen_height);
        ce_optparse_get(optparse, "fullscreen_bpp", &ce_option_manager->fullscreen_bpp);
        ce_optparse_get(optparse, "fullscreen_rate", &ce_option_manager->fullscreen_rate);
        ce_optparse_get(optparse, "fullscreen_rotation", &ce_option_manager->fullscreen_rotation);
        ce_optparse_get(optparse, "fullscreen_reflection_x", &ce_option_manager->fullscreen_reflection_x);
        ce_optparse_get(optparse, "fullscreen_reflection_y", &ce_option_manager->fullscreen_reflection_y);
        ce_optparse_get(optparse, "list_video_modes", &ce_option_manager->list_video_modes);
        ce_optparse_get(optparse, "list_video_rotations", &ce_option_manager->list_video_rotations);
        ce_optparse_get(optparse, "list_video_reflections", &ce_option_manager->list_video_reflections);
        ce_optparse_get(optparse, "inverse_trackball", &ce_option_manager->inverse_trackball);
        ce_optparse_get(optparse, "inverse_trackball_x", &ce_option_manager->inverse_trackball_x);
        ce_optparse_get(optparse, "inverse_trackball_y", &ce_option_manager->inverse_trackball_y);
        ce_optparse_get(optparse, "terrain_tiling", &ce_option_manager->terrain_tiling);
        ce_optparse_get(optparse, "texture_caching", &ce_option_manager->texture_caching);
        ce_optparse_get(optparse, "thread_count", &ce_option_manager->thread_count);
        ce_optparse_get(optparse, "disable_sound", &ce_option_manager->disable_sound);
        ce_optparse_get(optparse, "show_axes", &ce_option_manager->show_axes);
        ce_optparse_get(optparse, "show_fps", &ce_option_manager->show_fps);

        ce_path_normpath(ce_option_manager->ei_path->str);
        ce_path_normpath(ce_option_manager->ce_path->str);

        if (ce_option_manager->inverse_trackball) {
            ce_option_manager->inverse_trackball_x = true;
            ce_option_manager->inverse_trackball_y = true;
        }

        ce_logging_write("option manager: EI path is '%s'", ce_option_manager->ei_path->str);
        ce_logging_write("option manager: CE path is '%s'", ce_option_manager->ce_path->str);
        ce_logging_write("option manager: using up to %d threads", ce_option_manager->thread_count);
        ce_logging_write("option manager: terrain tiling %s",
            ce_option_manager->terrain_tiling ? "enabled" : "disabled");
    }

    void ce_option_manager_term(void)
    {
        if (NULL != ce_option_manager) {
            ce_string_del(ce_option_manager->ce_path);
            ce_string_del(ce_option_manager->ei_path);
            ce_free(ce_option_manager, sizeof(struct ce_option_manager));
        }
    }

    ce_optparse* ce_option_manager_create_option_parser(void)
    {
        ce_optparse* optparse = (ce_optparse*)ce_optparse_new();
        std::vector<char> path(CE_PATH_MAX), help(512);

        if (NULL != ce_registry_get_path_value(path.data(), path.size(), CE_REGISTRY_KEY_CURRENT_USER, "Software\\Nival Interactive\\EvilIslands\\Path Settings", "WORK PATH")) {
            snprintf(help.data(), help.size(), "path to EI directory; using `%s' by default (found in registry)", path.data());
        } else {
            ce_strlcpy(path.data(), ".", path.size());
            ce_strlcpy(help.data(), "path to EI directory; registry value not found, using current directory by default", help.size());
        }

        ce_optparse_add(optparse, "ei_path",
            CE_TYPE_STRING, path.data(), false, NULL, "ei-path", help.data());

        ce_optparse_add(optparse, "ce_path",
            CE_TYPE_STRING, NULL, false, NULL, "ce-path",
            "reserved for future use: path to CE directory; using EI path by default");

        const int window_width_default = 1024;
        ce_optparse_add(optparse, "window_width",
            CE_TYPE_INT, &window_width_default, false, NULL, "window-width",
            "desired window width in window mode");

        const int window_height_default = 768;
        ce_optparse_add(optparse, "window_height",
            CE_TYPE_INT, &window_height_default, false, NULL, "window-height",
            "desired window height in window mode");

        ce_optparse_add(optparse, "fullscreen",
            CE_TYPE_BOOL, NULL, false, "f", "fullscreen",
            "start program in fullscreen mode");

        ce_optparse_add(optparse, "fullscreen_width",
            CE_TYPE_INT, NULL, false, NULL, "fullscreen-width",
            "desired window width in fullscreen mode (max available by default)");

        ce_optparse_add(optparse, "fullscreen_height",
            CE_TYPE_INT, NULL, false, NULL, "fullscreen-height",
            "desired window height in fullscreen mode (max available by default)");

        ce_optparse_add(optparse, "fullscreen_bpp",
            CE_TYPE_INT, NULL, false, NULL, "fullscreen-bpp",
            "desired bits per pixel (max available by default)");

        ce_optparse_add(optparse, "fullscreen_rate",
            CE_TYPE_INT, NULL, false, NULL, "fullscreen-rate",
            "desired refresh rate (max available by default)");

        ce_optparse_add(optparse, "fullscreen_rotation",
            CE_TYPE_INT, NULL, false, NULL, "fullscreen-rotation",
            "specify rotation in degrees (90, 180, 270)");

        ce_optparse_add(optparse, "fullscreen_reflection_x",
            CE_TYPE_BOOL, NULL, false, NULL, "fullscreen-reflection-x",
            "reflect around x axis");

        ce_optparse_add(optparse, "fullscreen_reflection_y",
            CE_TYPE_BOOL, NULL, false, NULL, "fullscreen-reflection-y",
            "reflect around y axis");

        ce_optparse_add(optparse, "list_video_modes",
            CE_TYPE_BOOL, NULL, false, NULL, "list-video-modes",
            "display supported video modes in format WIDTHxHEIGHT:BPP@RATE "
            "and exit (output to stdout)");

        ce_optparse_add(optparse, "list_video_rotations",
            CE_TYPE_BOOL, NULL, false, NULL, "list-video-rotations",
            "display supported rotations and exit (output to stdout)");

        ce_optparse_add(optparse, "list_video_reflections",
            CE_TYPE_BOOL, NULL, false, NULL, "list-video-reflections",
            "display supported reflections and exit (output to stdout)");

        ce_optparse_add(optparse, "inverse_trackball",
            CE_TYPE_BOOL, NULL, false, NULL, "inverse-trackball",
            "invert the x and y axes of the mouse movement");

        ce_optparse_add(optparse, "inverse_trackball_x",
            CE_TYPE_BOOL, NULL, false, NULL, "inverse-trackball-x",
            "invert the x axis of the mouse movement");

        ce_optparse_add(optparse, "inverse_trackball_y",
            CE_TYPE_BOOL, NULL, false, NULL, "inverse-trackball-y",
            "invert the y axis of the mouse movement");

        ce_optparse_add(optparse, "terrain_tiling",
            CE_TYPE_BOOL, NULL, false, NULL, "terrain-tiling",
            "enable terrain tiling; may be useful if you have "
            "a prehistoric video adapter; very slow!");

        ce_optparse_add(optparse, "texture_caching",
            CE_TYPE_BOOL, NULL, false, NULL, "texture-caching",
            "save generated textures in cache (usually `Textures' directory); "
            "warning: up to 1 GB disk space usage is normal; "
            "very useful if you have a single-core slow CPU");

        const int thread_count = ce_online_cpu_count();
        snprintf(help.data(), help.size(),
            "allow THREAD_COUNT jobs at once; if this option is not "
            "specified, the value will be detected automatically depending on the "
            "number of CPUs you have (or the number of cores your CPU have); "
            "using %d thread(s) by default (auto-detected)", thread_count);

        ce_optparse_add(optparse, "thread_count",
            CE_TYPE_INT, &thread_count, false, "j", "jobs", help.data());

        ce_optparse_add(optparse, "disable_sound",
            CE_TYPE_BOOL, NULL, false, NULL, "disable-sound",
            "turn off all sounds");

        ce_optparse_add(optparse, "show_axes",
            CE_TYPE_BOOL, NULL, false, NULL, "show-axes",
            "show x (red), y (green), z (blue) axes");

        ce_optparse_add(optparse, "show_fps",
            CE_TYPE_BOOL, NULL, false, NULL, "show-fps",
            "show FPS counter");

        ce_optparse_add_control(optparse, "alt+tab", "minimize fullscreen window");
        ce_optparse_add_control(optparse, "alt+enter", "toggle fullscreen mode");
        ce_optparse_add_control(optparse, "b", "toggle bounding boxes (comprehensive/comprehensive+bones/none)");
        ce_optparse_add_control(optparse, "keyboard arrows", "move camera");
        ce_optparse_add_control(optparse, "mouse right button + motion", "rotate camera");
        ce_optparse_add_control(optparse, "mouse wheel", "zoom camera");

        return optparse;
    }
}
