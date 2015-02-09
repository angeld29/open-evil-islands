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
#include "root.hpp"
#include "scenemng.hpp"

namespace cursedearth
{
    void ce_scenemng_renderwindow_resized(void* listener, int width, int height)
    {
        ce_scenemng* scenemng = (ce_scenemng*)listener;
        scenemng->viewport.set_dimensions(width, height);
        ce_camera_set_aspect(scenemng->camera, static_cast<float>(width) / height);
    }

    void ce_scenemng_figproto_created(void* listener, ce_figproto* figproto)
    {
        ce_scenemng* scenemng = (ce_scenemng*)listener;
        ce_figproto_accept_renderqueue(figproto, scenemng->renderqueue);
    }

    ce_scenemng* ce_scenemng_new(void)
    {
        ce_scenemng* scenemng = new ce_scenemng;

        scenemng->thread_id = ce_thread_self();
        scenemng->scenenode = ce_scenenode_new(NULL);
        scenemng->renderqueue = ce_renderqueue_new();
        scenemng->camera = ce_camera_new();
        scenemng->fps = std::make_shared<fps_t>();
        scenemng->font = ce_font_new("fonts/evilislands.ttf", 24);
        scenemng->terrain = NULL;

        scenemng->input_supply = std::make_shared<input_supply_t>(ce_root::instance()->renderwindow->input_context());
        scenemng->skip_logo_event = scenemng->input_supply->single_front(scenemng->input_supply->push(input_button_t::kb_space));
        scenemng->pause_event = scenemng->input_supply->single_front(scenemng->input_supply->push(input_button_t::kb_space));
        scenemng->move_left_event = scenemng->input_supply->push(input_button_t::kb_left);
        scenemng->move_up_event = scenemng->input_supply->push(input_button_t::kb_up);
        scenemng->move_right_event = scenemng->input_supply->push(input_button_t::kb_right);
        scenemng->move_down_event = scenemng->input_supply->push(input_button_t::kb_down);
        scenemng->zoom_in_event = scenemng->input_supply->push(input_button_t::mb_wheelup);
        scenemng->zoom_out_event = scenemng->input_supply->push(input_button_t::mb_wheeldown);
        scenemng->rotate_on_event = scenemng->input_supply->push(input_button_t::mb_right);

        scenemng->renderwindow_listener = {ce_scenemng_renderwindow_resized, NULL, scenemng};
        scenemng->figure_manager_listener = {ce_scenemng_figproto_created, NULL, scenemng};

        ce_renderwindow_add_listener(ce_root::instance()->renderwindow, &scenemng->renderwindow_listener);
        ce_figure_manager_add_listener(&scenemng->figure_manager_listener);

        memset(&scenemng->logo, 0, sizeof(scenemng->logo));
        memset(&scenemng->loading, 0, sizeof(scenemng->loading));

        return scenemng;
    }

    void ce_scenemng_del(ce_scenemng* scenemng)
    {
        if (NULL != scenemng) {
            // FIXME: figure entities must be removed before terrain
            ce_figure_manager_clear();
            ce_terrain_del(scenemng->terrain);
            ce_font_del(scenemng->font);
            ce_camera_del(scenemng->camera);
            ce_renderqueue_del(scenemng->renderqueue);
            ce_scenenode_del(scenemng->scenenode);
            delete scenemng;
        }
    }

    void ce_scenemng_change_state(ce_scenemng* scenemng, int state)
    {
        scenemng->state = state;
        if (NULL != scenemng->listener.state_changed) {
            (*scenemng->listener.state_changed)(scenemng->listener.receiver, state);
        }
    }

    void ce_scenemng_advance_logo(ce_scenemng* scenemng, float elapsed)
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

    void ce_scenemng_render_logo(ce_scenemng* scenemng)
    {
        video_object_render(scenemng->logo.video_object);
    }

    void ce_scenemng_advance_ready(ce_scenemng*, float /*elapsed*/)
    {
    }

    void ce_scenemng_render_ready(ce_scenemng*)
    {
    }

