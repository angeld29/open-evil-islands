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
#include <cstdint>
#include <cstring>
#include <vector>

#include "lib.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "event.hpp"
#include "soundsystem.hpp"
#include "soundmixer.hpp"

namespace cursedearth
{
    struct ce_sound_mixer* ce_sound_mixer;

    void ce_sound_mixer_create_buffer_react(ce_event* event)
    {
        ce_sound_buffer* sound_buffer = (ce_sound_buffer*)((ce_event_ptr*)event->impl)->ptr;
        ce_vector_push_back(ce_sound_mixer->sound_buffers, sound_buffer);
    }

    void ce_sound_mixer_destroy_buffer_react(ce_event* event)
    {
        ce_sound_buffer* sound_buffer = (ce_sound_buffer*)((ce_event_ptr*)event->impl)->ptr;
        ce_vector_remove_all(ce_sound_mixer->sound_buffers, sound_buffer);
        ce_sound_buffer_del(sound_buffer);
    }

    void ce_sound_mixer_convert_sample_s16_s16(int16_t* sample1, const int16_t* sample2, const ce_sound_format* sound_format1, const ce_sound_format* sound_format2)
    {
        for (size_t i = 0; i < sound_format1->channel_count; ++i) {
            if (0 == i || i < sound_format2->channel_count) {
                sample1[i] = sample2[i];
            } else {
                sample1[i] = sample1[i - 1];
            }
        }
    }

    void ce_sound_mixer_convert_sample(void* sample1, const void* sample2, const ce_sound_format* sound_format1, const ce_sound_format* sound_format2)
    {
        if (16 == sound_format1->bits_per_sample && 16 == sound_format2->bits_per_sample) {
            ce_sound_mixer_convert_sample_s16_s16(static_cast<int16_t*>(sample1), static_cast<const int16_t*>(sample2), sound_format1, sound_format2);
        } else {
            assert(false && "not implemented");
        }
    }

    void ce_sound_mixer_mix_sample_s16(int16_t* sample, const int16_t* other, const ce_sound_format* sound_format)
    {
        for (size_t i = 0; i < sound_format->channel_count; ++i) {
            int32_t value = (int32_t)sample[i] + (int32_t)other[i];
            sample[i] = ce_clamp(int32_t, value, INT16_MIN, INT16_MAX);
        }
    }

    void ce_sound_mixer_mix_sample(void* sample, const void* other, const ce_sound_format* sound_format)
    {
        switch (sound_format->bits_per_sample) {
        case 16:
            ce_sound_mixer_mix_sample_s16(static_cast<int16_t*>(sample), static_cast<const int16_t*>(other), sound_format);
            break;
        default:
            assert(false && "not implemented");
        }
    }

    void ce_sound_mixer_exec(void*)
    {
        while (!ce_sound_mixer->done) {
            std::vector<char> sample(ce_sound_system->sound_format.sample_size);

            char* block = static_cast<char*>(ce_sound_system_map_block());
            memset(block, 0, CE_SOUND_SYSTEM_BLOCK_SIZE);

            for (size_t i = 0; i < CE_SOUND_SYSTEM_SAMPLES_IN_BLOCK; ++i, block += ce_sound_system->sound_format.sample_size) {
                for (size_t j = 0; j < ce_sound_mixer->sound_buffers->count; ++j) {
                    ce_sound_buffer* sound_buffer = (ce_sound_buffer*)ce_sound_mixer->sound_buffers->items[j];
                    if (ce_sound_buffer_is_one_sample_ready(sound_buffer)) {
                        std::vector<char> sample2(sound_buffer->sound_format.sample_size);
                        ce_sound_buffer_read_one_sample(sound_buffer, sample2.data());
                        ce_sound_mixer_convert_sample(sample.data(), sample2.data(), &ce_sound_system->sound_format, &sound_buffer->sound_format);
                        ce_sound_mixer_mix_sample(block, sample.data(), &ce_sound_system->sound_format);
                    }
                }
            }

            ce_sound_system_unmap_block();
            ce_event_manager_process_events(ce_thread_self(), CE_EVENT_FLAG_ALL_EVENTS);
        }
    }

    void ce_sound_mixer_init(void)
    {
        ce_sound_mixer = (struct ce_sound_mixer*)ce_alloc_zero(sizeof(struct ce_sound_mixer));
        ce_sound_mixer->sound_buffers = ce_vector_new();
        ce_sound_mixer->thread = ce_thread_new((void(*)())ce_sound_mixer_exec, NULL);
    }

    void ce_sound_mixer_term(void)
    {
        if (NULL != ce_sound_mixer) {
            ce_sound_mixer->done = true;
            ce_thread_wait(ce_sound_mixer->thread);
            ce_thread_del(ce_sound_mixer->thread);
            if (!ce_vector_empty(ce_sound_mixer->sound_buffers)) {
                ce_logging_warning("sound mixer: some buffers have not been unregistered");
            }
            ce_vector_for_each(ce_sound_mixer->sound_buffers, (void(*)(void*))ce_sound_buffer_del);
            ce_vector_del(ce_sound_mixer->sound_buffers);
            ce_free(ce_sound_mixer, sizeof(struct ce_sound_mixer));
        }
    }

    ce_sound_buffer* ce_sound_mixer_create_buffer(void)
    {
        ce_sound_buffer* sound_buffer = ce_sound_buffer_new(2 * CE_SOUND_SYSTEM_BLOCK_SIZE);
        ce_event_manager_post_ptr(ce_thread_get_id(ce_sound_mixer->thread), ce_sound_mixer_create_buffer_react, sound_buffer);
        return sound_buffer;
    }

    void ce_sound_mixer_destroy_buffer(ce_sound_buffer* sound_buffer)
    {
        ce_event_manager_post_ptr(ce_thread_get_id(ce_sound_mixer->thread), ce_sound_mixer_destroy_buffer_react, sound_buffer);
    }
}
