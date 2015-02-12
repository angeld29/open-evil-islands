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

#include <boost/signals2.hpp>

#include "singleton.hpp"
#include "input.hpp"
#include "display.hpp"
#include "graphicscontext.hpp"

namespace cursedearth
{
    class render_window_t: public singleton_t<render_window_t>
    {
    protected:
        enum state_t {
            state_window,
            state_fullscreen,
            state_count
        };

    public:
        render_window_t(const std::string& title, const input_context_ptr_t&);
        virtual ~render_window_t() = default;

        bool fullscreen() const { return state_fullscreen == m_state; }

        void show();
        void minimize();
        void toggle_fullscreen();
        void pump();
        void swap();

    public:
        boost::signals2::signal<void (size_t width, size_t height)> resized;
        boost::signals2::signal<void ()> closed;

    private:
        virtual void do_show() = 0;
        virtual void do_minimize() = 0;
        virtual void do_toggle_fullscreen() = 0;
        virtual void do_pump() = 0;

    private:
        struct geometry_t
        {
            int x, y;
            int width, height;
        };

        struct visual_t
        {
            int bpp, rate;
            ce_display_rotation rotation;
            ce_display_reflection reflection;
        };

    protected:
        const std::string m_title;
        input_context_ptr_t m_input_context;
        state_t m_state = state_window;
        geometry_t m_geometry[state_count];
        visual_t m_visual;
        std::unordered_map<unsigned long, input_button_t> m_input_map; // map platform-depended buttons to our buttons
        ce_displaymng* m_display_manager;
        ce_graphics_context* m_graphics_context;
    };

    typedef std::unique_ptr<render_window_t> render_window_ptr_t;

    render_window_ptr_t make_render_window(const std::string& title, const input_context_ptr_t&);
}

#endif
