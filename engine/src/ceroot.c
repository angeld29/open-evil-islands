/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cethread.h"
#include "cesysteminfo.h"
#include "cesystemevent.h"
#include "ceroot.h"

struct ce_root ce_root;

static void ce_root_systemevent_handler(ce_systemevent_type type)
{
	switch (type) {
	case CE_SYSTEMEVENT_TYPE_INT:
		ce_logging_warning("root: interactive attention event received");
		break;
	case CE_SYSTEMEVENT_TYPE_TERM:
		ce_logging_warning("root: termination event received");
		break;
	case CE_SYSTEMEVENT_TYPE_CTRLC:
		ce_logging_warning("root: ctrl+c event received");
		break;
	case CE_SYSTEMEVENT_TYPE_CTRLBREAK:
		ce_logging_warning("root: ctrl+break event received");
		break;
	case CE_SYSTEMEVENT_TYPE_CLOSE:
		ce_logging_warning("root: close event received");
		break;
	case CE_SYSTEMEVENT_TYPE_LOGOFF:
		ce_logging_warning("root: logoff event received");
		break;
	case CE_SYSTEMEVENT_TYPE_SHUTDOWN:
		ce_logging_warning("root: shutdown event received");
		break;
	default:
		ce_logging_critical("root: unknown event received");
		assert(false);
	}

	ce_logging_write("root: exiting sanely...");
	ce_root.done = true;
}

static void ce_root_renderwindow_closed(void* listener)
{
	ce_unused(listener);
	ce_root.done = true;
}

static void ce_root_term(void)
{
	if (ce_root.inited) {
		ce_input_supply_del(ce_root.input_supply);
		ce_scenemng_del(ce_root.scenemng);
		ce_timer_del(ce_root.timer);
		ce_rendersystem_del(ce_root.rendersystem);
		ce_renderwindow_del(ce_root.renderwindow);
		ce_root.inited = false;
	}
}

