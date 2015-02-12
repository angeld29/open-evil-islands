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
#include "graphicscontext.hpp"

namespace cursedearth
{
    typedef struct {
        void (*resized)(void* listener, size_t width, size_t height);
        void (*closed)(void* listener);
        void* listener;
    } ce_renderwindow_listener;

    class render_window_t: boost::noncopyable
    {
        struct ce_renderwindow_geometry
        {
            int x, y;
            int width, height;
        };

        struct ce_renderwindow_visual
        {
            int bpp, rate;
            ce_display_rotation rotation;
            ce_display_reflection reflection;
        };

    protected:
        enum ce_renderwindow_state {
            CE_RENDERWINDOW_STATE_WINDOW,
            CE_RENDERWINDOW_STATE_FULLSCREEN,
            CE_RENDERWINDOW_STATE_COUNT
        };

    public:
        explicit render_window_t(const std::string& title);
        virtual ~render_window_t();

        bool fullscreen() const { return CE_RENDERWINDOW_STATE_FULLSCREEN == state; }
        input_context_const_ptr_t input_context() const { return m_input_context; }

        void show();
        void minimize();
        void toggle_fullscreen();
        void pump();
        void swap();

        void emit_resized(size_t width, size_t height);
        void emit_closed();

        void add_listener(ce_renderwindow_listener* listener);

    private:
        virtual void do_show() = 0;
        virtual void do_minimize() = 0;
        virtual void do_toggle_fullscreen() = 0;
        virtual void do_pump() = 0;

    protected:
        const std::string m_title;
        ce_renderwindow_state state = CE_RENDERWINDOW_STATE_WINDOW;
        ce_renderwindow_geometry geometry[CE_RENDERWINDOW_STATE_COUNT];
        ce_renderwindow_visual visual;
        input_context_ptr_t m_input_context;
        std::unordered_map<unsigned long, input_button_t> m_input_map; // map platform-depended buttons to our buttons
        ce_displaymng* displaymng;
        ce_graphics_context* graphics_context;
        ce_vector* listeners;
    };

    typedef std::shared_ptr<render_window_t> render_window_ptr_t;

    render_window_ptr_t make_render_window(const std::string& title);
}

#endif
