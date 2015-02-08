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

#include <cstring>

#include "logging.hpp"
#include "rendersystem.hpp"
#include "shadermanager.hpp"
#include "videoinstance.hpp"

namespace cursedearth
{
    video_instance_t::video_instance_t(sound_object_t object, ce_video_resource* resource):
        m_object(object),
        m_resource(resource),
        m_texture(ce_texture_new("frame", NULL)),
        m_material(ce_material_new()),
        m_rgba_frame(ce_mmpfile_new(resource->width, resource->height, 1, CE_MMPFILE_FORMAT_R8G8B8A8, 0)),
        m_prepared_frames(ce_semaphore_new(0)),
        m_unprepared_frames(ce_semaphore_new(s_cache_size)),
        m_done(false)
    {
        const char* shaders[] = { "shaders/ycbcr2rgba.vert", "shaders/ycbcr2rgba.frag", NULL };
        m_material->mode = CE_MATERIAL_MODE_REPLACE;
        m_material->shader = ce_shader_manager_get(shaders);

        if (NULL != m_material->shader) {
            ce_shader_add_ref(m_material->shader);
        }

        for (size_t i = 0; i < s_cache_size; ++i) {
            m_ycbcr_frames[i] = ce_mmpfile_new(resource->width, resource->height, 1, CE_MMPFILE_FORMAT_YCBCR, 0);
        }

        m_thread = ce_thread_new((void(*)())execute, this);
    }

    video_instance_t::~video_instance_t()
    {
        m_done = true;
        ce_semaphore_release(m_unprepared_frames, 1);
        ce_thread_wait(m_thread);
        ce_thread_del(m_thread);
        ce_semaphore_del(m_unprepared_frames);
        ce_semaphore_del(m_prepared_frames);
        for (size_t i = 0; i < s_cache_size; ++i) {
            ce_mmpfile_del(m_ycbcr_frames[i]);
        }
        ce_mmpfile_del(m_rgba_frame);
        ce_material_del(m_material);
        ce_texture_del(m_texture);
        ce_video_resource_del(m_resource);
        remove_sound_object(m_object);
    }

    void video_instance_t::advance(float elapsed)
    {
        if (state_t::paused == m_state) {
            return;
        }

        if (sound_object_is_valid(m_object)) {
            // synchronization with sound
            float sound_time = get_sound_object_time(m_object);
            if (m_sync_time != sound_time) {
                m_sync_time = sound_time;
                m_play_time = sound_time;
            } else {
                m_play_time += elapsed;
            }
        } else {
            m_play_time += elapsed;
        }

        do_advance();
    }

    void video_instance_t::progress(int percents)
    {
        m_play_time = (m_resource->frame_count / m_resource->fps) * (0.01f * percents);
        do_advance();
    }

    void video_instance_t::render()
    {
        ce_render_system_apply_material(m_material);
        if (ce_texture_is_valid(m_texture)) {
            ce_texture_bind(m_texture);
            ce_render_system_draw_fullscreen_wire_rect(m_texture->width, m_texture->height);
            ce_texture_unbind(m_texture);
        }
        ce_render_system_discard_material(m_material);
    }

    bool video_instance_t::is_stopped()
    {
        return sound_object_is_valid(m_object) ? sound_object_is_stopped(m_object) : state_t::stopped == m_state;
    }

    void video_instance_t::play()
    {
        play_sound_object(m_object);
        m_state = state_t::playing;
    }

    void video_instance_t::pause()
    {
        pause_sound_object(m_object);
        m_state = state_t::paused;
    }

    void video_instance_t::stop()
    {
        stop_sound_object(m_object);
        m_state = state_t::stopped;
    }

    void video_instance_t::do_advance()
    {
        bool acquired = false;
        const int desired_frame = m_resource->fps * m_play_time;

        // if sound or time far away
        while (m_frame < desired_frame && ce_semaphore_try_acquire(m_prepared_frames, 1)) {
            // skip frames to reach desired frame
            if (++m_frame == desired_frame || /* or use the closest frame */ 0 == ce_semaphore_available(m_prepared_frames)) {
                ce_mmpfile* ycbcr_frame = m_ycbcr_frames[m_frame % s_cache_size];

                if (NULL != m_material->shader) {
                    const uint8_t* y_data = static_cast<const uint8_t*>(ycbcr_frame->texels);
                    const uint8_t* cb_data = y_data + ycbcr_frame->width * ycbcr_frame->height;
                    const uint8_t* cr_data = cb_data + (ycbcr_frame->width / 2) * (ycbcr_frame->height / 2);

                    uint8_t* texels = static_cast<uint8_t*>(m_rgba_frame->texels);

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
                    ce_mmpfile_convert2(ycbcr_frame, m_rgba_frame);
                }

                ce_texture_replace(m_texture, m_rgba_frame);
                acquired = true;
            }
            ce_semaphore_release(m_unprepared_frames, 1);
        }

        // TODO: think again how to hold last frame
        if (state_t::stopping == m_state && !acquired && 0 == ce_semaphore_available(m_prepared_frames)) {
            m_state = state_t::stopped;
        }
    }

    void video_instance_t::execute(video_instance_t* instance)
    {
        for (size_t i = ce_semaphore_available(instance->m_prepared_frames); !instance->m_done; ++i) {
            ce_semaphore_acquire(instance->m_unprepared_frames, 1);

            if (!ce_video_resource_read(instance->m_resource)) {
                instance->m_state = state_t::stopping;
                break;
            }

            ce_mmpfile* ycbcr_frame = instance->m_ycbcr_frames[i % s_cache_size];
            ycbcr_t* ycbcr = &instance->m_resource->ycbcr;

            uint8_t* y_data = static_cast<uint8_t*>(ycbcr_frame->texels);
            uint8_t* cb_data = y_data + ycbcr_frame->width * ycbcr_frame->height;
            uint8_t* cr_data = cb_data + (ycbcr_frame->width / 2) * (ycbcr_frame->height / 2);

            int y_offset = (ycbcr->crop_rect.x & ~1) + ycbcr->planes[0].stride * (ycbcr->crop_rect.y & ~1);
            int cb_offset = (ycbcr->crop_rect.x / 2) + ycbcr->planes[1].stride * (ycbcr->crop_rect.y / 2);
            int cr_offset = (ycbcr->crop_rect.x / 2) + ycbcr->planes[2].stride * (ycbcr->crop_rect.y / 2);

            for (unsigned int h = 0; h < ycbcr->crop_rect.height; ++h) {
                memcpy(y_data + h * ycbcr->crop_rect.width, ycbcr->planes[0].data + y_offset + h * ycbcr->planes[0].stride, ycbcr->crop_rect.width);
            }

            for (unsigned int h = 0; h < ycbcr->crop_rect.height / 2; ++h) {
                memcpy(cb_data + h * (ycbcr->crop_rect.width / 2), ycbcr->planes[1].data + cb_offset + h * ycbcr->planes[1].stride, ycbcr->crop_rect.width / 2);
                memcpy(cr_data + h * (ycbcr->crop_rect.width / 2), ycbcr->planes[2].data + cr_offset + h * ycbcr->planes[2].stride, ycbcr->crop_rect.width / 2);
            }

            ce_semaphore_release(instance->m_prepared_frames, 1);
        }
    }
}
