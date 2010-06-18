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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cethread.h"
#include "cesysteminfo.h"
#include "cesystemevent.h"
#include "ceoptionmanager.h"
#include "ceconfigmanager.h"
#include "ceeventmanager.h"
#include "cerendersystem.h"
#include "ceavcodec.h"
#include "cesoundmanager.h"
#include "cevideomanager.h"
#include "cetexturemanager.h"
#include "cemprmanager.h"
#include "cemobmanager.h"
#include "ceroot.h"

struct ce_root ce_root;

static void ce_root_system_event_handler(ce_system_event_type type)
{
	switch (type) {
	case CE_SYSTEM_EVENT_TYPE_INT:
		ce_logging_warning("root: interactive attention event received");
		break;
	case CE_SYSTEM_EVENT_TYPE_TERM:
		ce_logging_warning("root: termination event received");
		break;
	case CE_SYSTEM_EVENT_TYPE_CTRLC:
		ce_logging_warning("root: ctrl+c event received");
		break;
	case CE_SYSTEM_EVENT_TYPE_CTRLBREAK:
		ce_logging_warning("root: ctrl+break event received");
		break;
	case CE_SYSTEM_EVENT_TYPE_CLOSE:
		ce_logging_warning("root: close event received");
		break;
	case CE_SYSTEM_EVENT_TYPE_LOGOFF:
		ce_logging_warning("root: logoff event received");
		break;
	case CE_SYSTEM_EVENT_TYPE_SHUTDOWN:
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
	ce_inputsupply_del(ce_root.inputsupply);
	ce_timer_del(ce_root.timer);

	ce_scenemng_del(ce_root.scenemng);
	ce_figmng_del(ce_root.figmng);
	ce_mob_manager_term();
	ce_mpr_manager_term();
	ce_texture_manager_term();
	ce_video_manager_term();
	ce_sound_manager_term();
	ce_avcodec_term();
	ce_sound_system_del(ce_root.sound_system);
	ce_render_system_term();
	ce_renderwindow_del(ce_root.renderwindow);
	ce_thread_pool_term();
	ce_event_manager_term();
	ce_config_manager_term();
	ce_option_manager_term();
}

bool ce_root_init(ce_optparse* optparse, int argc, char* argv[])
{
	if (!ce_optparse_parse(optparse, argc, argv)) {
		return false;
	}

	if (!ce_system_info_check()) {
		return false;
	}

	atexit(ce_root_term);

	ce_root.show_bboxes = false;
	ce_root.comprehensive_bbox_only = true;
	ce_root.animation_fps = 15.0f;

	ce_option_manager_init(optparse);
	ce_config_manager_init();
	ce_event_manager_init();
	ce_thread_pool_init(ce_option_manager->thread_count);

	ce_root.renderwindow = ce_renderwindow_create(ce_option_manager->window_width,
		ce_option_manager->window_height, optparse->title->str);
	if (NULL == ce_root.renderwindow) {
		ce_logging_fatal("root: could not create window");
		return false;
	}

	// TODO: try without window creation
	if (ce_option_manager->list_video_modes) {
		ce_displaymng_dump_supported_modes_to_stdout(ce_root.renderwindow->displaymng);
		return false;
	}

	// TODO: try without window creation
	if (ce_option_manager->list_video_rotations) {
		ce_displaymng_dump_supported_rotations_to_stdout(ce_root.renderwindow->displaymng);
		return false;
	}

	// TODO: try without window creation
	if (ce_option_manager->list_video_reflections) {
		ce_displaymng_dump_supported_reflections_to_stdout(ce_root.renderwindow->displaymng);
		return false;
	}

	// FIXME: find better solution
	ce_root.renderwindow->restore_fullscreen = ce_option_manager->fullscreen;
	if (ce_option_manager->fullscreen) {
		ce_root.renderwindow->action = CE_RENDERWINDOW_ACTION_RESTORED;
	}

	ce_root.renderwindow->geometry[CE_RENDERWINDOW_STATE_FULLSCREEN].width = ce_option_manager->fullscreen_width;
	ce_root.renderwindow->geometry[CE_RENDERWINDOW_STATE_FULLSCREEN].height = ce_option_manager->fullscreen_height;

	ce_root.renderwindow->visual.bpp = ce_option_manager->fullscreen_bpp;
	ce_root.renderwindow->visual.rate = ce_option_manager->fullscreen_rate;

	ce_root.renderwindow->visual.rotation =
		ce_display_rotation_from_degrees(ce_option_manager->fullscreen_rotation);

	ce_root.renderwindow->visual.reflection =
		ce_display_reflection_from_bool(ce_option_manager->fullscreen_reflection_x,
										ce_option_manager->fullscreen_reflection_y);

	ce_render_system_init();
	ce_root.sound_system = ce_sound_system_new_platform();

	ce_avcodec_init();
	ce_sound_manager_init();
	ce_video_manager_init();

	ce_texture_manager_init();

	ce_mpr_manager_init();
	ce_mob_manager_init();

	ce_root.figmng = ce_figmng_new();

	const char* figure_resources[] = { "figures", "menus" };
	for (size_t i = 0; i < sizeof(figure_resources) / sizeof(figure_resources[0]); ++i) {
		char path[ce_option_manager->ei_path->length + 32];
		snprintf(path, sizeof(path), "%s/Res/%s.res",
			ce_option_manager->ei_path->str, figure_resources[i]);
		ce_figmng_register_resource(ce_root.figmng, path);
	}

	ce_root.scenemng = ce_scenemng_new();

	ce_root.timer = ce_timer_new();
	ce_root.inputsupply = ce_inputsupply_new(ce_root.renderwindow->inputcontext);
	ce_root.exit_event = ce_inputsupply_button(ce_root.inputsupply, CE_KB_ESCAPE);
	ce_root.switch_window_event = ce_inputsupply_single_front(ce_root.inputsupply,
		ce_inputsupply_shortcut(ce_root.inputsupply, "LAlt+Tab, RAlt+Tab"));
	ce_root.toggle_fullscreen_event = ce_inputsupply_single_front(ce_root.inputsupply,
		ce_inputsupply_shortcut(ce_root.inputsupply, "LAlt+Enter, RAlt+Enter"));
	ce_root.toggle_bbox_event = ce_inputsupply_single_front(ce_root.inputsupply,
		ce_inputsupply_shortcut(ce_root.inputsupply, "B"));

	ce_root.renderwindow_listener = (ce_renderwindow_listener)
									{.closed = ce_root_renderwindow_closed};
	ce_renderwindow_add_listener(ce_root.renderwindow,
								&ce_root.renderwindow_listener);

	ce_system_event_register(ce_root_system_event_handler);

	return true;
}

int ce_root_exec(void)
{
	ce_event_manager_create_queue();
	ce_renderwindow_show(ce_root.renderwindow);

	ce_timer_start(ce_root.timer);

	for (;;) {
		float elapsed = ce_timer_advance(ce_root.timer);

		ce_event_manager_process_events();
		ce_renderwindow_pump(ce_root.renderwindow);

		if (ce_root.done) {
			break;
		}

		ce_inputsupply_advance(ce_root.inputsupply, elapsed);

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

		ce_sound_manager_advance(elapsed);
		ce_video_manager_advance(elapsed);

		ce_scenemng_advance(ce_root.scenemng, elapsed);
		ce_scenemng_render(ce_root.scenemng);

		ce_graphic_context_swap(ce_root.renderwindow->graphic_context);
	}

	return EXIT_SUCCESS;
}
