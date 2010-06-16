/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cethread.h"
#include "ceoptionmanager.h"

struct ce_option_manager* ce_option_manager;

void ce_option_manager_init(ce_optparse* optparse)
{
	const char *ei_path, *ce_path;

	ce_option_manager = ce_alloc_zero(sizeof(struct ce_option_manager));
	ce_optparse_get(optparse, "ei_path", &ei_path);
	ce_optparse_get(optparse, "ce_path", &ce_path);
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
	ce_optparse_get(optparse, "thread_count", &ce_option_manager->thread_count);
	ce_optparse_get(optparse, "show_axes", &ce_option_manager->show_axes);
	ce_optparse_get(optparse, "show_fps", &ce_option_manager->show_fps);

	if (ce_option_manager->inverse_trackball) {
		ce_option_manager->inverse_trackball_x = true;
		ce_option_manager->inverse_trackball_y = true;
	}

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

ce_optparse* ce_option_manager_create_optparse(void)
{
	ce_optparse* optparse = ce_optparse_new();

	ce_optparse_add(optparse, "ei_path",
		CE_TYPE_STRING, ".", false, NULL, "ei-path",
		"path to EI directory (current by default)");

	ce_optparse_add(optparse, "ce_path",
		CE_TYPE_STRING, ".", false, NULL, "ce-path",
		"reserved for future use: path to CE directory (current by default)");

	ce_optparse_add(optparse, "window_width",
		CE_TYPE_INT, (int[]){1024}, false, NULL, "window-width",
		"desired window width in window mode");

	ce_optparse_add(optparse, "window_height",
		CE_TYPE_INT, (int[]){768}, false, NULL, "window-height",
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
		"enable terrain tiling; very slow, but reduce usage of video "
		"memory and disk space; use it if you have an old video card");

	ce_optparse_add(optparse, "thread_count",
		CE_TYPE_INT, (const int[]){ce_online_cpu_count()}, false, "j", "jobs",
		"allow THREAD_COUNT jobs at once; if this option is not "
		"specified, the value will be detected automatically depending on the "
		"number of CPUs you have (or the number of cores your CPU have)");

	ce_optparse_add(optparse, "show_axes",
		CE_TYPE_BOOL, NULL, false, NULL, "show-axes",
		"show x (red), y (green), z (blue) axes");

	ce_optparse_add(optparse, "show_fps",
		CE_TYPE_BOOL, NULL, false, NULL, "show-fps",
		"show FPS counter");

	ce_optparse_add_control(optparse, "alt+tab", "minimize fullscreen window");
	ce_optparse_add_control(optparse, "alt+enter", "toggle fullscreen mode");
	ce_optparse_add_control(optparse, "b", "toggle bounding boxes "
										"(comprehensive/comprehensive+bones/none)");
	ce_optparse_add_control(optparse, "keyboard arrows", "move camera");
	ce_optparse_add_control(optparse, "mouse right button + motion", "rotate camera");
	ce_optparse_add_control(optparse, "mouse wheel", "zoom camera");

	return optparse;
}
