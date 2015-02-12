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

#include <algorithm>

#include "optionmanager.hpp"
#include "renderwindow.hpp"

namespace cursedearth
{
    render_window_t::render_window_t(const std::string& title):
        m_title(title),
        m_input_context(std::make_shared<input_context_t>()),
        listeners(ce_vector_new())
    {
        geometry[CE_RENDERWINDOW_STATE_WINDOW].width = option_manager_t::instance()->window_width;
        geometry[CE_RENDERWINDOW_STATE_WINDOW].height = option_manager_t::instance()->window_height;

        geometry[CE_RENDERWINDOW_STATE_FULLSCREEN].width = option_manager_t::instance()->fullscreen_width;
        geometry[CE_RENDERWINDOW_STATE_FULLSCREEN].height = option_manager_t::instance()->fullscreen_height;

        visual.bpp = option_manager_t::instance()->fullscreen_bpp;
        visual.rate = option_manager_t::instance()->fullscreen_rate;

        visual.rotation = ce_display_rotation_from_degrees(option_manager_t::instance()->fullscreen_rotation);
        visual.reflection = ce_display_reflection_from_bool(option_manager_t::instance()->fullscreen_reflection_x, option_manager_t::instance()->fullscreen_reflection_y);
    }

    render_window_t::~render_window_t()
    {
        ce_vector_del(listeners);
    }

    void render_window_t::show()
    {
        do_show();
    }

    void render_window_t::minimize()
    {
        if (CE_RENDERWINDOW_STATE_FULLSCREEN == state) {
            // exit fullscreen before minimizing
            toggle_fullscreen();
        }
        do_minimize();
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

        do_toggle_fullscreen();
    }

    void render_window_t::pump()
    {
        // reset pointer offset every frame
        m_input_context->pointer_offset = CE_VEC2_ZERO;

        // reset wheel buttons: there are no "WheelRelease" events in most cases
        m_input_context->buttons[static_cast<size_t>(input_button_t::mb_wheelup)] = false;
        m_input_context->buttons[static_cast<size_t>(input_button_t::mb_wheeldown)] = false;

        do_pump();
    }

    void render_window_t::swap()
    {
        ce_graphics_context_swap(graphics_context);
    }

    void render_window_t::emit_resized(size_t width, size_t height)
    {
        for (size_t i = 0; i < listeners->count; ++i) {
            ce_renderwindow_listener* listener = (ce_renderwindow_listener*)listeners->items[i];
            if (NULL != listener->resized) {
                (*listener->resized)(listener->listener, width, height);
            }
        }
    }

    void render_window_t::emit_closed()
    {
        for (size_t i = 0; i < listeners->count; ++i) {
            ce_renderwindow_listener* listener = (ce_renderwindow_listener*)listeners->items[i];
            if (NULL != listener->closed) {
                (*listener->closed)(listener->listener);
            }
        }
    }

    void render_window_t::add_listener(ce_renderwindow_listener* listener)
    {
        ce_vector_push_back(listeners, listener);
    }
}
