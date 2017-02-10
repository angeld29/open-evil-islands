/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include "soundmixer.hpp"
#include "soundsystem.hpp"
#include "utility.hpp"

namespace cursedearth
{
    sound_mixer_t::sound_mixer_t():
        singleton_t<sound_mixer_t>(this),
        m_thread("sound mixer", [this]{execute();})
    {
    }

    sound_mixer_t::~sound_mixer_t()
    {
        if (!m_buffers.empty()) {
            ce_logging_warning("sound mixer: some buffers have not been unregistered");
        }
    }

    sound_buffer_ptr_t sound_mixer_t::make_buffer(const sound_format_t& format)
    {
        sound_buffer_ptr_t buffer = std::make_shared<sound_buffer_t>(format);
        std::lock_guard<std::mutex> lock(m_mutex);
        std::ignore = lock;
        m_buffers.push_back(buffer);
        return buffer;
    }

    void convert_sample_s16_s16(int16_t* native, const int16_t* foreign, const sound_format_t& native_format, const sound_format_t& foreign_format)
    {
        for (size_t i = 0; i < native_format.channel_count; ++i) {
            native[i] = (0 == i || i < foreign_format.channel_count) ? foreign[i] : native[i - 1];
        }
    }

    void convert_sample(void* native, const void* foreign, const sound_format_t& native_format, const sound_format_t& foreign_format)
    {
        if (16 != native_format.bits_per_sample || 16 != foreign_format.bits_per_sample) {
            assert(false && "not implemented");
        }
        convert_sample_s16_s16(static_cast<int16_t*>(native), static_cast<const int16_t*>(foreign), native_format, foreign_format);
    }

    void mix_sample_s16(int16_t* sample, const int16_t* other, const sound_format_t& format)
    {
        for (size_t i = 0; i < format.channel_count; ++i) {
            int32_t value = static_cast<int32_t>(sample[i]) + static_cast<int32_t>(other[i]);
            sample[i] = clamp(value, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
        }
    }

    void mix_sample(void* sample, const void* other, const sound_format_t& format)
    {
        if (16 != format.bits_per_sample) {
            assert(false && "not implemented");
        }
        mix_sample_s16(static_cast<int16_t*>(sample), static_cast<const int16_t*>(other), format);
    }

    void sound_mixer_t::execute()
    {
        uint8_t array[sound_options_t::max_block_size];
        uint8_t native_sample[sound_options_t::max_sample_size];
        uint8_t foreign_sample[sound_options_t::max_sample_size];
        while (true) {
            uint8_t* data = array;
            for (size_t i = 0; i < sound_options_t::samples_in_block; ++i, data += sound_system_t::instance()->format().sample_size) {
                std::fill_n(data, sound_system_t::instance()->format().sample_size, 0);
                std::lock_guard<std::mutex> lock(m_mutex);
                std::ignore = lock;
                for (const auto& buffer: m_buffers) {
                    if (!buffer->sleeping() && buffer->try_read_one_sample(foreign_sample)) {
                        convert_sample(native_sample, foreign_sample, sound_system_t::instance()->format(), buffer->format());
                        mix_sample(data, native_sample, buffer->format());
                    }
                }
                interruption_point();
            }
            sound_block_ptr_t block = sound_system_t::instance()->map();
            block->write(array, block->format().sample_size * sound_options_t::samples_in_block);
            sound_system_t::instance()->unmap(block);
        }
    }
}
