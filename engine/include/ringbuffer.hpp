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

#include "commonheaders.hpp"

#include <boost/lockfree/spsc_queue.hpp>

namespace cursedearth
{
    /**
     * @brief Single-Producer/Single-Consumer Queue
     */
    template <typename T, size_t n>
    class ring_buffer_t final: boost::noncopyable
    {
        struct acquire_in_ctor_release_in_dtor_t
        {
            semaphore_ptr_t for_release;

            acquire_in_ctor_release_in_dtor_t(const semaphore_ptr_t& for_acquire, const semaphore_ptr_t& for_release):
                for_release(for_release)
            {
                for_acquire->acquire();
            }

            ~acquire_in_ctor_release_in_dtor_t()
            {
                for_release->release();
            }
        };

    public:
        ring_buffer_t():
            m_free_items(make_semaphore(n)),
            m_used_items(make_semaphore(0)) {}

        /**
         * @brief get write space to write elements
         * @return number of elements that can be pushed to the ring buffer
         */
        size_t write_available() const { return m_free_items->available(); }

        /**
         * @brief get number of elements that are available for read
         * @return number of available elements that can be popped from the ring buffer
         */
        size_t read_available() const { return m_used_items->available(); }

        /**
         * @brief pushes element to the ringbuffer
         * @param wait blocks until more space is available
         * @return true if the push operation is successful, null otherwise
         */
        bool push(const T& item, bool wait = true)
        {
            if (0 == write_available() && !wait) {
                return false;
            }

            acquire_in_ctor_release_in_dtor_t helper(m_free_items, m_used_items);
            std::ignore = helper;

            bool ok = m_items.push(item);
            std::ignore = ok;
            assert(ok);

            return true;
        }

        /**
         * @brief pops element from ringbuffer
         * @param wait blocks until new element is available
         * @return true if the pop operation is successful, null otherwise
         */
        bool pop(T& item, bool wait = true)
        {
            if (0 == read_available() && !wait) {
                return false;
            }

            acquire_in_ctor_release_in_dtor_t helper(m_used_items, m_free_items);
            std::ignore = helper;

            bool ok = m_items.pop(item);
            std::ignore = ok;
            assert(ok);

            return true;
        }

    private:
        semaphore_ptr_t m_free_items, m_used_items;
        boost::lockfree::spsc_queue<T, boost::lockfree::capacity<n>> m_items;
    };
}

#endif
