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

#ifndef CE_SOUNDFORMAT_HPP
#define CE_SOUNDFORMAT_HPP

#include "soundcapabilities.hpp"

namespace cursedearth
{
    struct sound_format_t
    {
        size_t bits_per_sample;
        size_t samples_per_second;
        size_t channel_count;
        size_t sample_size;
        size_t bytes_per_second;

        sound_format_t(size_t bits_per_sample, size_t samples_per_second, size_t channel_count):
            bits_per_sample(bits_per_sample), samples_per_second(samples_per_second), channel_count(channel_count),
            sample_size(channel_count * (bits_per_sample / 8)), bytes_per_second(samples_per_second * sample_size) {}
    };

    inline bool operator ==(const sound_format_t& lhs, const sound_format_t& rhs)
    {
        return lhs.bits_per_sample    == rhs.bits_per_sample    &&
               lhs.samples_per_second == rhs.samples_per_second &&
               lhs.channel_count      == rhs.channel_count;
    }

    inline sound_format_t make_native_format()
    {
        return sound_format_t(sound_native_capabilities_t::bits_per_sample, sound_native_capabilities_t::samples_per_second, sound_native_capabilities_t::channel_count);
    }
}

#endif
