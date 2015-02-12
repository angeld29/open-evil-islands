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

#ifndef CE_ROOT_HPP
#define CE_ROOT_HPP

#include <atomic>
#include <memory>

#include "singleton.hpp"
#include "timer.hpp"
#include "input.hpp"
#include "systemevent.hpp"
#include "optionmanager.hpp"
#include "optparse.hpp"
#include "renderwindow.hpp"
#include "scenemanager.hpp"

namespace cursedearth
{
    class root_t: public singleton_t<root_t>
    {
    public:
        root_t(const ce_optparse_ptr_t&, int argc, char* argv[]);
        ~root_t();

        int exec(const scene_manager_ptr_t& scene_manager);

    private:
        static void system_event_handler(ce_system_event_type);
        static void renderwindow_closed(void*);

    public:
        float animation_fps = 15.0f;
        timer_ptr_t timer;
        render_window_t* renderwindow;

    private:
        std::atomic<bool> m_done;
        input_supply_ptr_t m_input_supply;
        input_event_const_ptr_t m_exit_event;
        input_event_const_ptr_t m_switch_window_event;
        input_event_const_ptr_t m_toggle_fullscreen_event;
        std::unique_ptr<class option_manager_t> m_option_manager;
        std::unique_ptr<class sound_system_t> m_sound_system;
        std::unique_ptr<class sound_mixer_t> m_sound_mixer;
        std::unique_ptr<class sound_manager_t> m_sound_manager;
        std::unique_ptr<class video_manager_t> m_video_manager;
        std::unique_ptr<class thread_pool_t> m_thread_pool;
        ce_renderwindow_listener renderwindow_listener;
    };
}

#endif
