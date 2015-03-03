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

#ifndef CE_THREADLOCK_HPP
#define CE_THREADLOCK_HPP

#include "threadflag.hpp"

namespace cursedearth
{
    /**
     * @brief custom lock with interruption support
     */
    class thread_lock_t: untransferable_t
    {
    public:
        thread_lock_t(std::mutex& mutex, const condition_variable_ptr_t& condition_variable):
            m_owns(false),
            m_mutex(mutex)
        {
            g_thread_flag.lock_and_set_cv(mutex, condition_variable);
            m_owns = true;

            assert(!s_nested_guard && "nested locks are not supported");
            s_nested_guard = true;

            s_thread_id = std::this_thread::get_id();
        }

        ~thread_lock_t()
        {
            assert(s_nested_guard);
            assert(std::this_thread::get_id() == s_thread_id);
            g_thread_flag.reset_cv();
            if (m_owns) {
                unlock();
            }
            s_nested_guard = false;
        }

        void lock()
        {
            assert(s_nested_guard);
            assert(std::this_thread::get_id() == s_thread_id);
            g_thread_flag.lock(m_mutex);
            m_owns = true;
        }

        void unlock()
        {
            assert(s_nested_guard);
            assert(std::this_thread::get_id() == s_thread_id);
            g_thread_flag.unlock(m_mutex);
            m_owns = false;
        }

    private:
        static thread_local std::atomic<bool> s_nested_guard;
        static thread_local std::thread::id s_thread_id;

        std::atomic<bool> m_owns;
        std::mutex& m_mutex;
    };
}

#endif
