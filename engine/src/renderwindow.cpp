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
#include <cstring>
#include <cstdio>
#include <algorithm>

#include "renderwindow.hpp"

namespace cursedearth
{
    render_window_t* ce_renderwindow_new(ce_renderwindow_vtable vtable, size_t size, ...)
    {
        render_window_t* renderwindow = new render_window_t;

        memset(&renderwindow->visual, 0, sizeof(ce_renderwindow_visual));

        renderwindow->impl = new uint8_t[size];
        memset(renderwindow->impl, 0, size);

        renderwindow->vtable = vtable;
        renderwindow->size = size;

        renderwindow->displaymng = NULL;
        renderwindow->graphics_context = NULL;
        renderwindow->m_input_context = std::make_shared<input_context_t>();
        renderwindow->listeners = ce_vector_new();

        va_list args;
        va_start(args, size);

        renderwindow->geometry[renderwindow->state].width = std::max(400, va_arg(args, int));
        renderwindow->geometry[renderwindow->state].height = std::max(300, va_arg(args, int));

        bool ok = (*vtable.ctor)(renderwindow, args);

        va_end(args);

        if (!ok) {
            ce_renderwindow_del(renderwindow);
            return NULL;
        }

        return renderwindow;
    }

    void ce_renderwindow_del(render_window_t* renderwindow)
    {
        if (NULL != renderwindow) {
            (*renderwindow->vtable.dtor)(renderwindow);
            ce_vector_del(renderwindow->listeners);
            delete[] static_cast<uint8_t*>(renderwindow->impl);
            delete renderwindow;
        }
    }

    void render_window_t::show()
    {
        (*vtable.show)(this);
    }

    void render_window_t::minimize()
    {
        if (CE_RENDERWINDOW_STATE_FULLSCREEN == state) {
            // exit from fullscreen before minimizing
            toggle_fullscreen();
        }
        (*vtable.minimize)(this);
    }

    void render_window_t::toggle_fullscreen()
    {
        if (CE_RENDERWINDOW_STATE_WINDOW == state) {
            state = CE_RENDERWINDOW_STATE_FULLSCREEN;

            size_t index = ce_displaymng_enter(displaymng,
                geometry[state].width, geometry[state].height,
                visual.bpp, visual.rate, visual.rotation, visual.reflection);

            const ce_displaymode* mode = (const ce_displaymode*)displaymng->supported_modes->items[index];

            geometry[state].width = mode->width;
            geometry[state].height = mode->height;

            visual.bpp = mode->bpp;
            visual.rate = mode->rate;
            // TODO: rotation, reflection
        } else {
            state = CE_RENDERWINDOW_STATE_WINDOW;
            ce_displaymng_exit(displaymng);
        }

        if (NULL != vtable.toggle_fullscreen) {
            (*vtable.toggle_fullscreen)(this);
        }
    }

    void render_window_t::pump()
    {
        // reset pointer offset every frame
        m_input_context->pointer_offset = CE_VEC2_ZERO;

        // reset wheel buttons: there are no "WheelRelease" events in most cases
        m_input_context->buttons[static_cast<size_t>(input_button_t::mb_wheelup)] = false;
        m_input_context->buttons[static_cast<size_t>(input_button_t::mb_wheeldown)] = false;

        (*vtable.pump)(this);
    }

    void ce_renderwindow_add_listener(render_window_t* renderwindow, ce_renderwindow_listener* listener)
    {
        ce_vector_push_back(renderwindow->listeners, listener);
    }

    void ce_renderwindow_emit_resized(render_window_t* renderwindow, int width, int height)
    {
        for (size_t i = 0; i < renderwindow->listeners->count; ++i) {
            ce_renderwindow_listener* listener = (ce_renderwindow_listener*)renderwindow->listeners->items[i];
            if (NULL != listener->resized) {
                (*listener->resized)(listener->listener, width, height);
            }
        }
    }

    void ce_renderwindow_emit_closed(render_window_t* renderwindow)
    {
        for (size_t i = 0; i < renderwindow->listeners->count; ++i) {
            ce_renderwindow_listener* listener = (ce_renderwindow_listener*)renderwindow->listeners->items[i];
            if (NULL != listener->closed) {
                (*listener->closed)(listener->listener);
            }
        }
    }
}
