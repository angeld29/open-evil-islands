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

#include "threadpool.hpp"

namespace cursedearth
{
    thread_pool_t::thread_pool_t():
        singleton_t<thread_pool_t>(this),
        m_idle_thread_count(std::max<size_t>(1, std::thread::hardware_concurrency())),
        m_threads(m_idle_thread_count)
    {
        for (auto& thread: m_threads) {
            thread = make_thread("thread pool", [this]{execute();});
        }
        ce_logging_info("thread pool: using up to %u threads", m_threads.size());
    }

    thread_pool_t::~thread_pool_t()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::ignore = lock;
        if (!m_tasks.empty()) {
            ce_logging_warning("thread pool: pool is being destroyed while queue is not empty");
        }
    }

    void thread_pool_t::enqueue(const task_t& task)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::ignore = lock;
        m_tasks.push_back(task);
        m_idle.notify_one();
    }

    void thread_pool_t::execute()
    {
        custom_lock<std::mutex> lock(m_idle, m_mutex);
        while (true) {
            //lock.unlock();
            //lock.lock();
            //throw game_error("thread pool", "break");
            if (m_tasks.empty()) {
                m_idle.wait(lock);
            } else {
                task_t task = m_tasks.back();
                m_tasks.pop_back();
                --m_idle_thread_count;
                lock.unlock();
                task();
                lock.lock();
                ++m_idle_thread_count;
            }
        }
    }

    thread_pool_ptr_t make_thread_pool()
    {
        return make_unique<thread_pool_t>();
    }
}
