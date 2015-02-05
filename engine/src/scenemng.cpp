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

#include "math.hpp"
#include "logging.hpp"
#include "alloc.hpp"
#include "frustum.hpp"
#include "byteformat.hpp"
#include "event.hpp"
#include "optionmanager.hpp"
#include "configmanager.hpp"
#include "rendersystem.hpp"
#include "figuremanager.hpp"
#include "mprmanager.hpp"
#include "mprhelper.hpp"
#include "mobloader.hpp"
#include "root.hpp"
#include "scenemng.hpp"

namespace cursedearth
{
    void ce_scenemng_renderwindow_resized(void* listener, int width, int height)
    {
        ce_scenemng* scenemng = (ce_scenemng*)listener;
        ce_viewport_set_rect(scenemng->viewport, 0, 0, width, height);
        ce_camera_set_aspect(scenemng->camera, (float)width / height);
    }

    void ce_scenemng_figproto_created(void* listener, ce_figproto* figproto)
    {
        ce_scenemng* scenemng = (ce_scenemng*)listener;
        ce_figproto_accept_renderqueue(figproto, scenemng->renderqueue);
    }

    ce_scenemng* ce_scenemng_new(void)
    {
        ce_scenemng* scenemng = (ce_scenemng*)ce_alloc_zero(sizeof(ce_scenemng));
        scenemng->thread_id = ce_thread_self();

        scenemng->camera_move_sensitivity = 10.0f;
        scenemng->camera_zoom_sensitivity = 5.0f;

        scenemng->scenenode = ce_scenenode_new(NULL);
        scenemng->renderqueue = ce_renderqueue_new();
        scenemng->viewport = ce_viewport_new();
        scenemng->camera = ce_camera_new();
        scenemng->fps = ce_fps_new();
        scenemng->font = ce_font_new("fonts/evilislands.ttf", 24);

        scenemng->input_supply = ce_input_supply_new(ce_root.renderwindow->input_context);
        scenemng->skip_logo_event = ce_input_supply_single_front(scenemng->input_supply,
                                        ce_input_supply_shortcut(scenemng->input_supply, "Space"));
        scenemng->pause_event = ce_input_supply_single_front(scenemng->input_supply,
                                        ce_input_supply_shortcut(scenemng->input_supply, "Space"));
        scenemng->move_left_event = ce_input_supply_shortcut(scenemng->input_supply, "ArrowLeft");
        scenemng->move_up_event = ce_input_supply_shortcut(scenemng->input_supply, "ArrowUp");
        scenemng->move_right_event = ce_input_supply_shortcut(scenemng->input_supply, "ArrowRight");
        scenemng->move_down_event = ce_input_supply_shortcut(scenemng->input_supply, "ArrowDown");
        scenemng->zoom_in_event = ce_input_supply_shortcut(scenemng->input_supply, "WheelUp");
        scenemng->zoom_out_event = ce_input_supply_shortcut(scenemng->input_supply, "WheelDown");
        scenemng->rotate_on_event = ce_input_supply_shortcut(scenemng->input_supply, "MouseRight");

        scenemng->renderwindow_listener = {ce_scenemng_renderwindow_resized, NULL, scenemng};
        scenemng->figure_manager_listener = {ce_scenemng_figproto_created, NULL, scenemng};

        ce_renderwindow_add_listener(ce_root.renderwindow, &scenemng->renderwindow_listener);
        ce_figure_manager_add_listener(&scenemng->figure_manager_listener);

        return scenemng;
    }

    void ce_scenemng_del(ce_scenemng* scenemng)
    {
        if (NULL != scenemng) {
            // FIXME: figure entities must be removed before terrain
            ce_figure_manager_clear();

            ce_input_supply_del(scenemng->input_supply);
            ce_terrain_del(scenemng->terrain);
            ce_font_del(scenemng->font);
            ce_fps_del(scenemng->fps);
            ce_camera_del(scenemng->camera);
            ce_viewport_del(scenemng->viewport);
            ce_renderqueue_del(scenemng->renderqueue);
            ce_scenenode_del(scenemng->scenenode);
            ce_free(scenemng, sizeof(ce_scenemng));
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
        ce_video_object_advance(scenemng->logo.video_object, elapsed);

        if (scenemng->skip_logo_event->triggered) {
            ce_video_object_stop(scenemng->logo.video_object);
        }

        if (ce_video_object_is_stopped(scenemng->logo.video_object)) {
            if (scenemng->logo.movie_index == ce_config_manager->movies[CE_CONFIG_MOVIE_START]->count) {
                ce_logging_debug("scene manager: switch to `ready'");
                ce_scenemng_change_state(scenemng, CE_SCENEMNG_STATE_READY);
            } else {
                ce_string* movie_name = (ce_string*)ce_config_manager->movies[CE_CONFIG_MOVIE_START]->items[scenemng->logo.movie_index++];
                scenemng->logo.video_object = ce_video_object_new(movie_name->str);
                ce_video_object_play(scenemng->logo.video_object);
            }
        }
    }

    void ce_scenemng_render_logo(ce_scenemng* scenemng)
    {
        ce_video_object_render(scenemng->logo.video_object);
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
                scenemng->loading.video_object = ce_video_object_new("progres");
            } else {
                char name[16];
                snprintf(name, sizeof(name), "progres%d", index);
                scenemng->loading.video_object = ce_video_object_new(name);
            }
            ce_video_object_play(scenemng->loading.video_object);
            scenemng->loading.created = true;
        }

