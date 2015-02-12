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

#include <cstdarg>
#include <unordered_map>

#include "vector.hpp"
#include "input.hpp"
#include "display.hpp"
#include "graphicscontext.hpp"

namespace cursedearth
{
    enum ce_renderwindow_state {
        CE_RENDERWINDOW_STATE_WINDOW,
        CE_RENDERWINDOW_STATE_FULLSCREEN,
        CE_RENDERWINDOW_STATE_COUNT
    };

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

    typedef struct {
        void (*resized)(void* listener, int width, int height);
        void (*closed)(void* listener);
        void* listener;
    } ce_renderwindow_listener;

    typedef struct {
        bool (*ctor)(class render_window_t* renderwindow, va_list args);
        void (*dtor)(class render_window_t* renderwindow);
        void (*show)(class render_window_t* renderwindow);
        void (*minimize)(class render_window_t* renderwindow);
        void (*toggle_fullscreen)(class render_window_t* renderwindow);
        void (*pump)(class render_window_t* renderwindow);
    } ce_renderwindow_vtable;

    class render_window_t
    {
    public:
        input_context_const_ptr_t input_context() const { return m_input_context; }

        void show();
        void minimize();
        void toggle_fullscreen();
        void pump();

        ce_renderwindow_state state = CE_RENDERWINDOW_STATE_WINDOW;
        ce_renderwindow_geometry geometry[CE_RENDERWINDOW_STATE_COUNT];
        ce_renderwindow_visual visual;
        input_context_ptr_t m_input_context;
        std::unordered_map<unsigned long, input_button_t> m_input_map; // map platform-depended buttons to our buttons
        ce_displaymng* displaymng;
        ce_graphics_context* graphics_context;
        ce_vector* listeners;
        ce_renderwindow_vtable vtable;
        size_t size;
        void* impl;
    };

    render_window_t* ce_renderwindow_new(ce_renderwindow_vtable vtable, size_t size, ...);
    void ce_renderwindow_del(render_window_t* renderwindow);

    void ce_renderwindow_add_listener(render_window_t* renderwindow, ce_renderwindow_listener* listener);

    void ce_renderwindow_emit_resized(render_window_t* renderwindow, int width, int height);
    void ce_renderwindow_emit_closed(render_window_t* renderwindow);

    render_window_t* ce_renderwindow_create(int width, int height, const char* title);
}

#endif
