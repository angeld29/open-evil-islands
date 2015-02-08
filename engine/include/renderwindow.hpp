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

    enum ce_renderwindow_action {
        CE_RENDERWINDOW_ACTION_NONE,
        CE_RENDERWINDOW_ACTION_MINIMIZE,
        CE_RENDERWINDOW_ACTION_RESTORED,
        CE_RENDERWINDOW_ACTION_COUNT
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

    typedef struct ce_renderwindow ce_renderwindow;

    typedef struct {
        bool (*ctor)(ce_renderwindow* renderwindow, va_list args);
        void (*dtor)(ce_renderwindow* renderwindow);
        void (*show)(ce_renderwindow* renderwindow);
        void (*minimize)(ce_renderwindow* renderwindow);
        struct {
            void (*prepare)(ce_renderwindow* renderwindow);
            void (*before_enter)(ce_renderwindow* renderwindow);
            void (*after_enter)(ce_renderwindow* renderwindow);
            void (*before_exit)(ce_renderwindow* renderwindow);
            void (*after_exit)(ce_renderwindow* renderwindow);
            void (*done)(ce_renderwindow* renderwindow);
        } fullscreen;
        void (*pump)(ce_renderwindow* renderwindow);
    } ce_renderwindow_vtable;

    class ce_renderwindow
    {
    public:
        input_context_const_ptr_t input_context() const { return m_input_context; }

        void pump();

        ce_renderwindow_state state = CE_RENDERWINDOW_STATE_WINDOW;
        ce_renderwindow_action action = CE_RENDERWINDOW_ACTION_NONE;
        ce_renderwindow_geometry geometry[CE_RENDERWINDOW_STATE_COUNT];
        ce_renderwindow_visual visual;
        bool restore_fullscreen = false; // request to switch in fullscreen mode when the window was restored
        input_context_ptr_t m_input_context;
        std::unordered_map<unsigned long, input_button_t> m_input_map; // map platform-depended buttons to our buttons
        ce_displaymng* displaymng;
        ce_graphics_context* graphics_context;
        ce_vector* listeners;
        ce_renderwindow_vtable vtable;
        size_t size;
        void* impl;
    };

    ce_renderwindow* ce_renderwindow_new(ce_renderwindow_vtable vtable, size_t size, ...);
    void ce_renderwindow_del(ce_renderwindow* renderwindow);

    void ce_renderwindow_add_listener(ce_renderwindow* renderwindow, ce_renderwindow_listener* listener);

    void ce_renderwindow_show(ce_renderwindow* renderwindow);
    void ce_renderwindow_minimize(ce_renderwindow* renderwindow);

    void ce_renderwindow_toggle_fullscreen(ce_renderwindow* renderwindow);

    void ce_renderwindow_emit_resized(ce_renderwindow* renderwindow, int width, int height);
    void ce_renderwindow_emit_closed(ce_renderwindow* renderwindow);

    ce_renderwindow* ce_renderwindow_create(int width, int height, const char* title);
}

#endif
