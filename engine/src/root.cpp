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

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "makeunique.hpp"
#include "logging.hpp"
#include "thread.hpp"
#include "event.hpp"
#include "systeminfo.hpp"
#include "systemevent.hpp"
#include "optionmanager.hpp"
#include "resourcemanager.hpp"
#include "configmanager.hpp"
#include "rendersystem.hpp"
#include "soundsystem.hpp"
#include "soundmixer.hpp"
#include "avcodec.hpp"
#include "soundmanager.hpp"
#include "videomanager.hpp"
#include "texturemanager.hpp"
#include "shadermanager.hpp"
#include "mprmanager.hpp"
#include "mobmanager.hpp"
#include "mobloader.hpp"
#include "figuremanager.hpp"
#include "root.hpp"

namespace cursedearth
{
    void ce_root_system_event_handler(ce_system_event_type type)
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

        ce_logging_info("root: exiting sanely...");
        ce_root::instance()->done = true;
    }

    void ce_root_renderwindow_closed(void*)
    {
        ce_root::instance()->done = true;
    }

    ce_root::ce_root(ce_optparse* optparse, int argc, char* argv[]):
        singleton_t<ce_root>(this)
    {
        if (!ce_optparse_parse(optparse, argc, argv)) {
            throw std::runtime_error("root: option parser failed");
        }

        detect_system();

        switch (host_order()) {
        case endian_t::little:
            ce_logging_info("root: little-endian system detected");
            break;
        case endian_t::big:
            ce_logging_info("root: big-endian system detected");
            break;
        case endian_t::middle:
            throw std::runtime_error("PDP-endian systems are not supported");
        }

        m_option_manager = make_unique<option_manager_t>(optparse);

        ce_resource_manager_init();
        ce_config_manager_init();
        ce_event_manager_init();

        renderwindow = ce_renderwindow_create(option_manager_t::instance()->window_width, option_manager_t::instance()->window_height, optparse->title->str);
        if (NULL == renderwindow) {
            throw std::runtime_error("root: could not create window");
        }

        // TODO: try without window creation
        if (option_manager_t::instance()->list_video_modes) {
            ce_displaymng_dump_supported_modes_to_stdout(renderwindow->displaymng);
            throw std::runtime_error("root: dump_supported_modes_to_stdout failed");
        }

        // TODO: try without window creation
        if (option_manager_t::instance()->list_video_rotations) {
            ce_displaymng_dump_supported_rotations_to_stdout(renderwindow->displaymng);
            throw std::runtime_error("root: dump_supported_rotations_to_stdout failed");
        }

        // TODO: try without window creation
        if (option_manager_t::instance()->list_video_reflections) {
            ce_displaymng_dump_supported_reflections_to_stdout(renderwindow->displaymng);
            throw std::runtime_error("root: dump_supported_reflections_to_stdout failed");
        }

        // FIXME: find better solution
        renderwindow->restore_fullscreen = option_manager_t::instance()->fullscreen;
        if (option_manager_t::instance()->fullscreen) {
            renderwindow->action = CE_RENDERWINDOW_ACTION_RESTORED;
        }

        renderwindow->geometry[CE_RENDERWINDOW_STATE_FULLSCREEN].width = option_manager_t::instance()->fullscreen_width;
        renderwindow->geometry[CE_RENDERWINDOW_STATE_FULLSCREEN].height = option_manager_t::instance()->fullscreen_height;

        renderwindow->visual.bpp = option_manager_t::instance()->fullscreen_bpp;
        renderwindow->visual.rate = option_manager_t::instance()->fullscreen_rate;

        renderwindow->visual.rotation = ce_display_rotation_from_degrees(option_manager_t::instance()->fullscreen_rotation);
        renderwindow->visual.reflection = ce_display_reflection_from_bool(option_manager_t::instance()->fullscreen_reflection_x, option_manager_t::instance()->fullscreen_reflection_y);

        ce_render_system_init();

        m_sound_system = make_unique<sound_system_t>();
        m_sound_mixer = make_unique<sound_mixer_t>();
        m_sound_manager = make_unique<sound_manager_t>();
        m_video_manager = make_unique<video_manager_t>();

        initialize_avcodec();

        ce_texture_manager_init();
        ce_shader_manager_init();

        ce_mpr_manager_init();
        ce_mob_manager_init();
        ce_mob_loader_init();

        ce_figure_manager_init();
        scenemng = ce_scenemng_new();
        m_thread_pool = make_unique<thread_pool_t>();

        timer = ce_timer_new();
        input_supply = std::make_shared<input_supply_t>(renderwindow->input_context());
        exit_event = input_supply->push(input_button_t::kb_escape);
        switch_window_event = input_supply->single_front(shortcut(input_supply, "LAlt+Tab, RAlt+Tab"));
        toggle_fullscreen_event = input_supply->single_front(shortcut(input_supply, "LAlt+Enter, RAlt+Enter"));
        toggle_bbox_event = input_supply->single_front(input_supply->push(input_button_t::kb_b));

        renderwindow_listener = {NULL, ce_root_renderwindow_closed, NULL};
        ce_renderwindow_add_listener(renderwindow, &renderwindow_listener);
        ce_system_event_register(ce_root_system_event_handler);
    }

    ce_root::~ce_root()
    {
        ce_timer_del(timer);
        m_thread_pool.reset();
        ce_scenemng_del(scenemng);
        ce_figure_manager_term();
        ce_mob_loader_term();
        ce_mob_manager_term();
        ce_mpr_manager_term();
        ce_shader_manager_term();
        ce_texture_manager_term();
        m_video_manager.reset();
        m_sound_manager.reset();
        terminate_avcodec();
        m_sound_mixer.reset();
        m_sound_system.reset();
        ce_render_system_term();
        ce_renderwindow_del(renderwindow);
        ce_event_manager_term();
        ce_config_manager_term();
        ce_resource_manager_term();
        m_option_manager.reset();
    }

    int ce_root::exec()
    {
        ce_renderwindow_show(renderwindow);
        ce_timer_start(timer);

        for (;;) {
            float elapsed = ce_timer_advance(timer);

            // 40 milliseconds - 25 times per second
            ce_event_manager_process_events_timeout(ce_thread_self(), 40);

            renderwindow->pump();

            if (done) {
                break;
            }

            input_supply->advance(elapsed);

            if (exit_event->triggered()) {
                break;
            }

            if (switch_window_event->triggered() && CE_RENDERWINDOW_STATE_FULLSCREEN == renderwindow->state) {
                ce_renderwindow_minimize(renderwindow);
            }

            if (toggle_fullscreen_event->triggered()) {
                ce_renderwindow_toggle_fullscreen(renderwindow);
            }

            if (toggle_bbox_event->triggered()) {
                if (show_bboxes) {
                    if (comprehensive_bbox_only) {
                        comprehensive_bbox_only = false;
                    } else {
                        show_bboxes = false;
                    }
                } else {
                    show_bboxes = true;
                    comprehensive_bbox_only = true;
                }
            }

            m_sound_manager->advance(elapsed);
            m_video_manager->advance(elapsed);

            ce_scenemng_advance(scenemng, elapsed);
            ce_scenemng_render(scenemng);

            ce_graphics_context_swap(renderwindow->graphics_context);
        }

        return EXIT_SUCCESS;
    }
}
