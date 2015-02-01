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

#ifndef CE_RENDERWINDOW_HPP
#define CE_RENDERWINDOW_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/noncopyable.hpp>

#include "vector.hpp"
#include "input.hpp"
#include "display.hpp"
#include "graphiccontext.hpp"

namespace cursedearth
{
    enum render_window_state_t
    {
        RENDER_WINDOW_STATE_WINDOW,
        RENDER_WINDOW_STATE_FULLSCREEN,
        RENDER_WINDOW_STATE_COUNT
    };

    enum render_window_action_t
    {
        RENDER_WINDOW_ACTION_NONE,
        RENDER_WINDOW_ACTION_MINIMIZE,
        RENDER_WINDOW_ACTION_RESTORED,
        RENDER_WINDOW_ACTION_COUNT
    };

    struct render_window_geometry_t
    {
        int x, y;
        int width, height;
    };

    struct render_window_visual_t
    {
        int bpp, rate;
        display_rotation_t rotation;
        display_reflection_t reflection;
    };

    struct ce_renderwindow_listener
    {
        void (*resized)(void* listener, int width, int height);
        void (*closed)(void* listener);
        void* listener;
    };

    class render_window_t: boost::noncopyable
    {
    public:
        render_window_t();
        virtual ~render_window_t();

        void add_listener(ce_renderwindow_listener* listener);

        void show();
        void minimize();
        void toggle_fullscreen();

        void pump();

        void emit_resized(int width, int height);
        void emit_closed();

        input_context_const_ptr_t get_input_context() const { return m_input_context; }

    private:
        virtual void do_show() = 0;
        virtual void do_minimize() = 0;
        virtual void prepare_fullscreen() = 0;
        virtual void before_enter_in_fullscreen() = 0;
        virtual void after_enter_in_fullscreen() = 0;
        virtual void before_exit_from_fullscreen() = 0;
        virtual void after_exit_fromfullscreen() = 0;
        virtual void done_fullscreen() = 0;
        virtual void do_pump() = 0;

    private:
        render_window_state_t state;
        render_window_action_t action;
        render_window_geometry_t geometry[RENDER_WINDOW_STATE_COUNT];
        render_window_visual_t visual;
        bool restore_fullscreen; // request to switch in fullscreen mode when the window was restored
        display_manager_t* displaymng;
        ce_graphic_context* graphic_context;
        input_context_ptr_t m_input_context;
        std::unordered_map<unsigned long int, input_button_t> keymap;
        ce_vector* listeners;
    };

    typedef std::shared_ptr<render_window_t> render_window_ptr_t;

    render_window_ptr_t create_render_window(int width, int height, const std::string& title);
}

#endif /* CE_RENDERWINDOW_HPP */
