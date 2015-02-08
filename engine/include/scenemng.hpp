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

    class ce_scenemng
    {
    public:
        ce_thread_id thread_id;
        int state = CE_SCENEMNG_STATE_LOGO;
        float camera_move_sensitivity = 10.0f; // FIXME: hard-coded
        float camera_zoom_sensitivity = 5.0f; // TODO: make strategy
        ce_scenenode* scenenode;
        ce_renderqueue* renderqueue;
        ce_viewport* viewport;
        ce_camera* camera;
        fps_t* fps;
        ce_font* font;
        ce_terrain* terrain;
        input_supply_ptr_t input_supply;
        input_event_const_ptr_t skip_logo_event;
        input_event_const_ptr_t pause_event;
        input_event_const_ptr_t move_left_event;
        input_event_const_ptr_t move_up_event;
        input_event_const_ptr_t move_right_event;
        input_event_const_ptr_t move_down_event;
        input_event_const_ptr_t zoom_in_event;
        input_event_const_ptr_t zoom_out_event;
        input_event_const_ptr_t rotate_on_event;
        ce_scenemng_listener listener;
        ce_renderwindow_listener renderwindow_listener;
        ce_figure_manager_listener figure_manager_listener;
        // TODO: split by states
        struct {
            size_t movie_index;
            video_object_t video_object;
        } logo;
        struct {
            bool created;
            video_object_t video_object;
        } loading;
    };

    ce_scenemng* ce_scenemng_new();
    void ce_scenemng_del(ce_scenemng* scenemng);

    void ce_scenemng_change_state(ce_scenemng* scenemng, int state);

    void ce_scenemng_advance(ce_scenemng* scenemng, float elapsed);
    void ce_scenemng_render(ce_scenemng* scenemng);

    void ce_scenemng_load_mpr(ce_scenemng* scenemng, const char* name);
}

#endif
