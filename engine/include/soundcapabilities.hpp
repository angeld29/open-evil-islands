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

#ifndef CE_SOUNDCAPABILITIES_HPP
#define CE_SOUNDCAPABILITIES_HPP

namespace cursedearth
{
    enum sound_capabilities_t {
        SOUND_CAPABILITY_BITS_PER_SAMPLE = 16,
        SOUND_CAPABILITY_SAMPLES_PER_SECOND = 44100,
        SOUND_CAPABILITY_CHANNEL_COUNT = 2,
        SOUND_CAPABILITY_SAMPLE_SIZE = SOUND_CAPABILITY_CHANNEL_COUNT * (SOUND_CAPABILITY_BITS_PER_SAMPLE / 8),
        SOUND_CAPABILITY_SAMPLES_IN_BLOCK = 1024,
        SOUND_CAPABILITY_BLOCK_COUNT = 32,
        SOUND_CAPABILITY_MAX_SAMPLE_SIZE = 8 * (64 / 8),
        SOUND_CAPABILITY_MAX_BLOCK_SIZE = SOUND_CAPABILITY_MAX_SAMPLE_SIZE * SOUND_CAPABILITY_SAMPLES_IN_BLOCK
    };
}

#endif
