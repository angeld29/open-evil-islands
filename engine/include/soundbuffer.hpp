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

#ifndef CE_SOUNDBUFFER_HPP
#define CE_SOUNDBUFFER_HPP

#include <cstddef>
#include <cstdint>

#include "thread.hpp"
#include "soundfeature.hpp"
#include "soundformat.hpp"

namespace cursedearth
{
    struct ce_sound_buffer
    {
        ce_sound_format sound_format;
        ce_semaphore* free_blocks;
        ce_semaphore* used_blocks;
        size_t next_block;
        uint8_t blocks[sound_feature_t::BLOCK_COUNT][sound_feature_t::BLOCK_SIZE];
    };

    ce_sound_buffer* ce_sound_buffer_new();
    void ce_sound_buffer_del(ce_sound_buffer* sound_buffer);

    void ce_sound_buffer_read(ce_sound_buffer* sound_buffer, void* buffer, size_t size);
    void ce_sound_buffer_write(ce_sound_buffer* sound_buffer, const void* buffer, size_t size);

    inline size_t ce_sound_buffer_available_size_for_read(ce_sound_buffer* sound_buffer)
    {
        return ce_semaphore_available(sound_buffer->prepared_data);
    }

    inline size_t ce_sound_buffer_available_size_for_write(ce_sound_buffer* sound_buffer)
    {
        return ce_semaphore_available(sound_buffer->unprepared_data);
    }

    inline bool ce_sound_buffer_is_one_sample_ready(ce_sound_buffer* sound_buffer)
    {
        return ce_sound_buffer_available_size_for_read(sound_buffer) >= sound_buffer->sound_format.sample_size;
    }

    inline void ce_sound_buffer_read_one_sample(ce_sound_buffer* sound_buffer, void* buffer)
    {
        ce_sound_buffer_read(sound_buffer, buffer, sound_buffer->sound_format.sample_size);
    }

    void* ce_sound_buffer_map_block(void)
    {
        ce_semaphore_acquire(ce_sound_system->free_blocks, 1);
        return ce_sound_system->blocks[ce_sound_system->next_block++ % CE_SOUND_SYSTEM_BLOCK_COUNT];
    }

    void ce_sound_buffer_unmap_block(void)
    {
        ce_semaphore_release(ce_sound_system->used_blocks, 1);
    }
}

#endif
