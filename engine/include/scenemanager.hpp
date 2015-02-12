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

#ifndef CE_SCENEMANAGER_HPP
#define CE_SCENEMANAGER_HPP

#include <memory>

#include <boost/noncopyable.hpp>

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
    class scene_manager_t: boost::noncopyable
    {
    public:
        scene_manager_t();
        virtual ~scene_manager_t();

        void advance(float elapsed);
        void render();

    protected:
        void load_mpr(const std::string& name);
        void load_mob(const std::string& name);

        void add_node(ce_scenenode*);

        const viewport_t& get_viewport() const { return m_viewport; }
        ce_font* get_font() { return m_font; }

    private:
        virtual void do_advance(float elapsed) = 0;
        virtual void do_render() = 0;

    public:
        viewport_t m_viewport;
        ce_camera* m_camera;
        ce_renderqueue* m_renderqueue;

    private:
        const ce_thread_id m_thread_id;
        bool m_show_bboxes = false;
        bool m_comprehensive_bbox_only = true;
        float m_camera_move_sensitivity = 10.0f;
        float m_camera_zoom_sensitivity = 5.0f;
        const std::string m_engine_text = "Powered by Cursed Earth engine";
        fps_ptr_t m_fps;
        ce_font* m_font;
        ce_scenenode* m_scenenode;
        ce_terrain* m_terrain;
        input_supply_ptr_t m_input_supply;
        input_event_const_ptr_t m_toggle_bbox_event;
        input_event_const_ptr_t m_skip_logo_event;
        input_event_const_ptr_t m_pause_event;
        input_event_const_ptr_t m_move_left_event;
        input_event_const_ptr_t m_move_up_event;
        input_event_const_ptr_t m_move_right_event;
        input_event_const_ptr_t m_move_down_event;
        input_event_const_ptr_t m_zoom_in_event;
        input_event_const_ptr_t m_zoom_out_event;
        input_event_const_ptr_t m_rotate_on_event;
        ce_renderwindow_listener m_renderwindow_listener;
        ce_figure_manager_listener m_figure_manager_listener;
    };

    typedef std::shared_ptr<scene_manager_t> scene_manager_ptr_t;
}

#endif
