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

#include "timer.hpp"
#include "input.hpp"
#include "systemevent.hpp"
#include "optionmanager.hpp"
#include "renderwindow.hpp"
#include "soundsystem.hpp"
#include "soundmixer.hpp"
#include "soundmanager.hpp"
#include "videomanager.hpp"
#include "threadpool.hpp"
#include "scenemanager.hpp"

namespace cursedearth
{
    class root_t: public singleton_t<root_t>
    {
    public:
        root_t(const ce_optparse_ptr_t&, int argc, char* argv[]);
        ~root_t();

        int exec();

    private:
        static void system_event_handler(ce_system_event_type);
        static void renderwindow_closed(void*);

    public:
        float animation_fps = 15.0f;
        timer_ptr_t timer;

    private:
        std::atomic<bool> m_done;
        input_context_ptr_t m_input_context;
        input_supply_ptr_t m_input_supply;
        input_event_const_ptr_t m_exit_event;
        input_event_const_ptr_t m_switch_window_event;
        input_event_const_ptr_t m_toggle_fullscreen_event;
        option_manager_ptr_t m_option_manager;
        render_window_ptr_t m_render_window;
        sound_system_ptr_t m_sound_system;
        sound_mixer_ptr_t m_sound_mixer;
        sound_manager_ptr_t m_sound_manager;
        video_manager_ptr_t m_video_manager;
        thread_pool_ptr_t m_thread_pool;
        scene_manager_ptr_t m_scene_manager;
    };
}

#endif
