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

static bool ce_root_inited;
static bool ce_root_done;

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
	ce_root_done = true;
}

static void ce_root_renderwindow_closed(void* listener)
{
	ce_unused(listener);
	ce_root_done = true;
}

struct ce_root ce_root;

bool ce_root_init(const char* ei_path)
{
	assert(!ce_root_inited && "the root subsystem has already been inited");

	if (!ce_alloc_init()) {
		ce_logging_fatal("root: could not initialize the memory subsystem, terminating");
		return false;
	}

	atexit(ce_alloc_term);

	ce_systeminfo_display();

	if (!ce_systeminfo_ensure()) {
		return false;
	}

	ce_root.show_axes = true;
	ce_root.show_bboxes = false;
	ce_root.comprehensive_bbox_only = true;
	ce_root.terrain_tiling = false;
	ce_root.thread_count = ce_thread_online_cpu_count();
	ce_root.anmfps = 15.0f;
	ce_root.timer = ce_timer_new();
	ce_root.renderwindow = ce_renderwindow_create(1024, 768, "Cursed Earth");
	ce_root.rendersystem = ce_rendersystem_new();
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

	return ce_root_inited = true;
}

void ce_root_term(void)
{
	assert(ce_root_inited && "the root subsystem has not yet been inited");
	ce_root_inited = false;

	ce_scenemng_del(ce_root.scenemng), ce_root.scenemng = NULL;
	ce_input_event_supply_del(ce_root.event_supply), ce_root.event_supply = NULL;
	ce_rendersystem_del(ce_root.rendersystem), ce_root.rendersystem = NULL;
	ce_renderwindow_del(ce_root.renderwindow), ce_root.renderwindow = NULL;
	ce_timer_del(ce_root.timer), ce_root.timer = NULL;
}

void ce_root_exec(void)
{
	assert(ce_root_inited && "the root subsystem has not yet been inited");

	ce_renderwindow_show(ce_root.renderwindow);

	ce_timer_start(ce_root.timer);

	for (;;) {
		float elapsed = ce_timer_advance(ce_root.timer);

		ce_renderwindow_pump(ce_root.renderwindow);

		if (ce_root_done) {
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
}
