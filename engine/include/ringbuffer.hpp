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

#ifndef CE_RINGBUFFER_HPP
#define CE_RINGBUFFER_HPP

#include <memory>

#include <boost/noncopyable.hpp>

#include "thread.hpp"

namespace cursedearth
{
    /**
     * @brief Single-Producer/Single-Consumer Queue
     */
    class ring_buffer_t final: boost::noncopyable
    {
    public:
        explicit ring_buffer_t(size_t capacity);
        ~ring_buffer_t();

        size_t write_available() const { return ce_semaphore_available(m_free_data); }
        size_t read_available() const { return ce_semaphore_available(m_used_data); }

        void push(const void*, size_t);
        void pop(void*, size_t);

    private:
        const size_t m_capacity;
        ce_semaphore* m_free_data;
        ce_semaphore* m_used_data;
        size_t m_start = 0, m_end = 0;
        std::unique_ptr<uint8_t[]> m_data;
    };

    typedef std::shared_ptr<ring_buffer_t> ring_buffer_ptr_t;
}

#endif
