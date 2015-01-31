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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "lib.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "event.hpp"
#include "rendersystem.hpp"
#include "shadermanager.hpp"
#include "videoinstance.hpp"

static void ce_video_instance_exit(ce_event* event)
{
    ce_video_instance* video_instance = ((ce_event_ptr*)event->impl)->ptr;
    video_instance->done = true;
}

static void ce_video_instance_exec(ce_video_instance* video_instance)
{
    for (size_t i = ce_semaphore_available(video_instance->prepared_frames); ; ++i) {
        ce_semaphore_acquire(video_instance->unprepared_frames, 1);

        if (!ce_video_resource_read(video_instance->video_resource)) {
            video_instance->state = CE_VIDEO_INSTANCE_STATE_STOPPING;
            break;
        }

        ce_mmpfile* ycbcr_frame = video_instance->ycbcr_frames[i % CE_VIDEO_INSTANCE_CACHE_SIZE];
        ce_ycbcr* ycbcr = &video_instance->video_resource->ycbcr;

        unsigned char* y_data = ycbcr_frame->texels;
        unsigned char* cb_data = y_data + ycbcr_frame->width * ycbcr_frame->height;
        unsigned char* cr_data = cb_data + (ycbcr_frame->width / 2) * (ycbcr_frame->height / 2);

        int y_offset = (ycbcr->crop_rect.x & ~1) +
                        ycbcr->planes[0].stride * (ycbcr->crop_rect.y & ~1);
        int cb_offset = (ycbcr->crop_rect.x / 2) +
                        ycbcr->planes[1].stride * (ycbcr->crop_rect.y / 2);
        int cr_offset = (ycbcr->crop_rect.x / 2) +
                        ycbcr->planes[2].stride * (ycbcr->crop_rect.y / 2);

        for (unsigned int h = 0; h < ycbcr->crop_rect.height; ++h) {
            memcpy(y_data + h * ycbcr->crop_rect.width, ycbcr->planes[0].data +
                y_offset + h * ycbcr->planes[0].stride, ycbcr->crop_rect.width);
        }

        for (unsigned int h = 0; h < ycbcr->crop_rect.height / 2; ++h) {
            memcpy(cb_data + h * (ycbcr->crop_rect.width / 2), ycbcr->planes[1].data +
                cb_offset + h * ycbcr->planes[1].stride, ycbcr->crop_rect.width / 2);
            memcpy(cr_data + h * (ycbcr->crop_rect.width / 2), ycbcr->planes[2].data +
                cr_offset + h * ycbcr->planes[2].stride, ycbcr->crop_rect.width / 2);
        }

        ce_semaphore_release(video_instance->prepared_frames, 1);
        ce_event_manager_process_events(ce_thread_self(), CE_EVENT_FLAG_ALL_EVENTS);

        if (video_instance->done) {
            break;
        }
    }
}

ce_video_instance* ce_video_instance_new(ce_video_object video_object,
                                        ce_sound_object sound_object,
                                        ce_video_resource* video_resource)
{
    ce_video_instance* video_instance = ce_alloc_zero(sizeof(ce_video_instance));

    video_instance->video_object = video_object;
    video_instance->sound_object = sound_object;
    video_instance->frame = -1;
    video_instance->video_resource = video_resource;
    video_instance->texture = ce_texture_new("frame", NULL);
    video_instance->material = ce_material_new();
    video_instance->material->mode = CE_MATERIAL_MODE_REPLACE;
    video_instance->material->shader = ce_shader_manager_get((const char*[])
        {"shaders/ycbcr2rgba.vert", "shaders/ycbcr2rgba.frag", NULL});

    if (NULL != video_instance->material->shader) {
        ce_shader_add_ref(video_instance->material->shader);
    }

    video_instance->rgba_frame = ce_mmpfile_new(video_resource->width,
        video_resource->height, 1, CE_MMPFILE_FORMAT_R8G8B8A8, 0);

    for (size_t i = 0; i < CE_VIDEO_INSTANCE_CACHE_SIZE; ++i) {
        video_instance->ycbcr_frames[i] = ce_mmpfile_new(video_resource->width,
            video_resource->height, 1, CE_MMPFILE_FORMAT_YCBCR, 0);
    }

    video_instance->prepared_frames = ce_semaphore_new(0);
    video_instance->unprepared_frames = ce_semaphore_new(CE_VIDEO_INSTANCE_CACHE_SIZE);
    video_instance->thread = ce_thread_new(ce_video_instance_exec, video_instance);

    return video_instance;
}

void ce_video_instance_del(ce_video_instance* video_instance)
{
    if (NULL != video_instance) {
        ce_event_manager_post_ptr(video_instance->thread->id, ce_video_instance_exit, video_instance);

        ce_semaphore_release(video_instance->unprepared_frames, 1);
        ce_thread_wait(video_instance->thread);

        ce_thread_del(video_instance->thread);
        ce_semaphore_del(video_instance->unprepared_frames);
        ce_semaphore_del(video_instance->prepared_frames);

        for (size_t i = 0; i < CE_VIDEO_INSTANCE_CACHE_SIZE; ++i) {
            ce_mmpfile_del(video_instance->ycbcr_frames[i]);
        }

        ce_mmpfile_del(video_instance->rgba_frame);
        ce_material_del(video_instance->material);
        ce_texture_del(video_instance->texture);
        ce_video_resource_del(video_instance->video_resource);
        ce_sound_object_del(video_instance->sound_object);

        ce_free(video_instance, sizeof(ce_video_instance));
    }
}

