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

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "utility.hpp"
#include "logging.hpp"
#include "frustum.hpp"
#include "event.hpp"
#include "optionmanager.hpp"
#include "configmanager.hpp"
#include "rendersystem.hpp"
#include "figuremanager.hpp"
#include "mprmanager.hpp"
#include "mprhelpers.hpp"
#include "mobloader.hpp"
#include "scenemanager.hpp"

namespace cursedearth
{
    void ce_scenemng_figproto_created(void* listener, ce_figproto* figproto)
    {
        scene_manager_t* scenemng = (scene_manager_t*)listener;
        ce_figproto_accept_renderqueue(figproto, scenemng->m_renderqueue);
    }

    scene_manager_t::scene_manager_t(const input_context_const_ptr_t& input_context):
        singleton_t<scene_manager_t>(this),
        m_camera(ce_camera_new()),
        m_renderqueue(ce_renderqueue_new()),
        m_thread_id(ce_thread_self()),
        m_fps(make_fps()),
        m_font(ce_font_new("fonts/evilislands.ttf", 24)),
        m_scenenode(ce_scenenode_new(NULL)),
        m_terrain(NULL),
        m_input_supply(std::make_shared<input_supply_t>(input_context)),
        m_toggle_bbox_event(m_input_supply->single_front(m_input_supply->push(input_button_t::kb_b))),
        m_skip_logo_event(m_input_supply->single_front(m_input_supply->push(input_button_t::kb_space))),
        m_pause_event(m_input_supply->single_front(m_input_supply->push(input_button_t::kb_space))),
        m_move_left_event(m_input_supply->push(input_button_t::kb_left)),
        m_move_up_event(m_input_supply->push(input_button_t::kb_up)),
        m_move_right_event(m_input_supply->push(input_button_t::kb_right)),
        m_move_down_event(m_input_supply->push(input_button_t::kb_down)),
        m_zoom_in_event(m_input_supply->push(input_button_t::mb_wheelup)),
        m_zoom_out_event(m_input_supply->push(input_button_t::mb_wheeldown)),
        m_rotate_on_event(m_input_supply->push(input_button_t::mb_right))
    {
        m_figure_manager_listener = {ce_scenemng_figproto_created, NULL, this};
        ce_figure_manager_add_listener(&m_figure_manager_listener);
    }

    scene_manager_t::~scene_manager_t()
    {
        // FIXME: figure entities must be removed before terrain
        ce_figure_manager_clear();
        ce_terrain_del(m_terrain);
        ce_font_del(m_font);
        ce_camera_del(m_camera);
        ce_renderqueue_del(m_renderqueue);
        ce_scenenode_del(m_scenenode);
    }

    void scene_manager_t::resize(size_t width, size_t height)
    {
        m_viewport.set_dimensions(width, height);
        ce_camera_set_aspect(m_camera, static_cast<float>(width) / height);
    }

    void scene_manager_t::advance(float elapsed)
    {
        m_fps->advance(elapsed);
        m_input_supply->advance(elapsed);

        if (m_toggle_bbox_event->triggered()) {
            if (m_show_bboxes) {
                if (m_comprehensive_bbox_only) {
                    m_comprehensive_bbox_only = false;
                } else {
                    m_show_bboxes = false;
                }
            } else {
                m_show_bboxes = true;
                m_comprehensive_bbox_only = true;
            }
        }

        if (m_move_left_event->triggered()) {
            ce_camera_move(m_camera, -m_camera_move_sensitivity * elapsed, 0.0f);
        }

        if (m_move_up_event->triggered()) {
            ce_camera_move(m_camera, 0.0f, m_camera_move_sensitivity * elapsed);
        }

        if (m_move_right_event->triggered()) {
            ce_camera_move(m_camera, m_camera_move_sensitivity * elapsed, 0.0f);
        }

        if (m_move_down_event->triggered()) {
            ce_camera_move(m_camera, 0.0f, -m_camera_move_sensitivity * elapsed);
        }

        if (m_zoom_in_event->triggered()) {
            ce_camera_zoom(m_camera, m_camera_zoom_sensitivity);
        }

        if (m_zoom_out_event->triggered()) {
            ce_camera_zoom(m_camera, -m_camera_zoom_sensitivity);
        }

        if (m_rotate_on_event->triggered()) {
            float xcoef = 0.25f * (option_manager_t::instance()->inverse_trackball_x ? 1.0f : -1.0f);
            float ycoef = 0.25f * (option_manager_t::instance()->inverse_trackball_y ? 1.0f : -1.0f);
            ce_camera_yaw_pitch(m_camera,
                deg2rad(xcoef * m_input_supply->pointer_offset().x),
                deg2rad(ycoef * m_input_supply->pointer_offset().y));
        }


        // Aleks // TODO move to aidesigner
        size_t completed_job_count = ce_mob_loader->completed_job_count;
        size_t queued_job_count = ce_mob_loader->queued_job_count;

        if (NULL != m_terrain && once && completed_job_count >= queued_job_count) {
            once = false;
            ce_logging_debug("test entities count: %d", ce_figure_manager->entities->count);
            for (size_t i = 0; i < ce_figure_manager->entities->count; ++i) {
                ce_figentity* figentity = (ce_figentity*) ce_figure_manager->entities->items[i];
                ce_figentity_fix_height(figentity,
                    ce_mpr_get_height(m_terrain->mprfile, &figentity->position));
                ce_scenenode_attach_child(ce_terrain_find_scenenode(m_terrain,
                    figentity->position.x, figentity->position.z), figentity->scenenode);
            }
        }
        // /Aleks

        do_advance(elapsed);
    }

