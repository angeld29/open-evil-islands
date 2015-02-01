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

#ifndef CE_SOUNDMIXER_HPP
#define CE_SOUNDMIXER_HPP

#include "vector.hpp"
#include "thread.hpp"
#include "soundbuffer.hpp"

namespace cursedearth
{
    extern struct ce_sound_mixer {
        bool done;
        ce_vector* sound_buffers;
        thread_t* thread;
    }* ce_sound_mixer;

    extern void ce_sound_mixer_init(void);
    extern void ce_sound_mixer_term(void);

    extern ce_sound_buffer* ce_sound_mixer_create_buffer(void);
    extern void ce_sound_mixer_destroy_buffer(ce_sound_buffer* sound_buffer);
}

#endif /* CE_SOUNDMIXER_HPP */
