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

#include "logging.hpp"
#include "threadpool.hpp"

namespace cursedearth
{
    thread_pool_t::thread_pool_t():
        singleton_t<thread_pool_t>(this),
        m_idle_thread_count(online_cpu_count()),
        m_done(false),
        m_threads(m_idle_thread_count)
    {
        for (auto& thread: m_threads) {
            thread = std::thread(std::bind(&thread_pool_t::execute, this));
        }
        // TODO: std::thread::hardware_concurrency()
        ce_logging_info("thread pool: using up to %u threads", m_threads.size());
    }

    thread_pool_t::~thread_pool_t()
    {
        m_done = true;
        m_idle.notify_all();

        for (auto& thread: m_threads) {
            thread.join();
        }

        if (!m_tasks.empty()) {
            ce_logging_warning("thread pool: pool is being destroyed while queue is not empty");
        }
    }

    void thread_pool_t::enqueue(const task_t& task)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_tasks.push_back(task);
        m_idle.notify_one();
    }

    void thread_pool_t::wait_one()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_tasks.empty() || m_idle_thread_count != m_threads.size()) {
            m_wait_one.wait(lock);
        }
    }

    void thread_pool_t::wait_all()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_tasks.empty() || m_idle_thread_count != m_threads.size()) {
            m_wait_all.wait(lock);
        }
    }

    void thread_pool_t::execute()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_done) {
            if (m_tasks.empty()) {
                if (m_idle_thread_count == m_threads.size()) {
                    m_wait_all.notify_all();
                }
                m_idle.wait(lock);
            } else {
                task_t task = m_tasks.back();
                m_tasks.pop_back();
                --m_idle_thread_count;

                lock.unlock();
                task();
                lock.lock();

                ++m_idle_thread_count;
                m_wait_one.notify_all();
            }
        }
    }

    thread_pool_ptr_t make_thread_pool()
    {
        return make_unique<thread_pool_t>();
    }
}
