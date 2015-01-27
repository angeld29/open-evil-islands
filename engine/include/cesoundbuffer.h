/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#ifndef CE_SOUNDBUFFER_H
#define CE_SOUNDBUFFER_H

#include <stddef.h>
#include <stdbool.h>

#include "cethread.h"
#include "cesoundformat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ce_sound_format sound_format;
    size_t capacity, start, end;
    ce_semaphore* prepared_data;
    ce_semaphore* unprepared_data;
    char data[];
} ce_sound_buffer;

extern ce_sound_buffer* ce_sound_buffer_new(size_t capacity);
extern void ce_sound_buffer_del(ce_sound_buffer* sound_buffer);

extern void ce_sound_buffer_read(ce_sound_buffer* sound_buffer,
                                void* buffer, size_t size);

extern void ce_sound_buffer_write(ce_sound_buffer* sound_buffer,
                                const void* buffer, size_t size);

static inline size_t ce_sound_buffer_available_size_for_read(ce_sound_buffer* sound_buffer)
{
    return ce_semaphore_available(sound_buffer->prepared_data);
}

static inline size_t ce_sound_buffer_available_size_for_write(ce_sound_buffer* sound_buffer)
{
    return ce_semaphore_available(sound_buffer->unprepared_data);
}

static inline bool ce_sound_buffer_is_one_sample_ready(ce_sound_buffer* sound_buffer)
{
    return ce_sound_buffer_available_size_for_read(sound_buffer) >= sound_buffer->sound_format.sample_size;
}

static inline void ce_sound_buffer_read_one_sample(ce_sound_buffer* sound_buffer, void* buffer)
{
    ce_sound_buffer_read(sound_buffer, buffer, sound_buffer->sound_format.sample_size);
}

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDBUFFER_H */
