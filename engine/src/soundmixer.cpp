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
#include <cstring>
#include <algorithm>

#include "utility.hpp"
#include "logging.hpp"
#include "soundsystem.hpp"
#include "soundmixer.hpp"

namespace cursedearth
{
    sound_mixer_t::sound_mixer_t():
        singleton_t<sound_mixer_t>(this),
        m_done(false),
        m_thread(std::bind(&sound_mixer_t::execute, this))
    {
    }

    sound_mixer_t::~sound_mixer_t()
    {
        m_done = true;
        m_thread.join();
        if (!m_buffers.empty()) {
            ce_logging_warning("sound mixer: some buffers have not been unregistered");
        }
    }

    sound_buffer_ptr_t sound_mixer_t::make_buffer(const sound_format_t& format)
    {
        sound_buffer_ptr_t buffer = std::make_shared<sound_buffer_t>(format);
        std::lock_guard<std::mutex> lock(m_mutex);
        m_buffers.push_back(buffer);
        return buffer;
    }

    void convert_sample_s16(int16_t* sample, const int16_t* other, const sound_format_t& format)
    {
        for (size_t i = 0; i < SOUND_CAPABILITY_CHANNEL_COUNT; ++i) {
            sample[i] = (0 == i || i < format.channel_count) ? other[i] : sample[i - 1];
        }
    }

    void convert_sample(void* sample, const void* other, const sound_format_t& format)
    {
        switch (format.bits_per_sample) {
        case 16:
            convert_sample_s16(static_cast<int16_t*>(sample), static_cast<const int16_t*>(other), format);
            break;
        default:
            assert(false && "not implemented");
        }
    }

    void mix_sample_s16(int16_t* sample, const int16_t* other)
    {
        for (size_t i = 0; i < SOUND_CAPABILITY_CHANNEL_COUNT; ++i) {
            int32_t value = static_cast<int32_t>(sample[i]) + static_cast<int32_t>(other[i]);
            sample[i] = clamp(value, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
        }
    }

    void mix_sample(void* sample, const void* other)
    {
        mix_sample_s16(static_cast<int16_t*>(sample), static_cast<const int16_t*>(other));
    }

    void sound_mixer_t::execute()
    {
        uint8_t block[SOUND_CAPABILITY_BLOCK_SIZE];
        uint8_t native_sample[SOUND_CAPABILITY_SAMPLE_SIZE];
        uint8_t foreign_sample[SOUND_CAPABILITY_MAX_SAMPLE_SIZE];

        while (!m_done) {
            uint8_t* data = block;
            for (size_t i = 0; i < SOUND_CAPABILITY_SAMPLES_IN_BLOCK; ++i, data += SOUND_CAPABILITY_SAMPLE_SIZE) {
                memset(data, 0, SOUND_CAPABILITY_SAMPLE_SIZE);
                std::lock_guard<std::mutex> lock(m_mutex);
                for (const auto& buffer: m_buffers) {
                    if (buffer->pop(foreign_sample, false)) {
                        convert_sample(native_sample, foreign_sample, buffer->format());
                        mix_sample(data, native_sample);
                    }
                }
            }
            sound_system_t::instance()->write(block);
        }
    }

    sound_mixer_ptr_t make_sound_mixer()
    {
        return make_unique<sound_mixer_t>();
    }
}
