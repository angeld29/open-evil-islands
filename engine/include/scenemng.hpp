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

#ifndef CE_SCENEMNG_HPP
#define CE_SCENEMNG_HPP

#include "timer.hpp"
#include "thread.hpp"
#include "input.hpp"
#include "terrain.hpp"
#include "fps.hpp"
#include "font.hpp"
#include "viewport.hpp"
#include "camera.hpp"
#include "scenenode.hpp"
#include "renderqueue.hpp"
#include "renderwindow.hpp"
#include "figuremanager.hpp"
#include "videoobject.hpp"

namespace cursedearth
{
    enum {
        CE_SCENEMNG_STATE_LOGO,
        CE_SCENEMNG_STATE_READY,
        CE_SCENEMNG_STATE_LOADING,
        CE_SCENEMNG_STATE_PLAYING,
        CE_SCENEMNG_STATE_COUNT
    };

    typedef struct {
        void (*state_changed)(void* listener, int state);
        void (*advance)(void* listener, float elapsed);
        void (*render)(void* listener);
        void* receiver;
    } ce_scenemng_listener;

    typedef struct {
        ce_thread_id thread_id;
        int state;
        float camera_move_sensitivity; // FIXME: hard-coded
        float camera_zoom_sensitivity; // TODO: make strategy
        ce_scenenode* scenenode;
        ce_renderqueue* renderqueue;
        ce_viewport* viewport;
        ce_camera* camera;
        ce_fps* fps;
        ce_font* font;
        ce_terrain* terrain;
        ce_input_supply* input_supply;
        ce_input_event* skip_logo_event;
        ce_input_event* pause_event;
        ce_input_event* move_left_event;
        ce_input_event* move_up_event;
        ce_input_event* move_right_event;
        ce_input_event* move_down_event;
        ce_input_event* zoom_in_event;
        ce_input_event* zoom_out_event;
        ce_input_event* rotate_on_event;
        ce_scenemng_listener listener;
        ce_renderwindow_listener renderwindow_listener;
        ce_figure_manager_listener figure_manager_listener;
        // TODO: split by states
        struct {
            size_t movie_index;
            ce_video_object video_object;
        } logo;
        struct {
            bool created;
            ce_video_object video_object;
        } loading;
    } ce_scenemng;

    extern ce_scenemng* ce_scenemng_new(void);
    extern void ce_scenemng_del(ce_scenemng* scenemng);

    extern void ce_scenemng_change_state(ce_scenemng* scenemng, int state);

    extern void ce_scenemng_advance(ce_scenemng* scenemng, float elapsed);
    extern void ce_scenemng_render(ce_scenemng* scenemng);

    extern void ce_scenemng_load_mpr(ce_scenemng* scenemng, const char* name);
}

#endif /* CE_SCENEMNG_HPP */