        size_t completed_job_count = ce_mob_loader->completed_job_count;
        size_t queued_job_count = ce_mob_loader->queued_job_count;

        if (NULL != scenemng->terrain) {
            completed_job_count += scenemng->terrain->completed_job_count;
            queued_job_count += scenemng->terrain->queued_job_count;
        }

        ce_video_object_progress(scenemng->loading.video_object,
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
        ce_video_object_render(scenemng->loading.video_object);
    }

    void ce_scenemng_advance_playing(ce_scenemng* scenemng, float elapsed)
    {
        if (scenemng->move_left_event->triggered) {
            ce_camera_move(scenemng->camera, -scenemng->camera_move_sensitivity * elapsed, 0.0f);
        }

        if (scenemng->move_up_event->triggered) {
            ce_camera_move(scenemng->camera, 0.0f, scenemng->camera_move_sensitivity * elapsed);
        }

        if (scenemng->move_right_event->triggered) {
            ce_camera_move(scenemng->camera, scenemng->camera_move_sensitivity * elapsed, 0.0f);
        }

        if (scenemng->move_down_event->triggered) {
            ce_camera_move(scenemng->camera, 0.0f, -scenemng->camera_move_sensitivity * elapsed);
        }

        if (scenemng->zoom_in_event->triggered) {
            ce_camera_zoom(scenemng->camera, scenemng->camera_zoom_sensitivity);
        }

        if (scenemng->zoom_out_event->triggered) {
            ce_camera_zoom(scenemng->camera, -scenemng->camera_zoom_sensitivity);
        }

        if (scenemng->rotate_on_event->triggered) {
            float xcoef = 0.25f * (ce_option_manager->inverse_trackball_x ? 1.0f : -1.0f);
            float ycoef = 0.25f * (ce_option_manager->inverse_trackball_y ? 1.0f : -1.0f);
            ce_camera_yaw_pitch(scenemng->camera,
                ce_deg2rad(xcoef * scenemng->input_supply->input_context->pointer_offset.x),
                ce_deg2rad(ycoef * scenemng->input_supply->input_context->pointer_offset.y));
        }
    }

    void ce_scenemng_render_playing(ce_scenemng* scenemng)
    {
        if (ce_option_manager->show_axes) {
            ce_render_system_draw_axes();
        }

        ce_renderqueue_render(scenemng->renderqueue);
        ce_renderqueue_clear(scenemng->renderqueue);

        ce_vec3 forward, right, up;
        ce_frustum frustum;

        ce_frustum_init(&frustum, scenemng->camera->fov,
            scenemng->camera->aspect, scenemng->camera->near,
            scenemng->camera->far, &scenemng->camera->position,
            ce_camera_get_forward(scenemng->camera, &forward),
            ce_camera_get_right(scenemng->camera, &right),
            ce_camera_get_up(scenemng->camera, &up));

        ce_scenenode_update_cascade(scenemng->scenenode, &frustum);

        if (ce_root.show_bboxes) {
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
        ce_fps_advance(scenemng->fps, elapsed);
        ce_input_supply_advance(scenemng->input_supply, elapsed);

        (*ce_scenemng_state_procs[scenemng->state].advance)(scenemng, elapsed);

        if (NULL != scenemng->listener.advance) {
            (*scenemng->listener.advance)(scenemng->listener.receiver, elapsed);
        }
    }

    void ce_scenemng_render(ce_scenemng* scenemng)
    {
        ce_render_system_begin_render(&CE_COLOR_WHITE);

        ce_render_system_setup_viewport(scenemng->viewport);
        ce_render_system_setup_camera(scenemng->camera);

        (*ce_scenemng_state_procs[scenemng->state].render)(scenemng);

        if (NULL != scenemng->listener.render) {
            (*scenemng->listener.render)(scenemng->listener.receiver);
        }

        if (ce_option_manager->show_fps) {
            ce_font_render(scenemng->font, scenemng->viewport->width -
                ce_font_get_width(scenemng->font, scenemng->fps->text) - 10,
                scenemng->viewport->height - ce_font_get_height(scenemng->font) - 10,
                &CE_COLOR_GOLD, scenemng->fps->text);
        }

        const char* engine_text = "Powered by Cursed Earth engine";
        ce_font_render(scenemng->font, scenemng->viewport->width - ce_font_get_width(scenemng->font, engine_text) - 10, 10, &CE_COLOR_RED, engine_text);

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
