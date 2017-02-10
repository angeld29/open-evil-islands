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

#include "thread.hpp"
#include "threadlock.hpp"

namespace cursedearth
{
    thread_local thread_flag_t g_thread_flag;

    thread_local std::atomic<bool> thread_lock_t::s_nested_guard;
    thread_local std::thread::id thread_lock_t::s_thread_id;

    void interruption_point()
    {
        if (g_thread_flag) {
            throw thread_interrupted_t();
        }
    }

    void thread_t::do_interrupt(thread_flag_t* flag, bool value)
    {
        if (m_flag) {
            *m_flag = value;
        }
        m_flag = flag;
        if (m_flag) {
            *m_flag = value;
        }
    }
}
