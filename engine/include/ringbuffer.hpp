/*
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

#include "concurrency.hpp"

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

        /**
         * @brief get write space to write data
         * @return number of bytes that can be pushed to the ring buffer
         */
        size_t write_available() const { return ce_semaphore_available(m_free_data); }

        /**
         * @brief get number of bytes that are available for read
         * @return number of available bytes that can be popped from the ring buffer
         */
        size_t read_available() const { return ce_semaphore_available(m_used_data); }

        /**
         * @brief pushes as many bytes from the array as there is space
         * @param wait if true, blocks until more space is available
         * @return number of pushed bytes
         */
        size_t push(const void*, size_t, bool wait = true);

        /**
         * @brief pops a maximum of size bytes from ring buffer
         * @param wait if true, blocks until new data is available
         * @return number of popped bytes
         */
        size_t pop(void*, size_t, bool wait = true);

        void write(const void*, size_t);
        void read(void*, size_t);

    private:
        const size_t m_capacity;
        semaphore_t* m_free_data;
        semaphore_t* m_used_data;
        size_t m_start = 0, m_end = 0;
        std::unique_ptr<uint8_t[]> m_data;
    };

    typedef std::shared_ptr<ring_buffer_t> ring_buffer_ptr_t;
}

#endif
