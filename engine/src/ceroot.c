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
		ce_input_event_supply_del(ce_root.event_supply);
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

	const char* ei_path;
	int width, height, fs_width, fs_height;

	ce_optparse_get(optparse, "ei_path", &ei_path);
	ce_optparse_get(optparse, "width", &width);
	ce_optparse_get(optparse, "height", &height);
	ce_optparse_get(optparse, "fs_width", &fs_width);
	ce_optparse_get(optparse, "fs_height", &fs_height);
	ce_optparse_get(optparse, "terrain_tiling", &ce_root.terrain_tiling);
	ce_optparse_get(optparse, "thread_count", &ce_root.thread_count);

	ce_root.show_axes = true;
	ce_root.show_bboxes = false;
	ce_root.comprehensive_bbox_only = true;
	ce_root.anmfps = 15.0f;

	ce_root.renderwindow = ce_renderwindow_create(width, height, optparse->title->str);
	if (NULL == ce_root.renderwindow) {
		ce_logging_fatal("root: could not create a window");
		return false;
	}

	ce_root.rendersystem = ce_rendersystem_new();
	ce_root.timer = ce_timer_new();
	ce_root.scenemng = ce_scenemng_new(ei_path);

	ce_root.event_supply = ce_input_event_supply_new(ce_root.renderwindow->input_context);
	ce_root.exit_event = ce_input_event_supply_button(ce_root.event_supply, CE_KB_ESCAPE);
	ce_root.switch_window_event = ce_input_event_supply_single_front(ce_root.event_supply,
		ce_input_event_supply_shortcut(ce_root.event_supply, "LAlt+Tab, RAlt+Tab"));
	ce_root.toggle_fullscreen_event = ce_input_event_supply_single_front(ce_root.event_supply,
		ce_input_event_supply_shortcut(ce_root.event_supply, "LAlt+Enter, RAlt+Enter"));
	ce_root.toggle_bbox_event = ce_input_event_supply_single_front(ce_root.event_supply,
		ce_input_event_supply_shortcut(ce_root.event_supply, "B"));

	ce_root.renderwindow_listener.closed = ce_root_renderwindow_closed;
	ce_root.renderwindow_listener.listener = NULL;

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

		ce_input_event_supply_advance(ce_root.event_supply, elapsed);

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
	ce_optparse_add(optparse, "terrain_tiling", CE_TYPE_BOOL, NULL, false,
		NULL, "terrain-tiling", "enable terrain tiling; very slow, but reduce "
		"usage of video memory and disk space; use it on old video cards");
	ce_optparse_add(optparse, "thread_count", CE_TYPE_INT,
		(const int[]){ce_thread_online_cpu_count()}, false,
		"j", "jobs", "allow THREAD_COUNT jobs at once; if this option is not "
		"specified, the value will be detected automatically depending on the "
		"number of CPUs you have (or the number of cores your CPU have)");

	ce_optparse_add_control(optparse, "alt + tab", "minimize fullscreen window");
	ce_optparse_add_control(optparse, "alt + enter", "toggle fullscreen mode");
	ce_optparse_add_control(optparse, "b", "toggle bounding boxes");
	ce_optparse_add_control(optparse, "keyboard arrows", "move camera");
	ce_optparse_add_control(optparse, "mouse right button + motion", "rotate camera");
	ce_optparse_add_control(optparse, "mouse wheel", "zoom camera");

	return optparse;
}
