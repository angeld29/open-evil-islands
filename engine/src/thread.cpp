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

#include "alloc.hpp"
#include "logging.hpp"
#include "thread.hpp"

namespace cursedearth
{
    semaphore_t* ce_semaphore_new(size_t n)
    {
        semaphore_t* semaphore = (semaphore_t*)ce_alloc_zero(sizeof(semaphore_t));
        semaphore->available = n;
        semaphore->mutex = ce_mutex_new();
        semaphore->wait_condition = ce_wait_condition_new();
        return semaphore;
    }

    void ce_semaphore_del(semaphore_t* semaphore)
    {
        if (NULL != semaphore) {
            ce_wait_condition_del(semaphore->wait_condition);
            ce_mutex_del(semaphore->mutex);
            ce_free(semaphore, sizeof(semaphore_t));
        }
    }

    size_t ce_semaphore_available(const semaphore_t* semaphore)
    {
        ce_mutex_lock(semaphore->mutex);
        size_t n = semaphore->available;
        ce_mutex_unlock(semaphore->mutex);
        return n;
    }

    void ce_semaphore_acquire(semaphore_t* semaphore, size_t n)
    {
        ce_mutex_lock(semaphore->mutex);
        while (n > semaphore->available) {
            ce_wait_condition_wait(semaphore->wait_condition, semaphore->mutex);
        }
        semaphore->available -= n;
        ce_mutex_unlock(semaphore->mutex);
    }

    void ce_semaphore_release(semaphore_t* semaphore, size_t n)
    {
        ce_mutex_lock(semaphore->mutex);
        semaphore->available += n;
        ce_wait_condition_wake_all(semaphore->wait_condition);
        ce_mutex_unlock(semaphore->mutex);
    }

    bool ce_semaphore_try_acquire(semaphore_t* semaphore, size_t n)
    {
        ce_mutex_lock(semaphore->mutex);
        bool result = true;
        if (n > semaphore->available) {
            result = false;
        } else {
            semaphore->available -= n;
        }
        ce_mutex_unlock(semaphore->mutex);
        return result;
    }

    thread_pool_t::thread_pool_t():
        singleton_t<thread_pool_t>(this),
        m_idle_thread_count(online_cpu_count()),
        m_done(false),
        m_threads(m_idle_thread_count)
    {
        for (auto& thread: m_threads) {
            thread = std::thread(std::bind(&thread_pool_t::execute, this));
        }
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
}
