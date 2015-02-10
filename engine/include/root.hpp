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

#include <memory>

#include "singleton.hpp"
#include "timer.hpp"
#include "input.hpp"
#include "renderwindow.hpp"
#include "scenemng.hpp"
#include "optparse.hpp"

namespace cursedearth
{
    class ce_root: public singleton_t<ce_root>
    {
    public:
        ce_root(ce_optparse* optparse, int argc, char* argv[]);
        ~ce_root();

        int exec();

        bool done = false;
        bool show_bboxes = false;
        bool comprehensive_bbox_only = true;
        float animation_fps = 15.0f;
        ce_renderwindow* renderwindow;
        ce_scenemng* scenemng;
        ce_timer* timer;
        input_supply_ptr_t input_supply;
        input_event_const_ptr_t exit_event;
        input_event_const_ptr_t switch_window_event;
        input_event_const_ptr_t toggle_fullscreen_event;
        input_event_const_ptr_t toggle_bbox_event;
        ce_renderwindow_listener renderwindow_listener;
        std::unique_ptr<class option_manager_t> m_option_manager;
        std::unique_ptr<class sound_system_t> m_sound_system;
        std::unique_ptr<class sound_mixer_t> m_sound_mixer;
        std::unique_ptr<class sound_manager_t> m_sound_manager;
        std::unique_ptr<class video_manager_t> m_video_manager;
        std::unique_ptr<class thread_pool_t> m_thread_pool;
    };
}

#endif
