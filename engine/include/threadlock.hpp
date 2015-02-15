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
     * @brief custom lock for condition variable with interruption support
     */
    template <class lockable_t>
    class thread_lock_t: untransferable_t
    {
    public:
        thread_lock_t(lockable_t& lockable, const condition_variable_ptr_t& condition_variable):
            m_owns(false),
            m_lockable(lockable)
        {
            g_thread_flag.lock_and_set_condition_variable(lockable, condition_variable);
            m_owns = true;
        }

        ~thread_lock_t()
        {
            if (m_owns) {
                unlock();
            }
        }

        void lock()
        {
            g_thread_flag.lock(m_lockable);
            m_owns = true;
        }

        void unlock()
        {
            g_thread_flag.unlock(m_lockable);
            m_owns = false;
        }

    private:
        std::atomic<bool> m_owns;
        lockable_t& m_lockable;
    };
}

#endif