static void ce_video_instance_do_advance(ce_video_instance* video_instance)
{
    bool acquired = false;
    int desired_frame = video_instance->video_resource->fps * video_instance->play_time;

    // if sound or time far away
    while (video_instance->frame < desired_frame &&
            ce_semaphore_try_acquire(video_instance->prepared_frames, 1)) {
        // skip frames to reach desired frame
        if (++video_instance->frame == desired_frame ||
                // or use the closest frame
                0 == ce_semaphore_available(video_instance->prepared_frames)) {
            ce_mmpfile* ycbcr_frame = video_instance->ycbcr_frames
                [video_instance->frame % CE_VIDEO_INSTANCE_CACHE_SIZE];

            if (NULL != video_instance->material->shader) {
                const uint8_t* y_data = ycbcr_frame->texels;
                const uint8_t* cb_data = y_data + ycbcr_frame->width * ycbcr_frame->height;
                const uint8_t* cr_data = cb_data + (ycbcr_frame->width / 2) * (ycbcr_frame->height / 2);

                uint8_t* texels = video_instance->rgba_frame->texels;

                for (unsigned int h = 0; h < ycbcr_frame->height; ++h) {
                    for (unsigned int w = 0; w < ycbcr_frame->width; ++w) {
                        size_t index = 4 * (h * ycbcr_frame->width + w);
                        texels[index + 0] = y_data[h * ycbcr_frame->width + w];
                        texels[index + 1] = cb_data[(h / 2) * (ycbcr_frame->width / 2) + w / 2];
                        texels[index + 2] = cr_data[(h / 2) * (ycbcr_frame->width / 2) + w / 2];
                        texels[index + 3] = 255;
                    }
                }
            } else {
                ce_mmpfile_convert2(ycbcr_frame, video_instance->rgba_frame);
            }

            ce_texture_replace(video_instance->texture, video_instance->rgba_frame);
            acquired = true;
        }
        ce_semaphore_release(video_instance->unprepared_frames, 1);
    }

    // TODO: think again how to hold last frame
    if (CE_VIDEO_INSTANCE_STATE_STOPPING == video_instance->state && !acquired &&
            0 == ce_semaphore_available(video_instance->prepared_frames)) {
        video_instance->state = CE_VIDEO_INSTANCE_STATE_STOPPED;
    }
}

void ce_video_instance_advance(ce_video_instance* video_instance, float elapsed)
{
    if (CE_VIDEO_INSTANCE_STATE_PAUSED == video_instance->state) {
        return;
    }

    if (ce_sound_object_is_valid(video_instance->sound_object)) {
        // synchronization with sound
        float sound_time = ce_sound_object_get_time(video_instance->sound_object);
        if (video_instance->sync_time != sound_time) {
            video_instance->sync_time = sound_time;
            video_instance->play_time = sound_time;
        } else {
            video_instance->play_time += elapsed;
        }
    } else {
        video_instance->play_time += elapsed;
    }

    ce_video_instance_do_advance(video_instance);
}

void ce_video_instance_progress(ce_video_instance* video_instance, int percents)
{
    video_instance->play_time = (video_instance->video_resource->frame_count /
        video_instance->video_resource->fps) * (0.01f * percents);

    ce_video_instance_do_advance(video_instance);
}

void ce_video_instance_render(ce_video_instance* video_instance)
{
    ce_render_system_apply_material(video_instance->material);
    if (ce_texture_is_valid(video_instance->texture)) {
        ce_texture_bind(video_instance->texture);
        ce_render_system_draw_fullscreen_wire_rect(video_instance->texture->width,
                                                video_instance->texture->height);
        ce_texture_unbind(video_instance->texture);
    }
    ce_render_system_discard_material(video_instance->material);
}

bool ce_video_instance_is_stopped(ce_video_instance* video_instance)
{
    return 0 != video_instance->sound_object ?
        ce_sound_object_is_stopped(video_instance->sound_object) :
        CE_VIDEO_INSTANCE_STATE_STOPPED == video_instance->state;
}

void ce_video_instance_play(ce_video_instance* video_instance)
{
    ce_sound_object_play(video_instance->sound_object);
    video_instance->state = CE_VIDEO_INSTANCE_STATE_PLAYING;
}

void ce_video_instance_pause(ce_video_instance* video_instance)
{
    ce_sound_object_pause(video_instance->sound_object);
    video_instance->state = CE_VIDEO_INSTANCE_STATE_PAUSED;
}

void ce_video_instance_stop(ce_video_instance* video_instance)
{
    ce_sound_object_stop(video_instance->sound_object);
    video_instance->state = CE_VIDEO_INSTANCE_STATE_STOPPED;
}
