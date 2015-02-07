/**
 * This file is part of Cursed Earth.
 *
 * Cursed Earth is an open source, cross-platform port of Evil Islands.
 * Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstring>

#include "makeunique.hpp"
#include "ringbuffer.hpp"

namespace cursedearth
{
    ring_buffer_t::ring_buffer_t(size_t capacity):
        m_capacity(capacity),
        m_free_data(ce_semaphore_new(capacity)),
        m_used_data(ce_semaphore_new(0)),
        m_data(make_unique<uint8_t[]>(capacity))
    {
    }

    ring_buffer_t::~ring_buffer_t()
    {
        ce_semaphore_del(m_used_data);
        ce_semaphore_del(m_free_data);
    }

    void ring_buffer_t::push(const void* buffer, size_t size)
    {
        ce_semaphore_acquire(m_free_data, size);
        const uint8_t* data = static_cast<const uint8_t*>(buffer);
        if (size > m_capacity - m_end) {
            size_t length = m_capacity - m_end;
            memcpy(m_data.get() + m_end, data, length);
            memcpy(m_data.get(), data + length, size - length);
        } else {
            memcpy(m_data.get() + m_end, data, size);
        }
        m_end = (m_end + size) % m_capacity;
        ce_semaphore_release(m_used_data, size);
    }

    void ring_buffer_t::pop(void* buffer, size_t size)
    {
        ce_semaphore_acquire(m_used_data, size);
        uint8_t* data = static_cast<uint8_t*>(buffer);
        if (size > m_capacity - m_start) {
            size_t length = m_capacity - m_start;
            memcpy(data, m_data.get() + m_start, length);
            memcpy(data + length, m_data.get(), size - length);
        } else {
            memcpy(data, m_data.get() + m_start, size);
        }
        m_start = (m_start + size) % m_capacity;
        ce_semaphore_release(m_free_data, size);
    }
}
