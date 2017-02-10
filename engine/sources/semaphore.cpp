/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include "semaphore.hpp"
#include "threadlock.hpp"

namespace cursedearth
{
    semaphore_t::semaphore_t(size_t n):
        m_available(n),
        m_condition_variable(make_condition_variable())
    {
    }

    void semaphore_t::acquire(size_t n)
    {
        interruption_point();
        thread_lock_t lock(m_mutex, m_condition_variable);
        while (n > m_available) {
            m_condition_variable->wait(lock);
        }
        m_available -= n;
    }

    void semaphore_t::release(size_t n)
    {
        interruption_point();
        m_available += n;
        std::lock_guard<std::mutex> lock(m_mutex);
        std::ignore = lock;
        m_condition_variable->notify_all();
    }

    bool semaphore_t::try_acquire(size_t n)
    {
        interruption_point();
        if (n > m_available) {
            return false;
        }
        m_available -= n;
        return true;
    }

    semaphore_ptr_t make_semaphore(size_t n)
    {
        return std::make_shared<semaphore_t>(n);
    }
}
