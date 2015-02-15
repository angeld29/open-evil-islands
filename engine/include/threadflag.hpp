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

#ifndef CE_THREADFLAG_HPP
#define CE_THREADFLAG_HPP

#include "conditionvariable.hpp"

namespace cursedearth
{
    extern thread_local class thread_flag_t: untransferable_t
    {
    public:
        thread_flag_t(): m_flag(false) {}

        explicit operator bool() const { return m_flag; }

        void operator =(bool flag)
        {
            m_flag = flag;
            std::lock_guard<std::mutex> lock(m_mutex);
            std::ignore = lock;
            if (m_condition_variable) {
                m_condition_variable->notify_all();
            }
        }

        void set_condition_variable(condition_variable_t& condition_variable)
        {
            m_mutex.lock();
            assert(!m_condition_variable);
            m_condition_variable = &condition_variable;
        }

        void reset_condition_variable()
        {
            assert(m_condition_variable);
            m_condition_variable = nullptr;
            m_mutex.unlock();
        }

        template <typename lockable_t>
        void lock(lockable_t& lockable)
        {
            std::lock(m_mutex, lockable);
        }

        template <typename lockable_t>
        void unlock(lockable_t& lockable)
        {
            lockable.unlock();
            m_mutex.unlock();
        }

    private:
        std::atomic<bool> m_flag;
        std::mutex m_mutex;
        condition_variable_t* m_condition_variable = nullptr;
    } g_thread_flag;
}

#endif