bool ce_root_init(ce_optparse* optparse)
{
	assert(!ce_root.inited && "the root subsystem has already been inited");

	ce_root.inited = true;
	atexit(ce_root_term);

	ce_systeminfo_display();

	if (!ce_systeminfo_ensure()) {
		return false;
	}

	bool fs_reflection_x, fs_reflection_y, list_vm, list_vrot, list_vref;
	int width, height, fs_rotation;
	const char *ei_path, *ce_path;

	ce_optparse_get(optparse, "ei_path", &ei_path);
	ce_optparse_get(optparse, "ce_path", &ce_path);
	ce_optparse_get(optparse, "width", &width);
	ce_optparse_get(optparse, "height", &height);
	ce_optparse_get(optparse, "fs_rotation", &fs_rotation);
	ce_optparse_get(optparse, "fs_reflection_x", &fs_reflection_x);
	ce_optparse_get(optparse, "fs_reflection_y", &fs_reflection_y);
	ce_optparse_get(optparse, "list_vm", &list_vm);
	ce_optparse_get(optparse, "list_vrot", &list_vrot);
	ce_optparse_get(optparse, "list_vref", &list_vref);
	ce_optparse_get(optparse, "thread_count", &ce_root.thread_count);
	ce_optparse_get(optparse, "terrain_tiling", &ce_root.terrain_tiling);
	ce_optparse_get(optparse, "show_axes", &ce_root.show_axes);
	ce_optparse_get(optparse, "show_fps", &ce_root.show_fps);

	ce_root.show_bboxes = false;
	ce_root.comprehensive_bbox_only = true;
	ce_root.anmfps = 15.0f;

	ce_root.renderwindow = ce_renderwindow_create(width, height, optparse->title->str);
	if (NULL == ce_root.renderwindow) {
		ce_logging_fatal("root: could not create a window");
		return false;
	}

	if (list_vm) { // TODO: try without window creation
		ce_displaymng_dump_supported_modes_to_stdout(ce_root.renderwindow->displaymng);
		return false;
	}

	if (list_vrot) { // TODO: try without window creation
		ce_displaymng_dump_supported_rotations_to_stdout(ce_root.renderwindow->displaymng);
		return false;
	}

	if (list_vref) { // TODO: try without window creation
		ce_displaymng_dump_supported_reflections_to_stdout(ce_root.renderwindow->displaymng);
		return false;
	}

	// FIXME: find better solution
	ce_optparse_get(optparse, "fullscreen", &ce_root.renderwindow->restore_fullscreen);
	if (ce_root.renderwindow->restore_fullscreen) {
		ce_root.renderwindow->action = CE_RENDERWINDOW_ACTION_RESTORED;
	}

	ce_optparse_get(optparse, "fs_width",
		&ce_root.renderwindow->geometry[CE_RENDERWINDOW_STATE_FULLSCREEN].width);
	ce_optparse_get(optparse, "fs_height",
		&ce_root.renderwindow->geometry[CE_RENDERWINDOW_STATE_FULLSCREEN].height);

	ce_optparse_get(optparse, "fs_bpp", &ce_root.renderwindow->visual.bpp);
	ce_optparse_get(optparse, "fs_rate", &ce_root.renderwindow->visual.rate);

	ce_root.renderwindow->visual.rotation =
		ce_display_rotation_from_degrees(fs_rotation);

	ce_root.renderwindow->visual.reflection =
		ce_display_reflection_from_bool(fs_reflection_x, fs_reflection_y);

	ce_root.rendersystem = ce_rendersystem_new();
	ce_root.timer = ce_timer_new();
	ce_root.scenemng = ce_scenemng_new(ei_path);

	ce_root.input_supply = ce_input_supply_new(ce_root.renderwindow->input_context);
	ce_root.exit_event = ce_input_supply_button(ce_root.input_supply, CE_KB_ESCAPE);
	ce_root.switch_window_event = ce_input_supply_single_front(ce_root.input_supply,
		ce_input_supply_shortcut(ce_root.input_supply, "LAlt+Tab, RAlt+Tab"));
	ce_root.toggle_fullscreen_event = ce_input_supply_single_front(ce_root.input_supply,
		ce_input_supply_shortcut(ce_root.input_supply, "LAlt+Enter, RAlt+Enter"));
	ce_root.toggle_bbox_event = ce_input_supply_single_front(ce_root.input_supply,
		ce_input_supply_shortcut(ce_root.input_supply, "B"));

	ce_root.renderwindow_listener = (ce_renderwindow_listener)
		{.closed = ce_root_renderwindow_closed};

	ce_renderwindow_add_listener(ce_root.renderwindow,
								&ce_root.renderwindow_listener);

	ce_systemevent_register(ce_root_systemevent_handler);

	ce_logging_write("root: using up to %d threads", ce_root.thread_count);
	ce_logging_write("root: terrain tiling %s",
		ce_root.terrain_tiling ? "enabled" : "disabled");

	return true;
}

int ce_root_exec(void)
{
	assert(ce_root.inited && "the root subsystem has not yet been inited");

	ce_renderwindow_show(ce_root.renderwindow);

	ce_timer_start(ce_root.timer);

	for (;;) {
		float elapsed = ce_timer_advance(ce_root.timer);

		ce_renderwindow_pump(ce_root.renderwindow);

		if (ce_root.done) {
			break;
		}

		ce_input_supply_advance(ce_root.input_supply, elapsed);

		if (ce_root.exit_event->triggered) {
			break;
		}

		if (ce_root.switch_window_event->triggered &&
				CE_RENDERWINDOW_STATE_FULLSCREEN == ce_root.renderwindow->state) {
			ce_renderwindow_minimize(ce_root.renderwindow);
		}

		if (ce_root.toggle_fullscreen_event->triggered) {
			ce_renderwindow_toggle_fullscreen(ce_root.renderwindow);
		}

		if (ce_root.toggle_bbox_event->triggered) {
			if (ce_root.show_bboxes) {
				if (ce_root.comprehensive_bbox_only) {
					ce_root.comprehensive_bbox_only = false;
				} else {
					ce_root.show_bboxes = false;
				}
			} else {
				ce_root.show_bboxes = true;
				ce_root.comprehensive_bbox_only = true;
			}
		}

		ce_scenemng_advance(ce_root.scenemng, elapsed);
		ce_scenemng_render(ce_root.scenemng);

		ce_context_swap(ce_root.renderwindow->context);
	}

	return EXIT_SUCCESS;
}

