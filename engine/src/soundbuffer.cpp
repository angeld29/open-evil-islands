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

#include <cstdint>
#include <cstring>
#include <cassert>

#include "lib.hpp"
#include "alloc.hpp"
#include "soundbuffer.hpp"

namespace cursedearth
{
    ce_sound_buffer* ce_sound_buffer_new(size_t capacity)
    {
        ce_sound_buffer* sound_buffer = (ce_sound_buffer*)ce_alloc_zero(sizeof(ce_sound_buffer));
        sound_buffer->data = (uint8_t*)ce_alloc_zero(capacity);
        sound_buffer->capacity = capacity;
        sound_buffer->prepared_data = ce_semaphore_new(0);
        sound_buffer->unprepared_data = ce_semaphore_new(capacity);
        return sound_buffer;
    }

    void ce_sound_buffer_del(ce_sound_buffer* sound_buffer)
    {
        if (NULL != sound_buffer) {
            ce_semaphore_del(sound_buffer->unprepared_data);
            ce_semaphore_del(sound_buffer->prepared_data);
            ce_free(sound_buffer->data, sound_buffer->capacity);
            ce_free(sound_buffer, sizeof(ce_sound_buffer));
        }
    }

    void ce_sound_buffer_read(ce_sound_buffer* sound_buffer, void* buffer, size_t size)
    {
        uint8_t* data = static_cast<uint8_t*>(buffer);

        assert(0 == size % sound_buffer->sound_format.sample_size);
        ce_semaphore_acquire(sound_buffer->prepared_data, size);

        if (size > sound_buffer->capacity - sound_buffer->start) {
            size_t length = sound_buffer->capacity - sound_buffer->start;
            memcpy(data, sound_buffer->data + sound_buffer->start, length);
            memcpy(data + length, sound_buffer->data, size - length);
        } else {
            memcpy(data, sound_buffer->data + sound_buffer->start, size);
        }

        sound_buffer->start = (sound_buffer->start + size) % sound_buffer->capacity;
        ce_semaphore_release(sound_buffer->unprepared_data, size);
    }

    void ce_sound_buffer_write(ce_sound_buffer* sound_buffer, const void* buffer, size_t size)
    {
        const uint8_t* data = static_cast<const uint8_t*>(buffer);

        assert(0 == size % sound_buffer->sound_format.sample_size);
        ce_semaphore_acquire(sound_buffer->unprepared_data, size);

        if (size > sound_buffer->capacity - sound_buffer->end) {
            size_t length = sound_buffer->capacity - sound_buffer->end;
            memcpy(sound_buffer->data + sound_buffer->end, data, length);
            memcpy(sound_buffer->data, data + length, size - length);
        } else {
            memcpy(sound_buffer->data + sound_buffer->end, data, size);
        }

        sound_buffer->end = (sound_buffer->end + size) % sound_buffer->capacity;
        ce_semaphore_release(sound_buffer->prepared_data, size);
    }
}