    void scene_manager_t::render()
    {
        ce_render_system_begin_render(&CE_COLOR_WHITE);

        ce_render_system_setup_viewport(&m_viewport);
        ce_render_system_setup_camera(m_camera);

        if (option_manager_t::instance()->show_axes()) {
            ce_render_system_draw_axes();
        }

        ce_renderqueue_render(m_renderqueue);
        ce_renderqueue_clear(m_renderqueue);

        vector3_t forward, right, up;
        frustum_t frustum;

        ce_frustum_init(&frustum, m_camera->fov,
            m_camera->aspect, m_camera->m_near,
            m_camera->m_far, &m_camera->position,
            ce_camera_get_forward(m_camera, &forward),
            ce_camera_get_right(m_camera, &right),
            ce_camera_get_up(m_camera, &up));

        ce_scenenode_update_cascade(m_scenenode, &frustum);

        if (m_show_bboxes) {
            ce_render_system_apply_color(&CE_COLOR_BLUE);
            ce_scenenode_draw_bboxes_cascade(m_scenenode, m_comprehensive_bbox_only);
        }

        do_render();

#ifndef NDEBUG
        ce_font_render(m_font, 10, 10, CE_COLOR_SILVER, std::to_string(ce_scenenode_count_visible_cascade(m_scenenode)));
#endif

        if (option_manager_t::instance()->show_fps()) {
            ce_font_render(m_font, m_viewport.width - ce_font_get_width(m_font, m_fps->text()) - 10,
                m_viewport.height - ce_font_get_height(m_font) - 10, CE_COLOR_GOLD, m_fps->text());
        }

        ce_font_render(m_font, m_viewport.width - ce_font_get_width(m_font, m_engine_text) - 10, 10, CE_COLOR_RED, m_engine_text);

        ce_render_system_end_render();
    }

    void scene_manager_t::load_mpr(const std::string& name)
    {
        // TODO: mpr loader?
        ce_mprfile* mprfile = ce_mpr_manager_open(name);
        if (NULL != mprfile) {
            ce_terrain_del(m_terrain);

            m_terrain = ce_terrain_new(mprfile, m_renderqueue,
                &CE_VEC3_ZERO, &CE_QUAT_IDENTITY, m_scenenode);
        }
    }

    void scene_manager_t::load_mob(const std::string& name)
    {
        ce_mob_loader_load_mob(name);
    }

    void scene_manager_t::add_node(ce_scenenode* node)
    {
        ce_scenenode_attach_child(m_scenenode, node);
    }

    /*void ce_scenemng_advance_logo(scene_manager_t* scenemng, float elapsed)
    {
        video_object_advance(scenemng->logo.video_object, elapsed);

        if (scenemng->skip_logo_event->triggered()) {
            stop_video_object(scenemng->logo.video_object);
        }

        if (video_object_is_stopped(scenemng->logo.video_object)) {
            if (scenemng->logo.movie_index == ce_config_manager->movies[CE_CONFIG_MOVIE_START]->count) {
                ce_logging_debug("scene manager: switch to `ready'");
                ce_scenemng_change_state(scenemng, CE_SCENEMNG_STATE_READY);
            } else {
                ce_string* movie_name = (ce_string*)ce_config_manager->movies[CE_CONFIG_MOVIE_START]->items[scenemng->logo.movie_index++];
                scenemng->logo.video_object = make_video_object(movie_name->str);
                play_video_object(scenemng->logo.video_object);
            }
        }
    }

    void ce_scenemng_advance_loading(scene_manager_t* scenemng, float elapsed)
    {
        // TODO: constructor
        if (!scenemng->loading.created) {
            srand(time(NULL));
            int index = rand() % 5;
            if (0 == index) {
                scenemng->loading.video_object = make_video_object("progres");
            } else {
                char name[16];
                snprintf(name, sizeof(name), "progres%d", index);
                scenemng->loading.video_object = make_video_object(name);
            }
            play_video_object(scenemng->loading.video_object);
            scenemng->loading.created = true;
        }

        size_t completed_job_count = ce_mob_loader->completed_job_count;
        size_t queued_job_count = ce_mob_loader->queued_job_count;

        if (NULL != scenemng->terrain) {
            completed_job_count += scenemng->terrain->completed_job_count;
            queued_job_count += scenemng->terrain->queued_job_count;
        }

        video_object_progress(scenemng->loading.video_object,
            100.0f * completed_job_count / queued_job_count);

        // TODO: hold last frame
        if (completed_job_count >= queued_job_count) {
            if (NULL != scenemng->terrain) {
                for (size_t i = 0; i < ce_figure_manager->entities->count; ++i) {
                    ce_figentity* figentity = (ce_figentity*)ce_figure_manager->entities->items[i];
                    ce_figentity_fix_height(figentity, ce_mpr_get_height(scenemng->terrain->mprfile, &figentity->position));
                    ce_scenenode_attach_child(ce_terrain_find_scenenode(scenemng->terrain, figentity->position.x, figentity->position.z), figentity->scenenode);
                }
            }

            ce_scenenode_update_force_cascade(scenemng->scenenode);

            ce_logging_debug("scene manager: switch to `playing'");
            ce_scenemng_change_state(scenemng, CE_SCENEMNG_STATE_PLAYING);
        } else {
            // do not eat CPU time, mostly for single-core CPU
            if (!ce_event_manager_has_pending_events(ce_thread_self())) {
                thread_pool_t::instance()->wait_one();

            }
        }
    }*/
}