    void ce_scenemng_advance_loading(ce_scenemng* scenemng, float /*elapsed*/)
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
                ce_thread_pool_wait_one();
            }
        }
    }

    void ce_scenemng_render_loading(ce_scenemng* scenemng)
    {
        video_object_render(scenemng->loading.video_object);
    }

    void ce_scenemng_advance_playing(ce_scenemng* scenemng, float elapsed)
    {
        if (scenemng->move_left_event->triggered()) {
            ce_camera_move(scenemng->camera, -scenemng->camera_move_sensitivity * elapsed, 0.0f);
        }

        if (scenemng->move_up_event->triggered()) {
            ce_camera_move(scenemng->camera, 0.0f, scenemng->camera_move_sensitivity * elapsed);
        }

        if (scenemng->move_right_event->triggered()) {
            ce_camera_move(scenemng->camera, scenemng->camera_move_sensitivity * elapsed, 0.0f);
        }

        if (scenemng->move_down_event->triggered()) {
            ce_camera_move(scenemng->camera, 0.0f, -scenemng->camera_move_sensitivity * elapsed);
        }

        if (scenemng->zoom_in_event->triggered()) {
            ce_camera_zoom(scenemng->camera, scenemng->camera_zoom_sensitivity);
        }

        if (scenemng->zoom_out_event->triggered()) {
            ce_camera_zoom(scenemng->camera, -scenemng->camera_zoom_sensitivity);
        }

        if (scenemng->rotate_on_event->triggered()) {
            float xcoef = 0.25f * (option_manager_t::instance()->inverse_trackball_x ? 1.0f : -1.0f);
            float ycoef = 0.25f * (option_manager_t::instance()->inverse_trackball_y ? 1.0f : -1.0f);
            ce_camera_yaw_pitch(scenemng->camera,
                deg2rad(xcoef * scenemng->input_supply->pointer_offset().x),
                deg2rad(ycoef * scenemng->input_supply->pointer_offset().y));
        }
    }

    void ce_scenemng_render_playing(ce_scenemng* scenemng)
    {
        if (option_manager_t::instance()->show_axes) {
            ce_render_system_draw_axes();
        }

        ce_renderqueue_render(scenemng->renderqueue);
        ce_renderqueue_clear(scenemng->renderqueue);

        vector3_t forward, right, up;
        frustum_t frustum;

        ce_frustum_init(&frustum, scenemng->camera->fov,
            scenemng->camera->aspect, scenemng->camera->near,
            scenemng->camera->far, &scenemng->camera->position,
            ce_camera_get_forward(scenemng->camera, &forward),
            ce_camera_get_right(scenemng->camera, &right),
            ce_camera_get_up(scenemng->camera, &up));

        ce_scenenode_update_cascade(scenemng->scenenode, &frustum);

        if (ce_root::instance()->show_bboxes) {
            ce_render_system_apply_color(&CE_COLOR_BLUE);
            ce_scenenode_draw_bboxes_cascade(scenemng->scenenode);
        }

    #ifndef NDEBUG
        char scenenode_text[8];
        snprintf(scenenode_text, sizeof(scenenode_text), "%d",
            ce_scenenode_count_visible_cascade(scenemng->scenenode));
        ce_font_render(scenemng->font, 10, 10, &CE_COLOR_SILVER, scenenode_text);
    #endif
    }

    struct {
        void (*advance)(ce_scenemng* scenemng, float elapsed);
        void (*render)(ce_scenemng* scenemng);
    } ce_scenemng_state_procs[CE_SCENEMNG_STATE_COUNT] = {
        { ce_scenemng_advance_logo, ce_scenemng_render_logo },
        { ce_scenemng_advance_ready, ce_scenemng_render_ready },
        { ce_scenemng_advance_loading, ce_scenemng_render_loading },
        { ce_scenemng_advance_playing, ce_scenemng_render_playing }
    };

    void ce_scenemng_advance(ce_scenemng* scenemng, float elapsed)
    {
        scenemng->fps->advance(elapsed);
        scenemng->input_supply->advance(elapsed);

        (*ce_scenemng_state_procs[scenemng->state].advance)(scenemng, elapsed);

        if (NULL != scenemng->listener.advance) {
            (*scenemng->listener.advance)(scenemng->listener.receiver, elapsed);
        }
    }

    void ce_scenemng_render(ce_scenemng* scenemng)
    {
        ce_render_system_begin_render(&CE_COLOR_WHITE);

        ce_render_system_setup_viewport(&scenemng->viewport);
        ce_render_system_setup_camera(scenemng->camera);

        (*ce_scenemng_state_procs[scenemng->state].render)(scenemng);

        if (NULL != scenemng->listener.render) {
            (*scenemng->listener.render)(scenemng->listener.receiver);
        }

        if (option_manager_t::instance()->show_fps) {
            ce_font_render(scenemng->font, scenemng->viewport.width -
                ce_font_get_width(scenemng->font, scenemng->fps->text()) - 10,
                scenemng->viewport.height - ce_font_get_height(scenemng->font) - 10,
                &CE_COLOR_GOLD, scenemng->fps->text());
        }

        const char* engine_text = "Powered by Cursed Earth engine";
        ce_font_render(scenemng->font, scenemng->viewport.width - ce_font_get_width(scenemng->font, engine_text) - 10, 10, &CE_COLOR_RED, engine_text);

        ce_render_system_end_render();
    }

    void ce_scenemng_load_mpr(ce_scenemng* scenemng, const char* name)
    {
        // TODO: mpr loader?
        ce_mprfile* mprfile = ce_mpr_manager_open(name);
        if (NULL != mprfile) {
            ce_terrain_del(scenemng->terrain);

            scenemng->terrain = ce_terrain_new(mprfile, scenemng->renderqueue,
                &CE_VEC3_ZERO, &CE_QUAT_IDENTITY, scenemng->scenenode);
        }
    }
}
