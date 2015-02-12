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

#include "makeunique.hpp"
#include "exception.hpp"
#include "logging.hpp"
#include "thread.hpp"
#include "event.hpp"
#include "systeminfo.hpp"
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
    root_t::root_t(const ce_optparse_ptr_t& optparse, int argc, char* argv[]):
        singleton_t<root_t>(this),
        timer(make_timer()),
        m_done(false)
    {
        if (!ce_optparse_parse(optparse, argc, argv)) {
            throw game_error("root", "option parser failed");
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
            throw game_error("root", "PDP-endian systems are not supported");
        }

        m_option_manager = make_unique<option_manager_t>(optparse);

        ce_resource_manager_init();
        ce_config_manager_init();
        ce_event_manager_init();

        renderwindow = ce_renderwindow_create(m_option_manager->window_width, m_option_manager->window_height, optparse->title->str);
        if (NULL == renderwindow) {
            throw game_error("root", "could not create window");
        }

        // TODO: try without window creation
        if (m_option_manager->list_video_modes) {
            ce_displaymng_dump_supported_modes_to_stdout(renderwindow->displaymng);
            throw game_error("root", "dump_supported_modes_to_stdout failed");
        }

        // TODO: try without window creation
        if (m_option_manager->list_video_rotations) {
            ce_displaymng_dump_supported_rotations_to_stdout(renderwindow->displaymng);
            throw game_error("root", "dump_supported_rotations_to_stdout failed");
        }

        // TODO: try without window creation
        if (m_option_manager->list_video_reflections) {
            ce_displaymng_dump_supported_reflections_to_stdout(renderwindow->displaymng);
            throw game_error("root", "dump_supported_reflections_to_stdout failed");
        }

        renderwindow->geometry[CE_RENDERWINDOW_STATE_FULLSCREEN].width = m_option_manager->fullscreen_width;
        renderwindow->geometry[CE_RENDERWINDOW_STATE_FULLSCREEN].height = m_option_manager->fullscreen_height;

        renderwindow->visual.bpp = m_option_manager->fullscreen_bpp;
        renderwindow->visual.rate = m_option_manager->fullscreen_rate;

        renderwindow->visual.rotation = ce_display_rotation_from_degrees(m_option_manager->fullscreen_rotation);
        renderwindow->visual.reflection = ce_display_reflection_from_bool(m_option_manager->fullscreen_reflection_x, m_option_manager->fullscreen_reflection_y);

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
        m_thread_pool = make_unique<thread_pool_t>();

        m_input_supply = std::make_shared<input_supply_t>(renderwindow->input_context());
        m_exit_event = m_input_supply->push(input_button_t::kb_escape);
        m_switch_window_event = m_input_supply->single_front(shortcut(m_input_supply, "LAlt+Tab, RAlt+Tab"));
        m_toggle_fullscreen_event = m_input_supply->single_front(shortcut(m_input_supply, "LAlt+Enter, RAlt+Enter"));

        renderwindow_listener = {NULL, renderwindow_closed, NULL};
        ce_renderwindow_add_listener(renderwindow, &renderwindow_listener);
        ce_system_event_register(system_event_handler);
    }

    root_t::~root_t()
    {
        m_thread_pool.reset();
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

    int root_t::exec(const scene_manager_ptr_t& scene_manager)
    {
        renderwindow->show();
        if (m_option_manager->fullscreen) {
            renderwindow->toggle_fullscreen();
        }

        timer->start();

        while (!m_done) {
            const float elapsed = timer->advance();

            // 40 milliseconds - 25 times per second
            ce_event_manager_process_events_timeout(ce_thread_self(), 40);

            renderwindow->pump();

            m_input_supply->advance(elapsed);

            if (m_exit_event->triggered()) {
                m_done = true;
            }

            // TODO: win keys also
            if (m_switch_window_event->triggered() && CE_RENDERWINDOW_STATE_FULLSCREEN == renderwindow->state) {
                renderwindow->minimize();
            }

            if (m_toggle_fullscreen_event->triggered()) {
                renderwindow->toggle_fullscreen();
            }

            m_sound_manager->advance(elapsed);
            m_video_manager->advance(elapsed);

            scene_manager->advance(elapsed);
            scene_manager->render();

            ce_graphics_context_swap(renderwindow->graphics_context);
        }

        return EXIT_SUCCESS;
    }

    void root_t::system_event_handler(ce_system_event_type type)
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
        root_t::instance()->m_done = true;
    }

    void root_t::renderwindow_closed(void*)
    {
        root_t::instance()->m_done = true;
    }
}
