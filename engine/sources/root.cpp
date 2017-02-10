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

#include "exception.hpp"
#include "logging.hpp"
#include "event.hpp"
#include "systeminfo.hpp"
#include "systemevent.hpp"
#include "resourcemanager.hpp"
#include "configmanager.hpp"
#include "rendersystem.hpp"
#include "avcodec.hpp"
#include "texturemanager.hpp"
#include "shadermanager.hpp"
#include "mprmanager.hpp"
#include "mobmanager.hpp"
#include "mobloader.hpp"
#include "figuremanager.hpp"
#include "root.hpp"

namespace cursedearth
{
    root_t::root_t(const ce_optparse_ptr_t& option_parser, int argc, char* argv[]):
        singleton_t<root_t>(this),
        timer(make_timer()),
        m_done(false),
        m_input_context(std::make_shared<input_context_t>()),
        m_input_supply(std::make_shared<input_supply_t>(m_input_context)),
        m_exit_event(m_input_supply->push(input_button_t::kb_escape)),
        m_minimize_fullscreen_event(m_input_supply->single_front(shortcut(m_input_supply, "LAlt+Tab, RAlt+Tab, LMeta, RMeta"))),
        m_toggle_fullscreen_event(m_input_supply->single_front(shortcut(m_input_supply, "LAlt+Enter, RAlt+Enter")))
    {
        if (!ce_optparse_parse(option_parser, argc, argv)) {
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

        m_option_manager = make_option_manager(option_parser);

        ce_resource_manager_init();
        ce_config_manager_init();
        ce_event_manager_init();

        m_render_window = make_render_window(option_parser->title->str, m_input_context);

        // TODO: try without window creation
        if (m_option_manager->list_video_modes) {
            //ce_displaymng_dump_supported_modes_to_stdout(renderwindow->displaymng);
            throw game_error("root", "dump_supported_modes_to_stdout failed");
        }

        // TODO: try without window creation
        if (m_option_manager->list_video_rotations) {
            //ce_displaymng_dump_supported_rotations_to_stdout(renderwindow->displaymng);
            throw game_error("root", "dump_supported_rotations_to_stdout failed");
        }

        // TODO: try without window creation
        if (m_option_manager->list_video_reflections) {
            //ce_displaymng_dump_supported_reflections_to_stdout(renderwindow->displaymng);
            throw game_error("root", "dump_supported_reflections_to_stdout failed");
        }

        ce_render_system_init();

        m_sound_system = make_sound_system();
        m_sound_mixer = make_sound_mixer();
        m_sound_manager = make_sound_manager();
        m_video_manager = make_video_manager();

        initialize_avcodec();

        ce_texture_manager_init();
        ce_shader_manager_init();

        ce_mpr_manager_init();
        ce_mob_manager_init();
        ce_mob_loader_init();

        ce_figure_manager_init();
        m_thread_pool = make_thread_pool();
        m_scene_manager = make_scene_manager(m_input_context, option_parser);

        m_render_window->closed.connect([this] { m_done = true; });
        m_render_window->resized.connect([this] (size_t width, size_t height) {
            m_scene_manager->resize(width, height);
        });

        add_system_event_handler([this] (system_event_type_t type) {
            switch (type) {
            case system_event_type_t::interrupt:
                ce_logging_warning("root: interruption event received");
                break;
            case system_event_type_t::terminate:
                ce_logging_warning("root: termination event received");
                break;
            case system_event_type_t::ctrlc:
                ce_logging_warning("root: ctrl+c event received");
                break;
            case system_event_type_t::ctrlbreak:
                ce_logging_warning("root: ctrl+break event received");
                break;
            case system_event_type_t::close:
                ce_logging_warning("root: close event received");
                break;
            case system_event_type_t::logoff:
                ce_logging_warning("root: logoff event received");
                break;
            case system_event_type_t::shutdown:
                ce_logging_warning("root: shutdown event received");
                break;
            default:
                ce_logging_warning("root: unknown event received");
                assert(false);
            }
            m_done = true;
        });
    }

    root_t::~root_t()
    {
        m_scene_manager.reset();
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
        m_render_window.reset();
        ce_event_manager_term();
        ce_config_manager_term();
        ce_resource_manager_term();
        m_option_manager.reset();
    }

    int root_t::exec()
    {
        m_render_window->show();
        timer->start();

        while (!m_done) {
            const float elapsed = timer->advance();

            // 40 milliseconds - 25 times per second
            ce_event_manager_process_events_timeout(ce_thread_self(), 40);

            m_render_window->pump();

            m_input_supply->advance(elapsed);

            if (m_exit_event->triggered()) {
                m_done = true;
            }

            if (m_minimize_fullscreen_event->triggered() && m_render_window->fullscreen()) {
                m_render_window->minimize();
            }

            if (m_toggle_fullscreen_event->triggered()) {
                m_render_window->toggle_fullscreen();
            }

            m_sound_manager->advance(elapsed);
            m_video_manager->advance(elapsed);
            m_scene_manager->advance(elapsed);

            m_scene_manager->render();
            m_render_window->swap();
        }

        ce_logging_info("root: exiting sanely...");
        return EXIT_SUCCESS;
    }
}