ce_optparse* ce_root_create_optparse(void)
{
	ce_optparse* optparse = ce_optparse_new();

	ce_optparse_add(optparse, "ei_path", CE_TYPE_STRING, ".", false,
		NULL, "ei-path", "path to EI directory (current by default)");
	ce_optparse_add(optparse, "ce_path", CE_TYPE_STRING, ".", false,
		NULL, "ce-path",
		"reserved for future use: path to CE directory (current by default)");

	ce_optparse_add(optparse, "width", CE_TYPE_INT, (int[]){1024}, false,
		NULL, "width", "desired window width");
	ce_optparse_add(optparse, "height", CE_TYPE_INT, (int[]){768}, false,
		NULL, "height", "desired window height");

	ce_optparse_add(optparse, "fullscreen", CE_TYPE_BOOL, NULL, false,
		"f", "fullscreen", "start program in fullscreen mode");

	ce_optparse_add(optparse, "fs_width", CE_TYPE_INT, NULL, false,
		NULL, "fullscreen-width",
		"desired window width in fullscreen mode (max available by default)");
	ce_optparse_add(optparse, "fs_height", CE_TYPE_INT, NULL, false,
		NULL, "fullscreen-height",
		"desired window height in fullscreen mode (max available by default)");

	ce_optparse_add(optparse, "fs_bpp", CE_TYPE_INT, NULL, false,
		NULL, "fullscreen-bpp", "desired bits per pixel (max available by default)");
	ce_optparse_add(optparse, "fs_rate", CE_TYPE_INT, NULL, false,
		NULL, "fullscreen-rate", "desired refresh rate (max available by default)");

	ce_optparse_add(optparse, "fs_rotation", CE_TYPE_INT, NULL, false,
		NULL, "fullscreen-rotation", "specify rotation in degrees (90, 180, 270)");
	ce_optparse_add(optparse, "fs_reflection_x", CE_TYPE_BOOL, NULL, false,
		NULL, "fullscreen-reflection-x", "reflect around x axis");
	ce_optparse_add(optparse, "fs_reflection_y", CE_TYPE_BOOL, NULL, false,
		NULL, "fullscreen-reflection-y", "reflect around y axis");

	ce_optparse_add(optparse, "list_vm", CE_TYPE_BOOL, NULL, false,
		NULL, "list-video-modes", "display supported video modes in "
		"format WIDTHxHEIGHT:BPP@RATE and exit (output to stdout)");
	ce_optparse_add(optparse, "list_vrot", CE_TYPE_BOOL, NULL, false,
		NULL, "list-video-rotations",
		"display supported rotations and exit (output to stdout)");
	ce_optparse_add(optparse, "list_vref", CE_TYPE_BOOL, NULL, false,
		NULL, "list-video-reflections",
		"display supported reflections and exit (output to stdout)");

	ce_optparse_add(optparse, "thread_count", CE_TYPE_INT,
		(const int[]){ce_thread_online_cpu_count()}, false,
		"j", "jobs", "allow THREAD_COUNT jobs at once; if this option is not "
		"specified, the value will be detected automatically depending on the "
		"number of CPUs you have (or the number of cores your CPU have)");
	ce_optparse_add(optparse, "terrain_tiling", CE_TYPE_BOOL, NULL, false,
		NULL, "terrain-tiling", "enable terrain tiling; very slow, but reduce "
		"usage of video memory and disk space; use it on old video cards");

	ce_optparse_add(optparse, "show_axes", CE_TYPE_BOOL, NULL, false,
		NULL, "show-axes", "show x (red), y (green), z (blue) axes");
	ce_optparse_add(optparse, "show_fps", CE_TYPE_BOOL, NULL, false,
		NULL, "show-fps", "show fps counter");

	ce_optparse_add_control(optparse, "alt + tab", "minimize fullscreen window");
	ce_optparse_add_control(optparse, "alt + enter", "toggle fullscreen mode");
	ce_optparse_add_control(optparse, "b", "toggle bounding boxes");
	ce_optparse_add_control(optparse, "keyboard arrows", "move camera");
	ce_optparse_add_control(optparse, "mouse right button + motion", "rotate camera");
	ce_optparse_add_control(optparse, "mouse wheel", "zoom camera");

	return optparse;
}
