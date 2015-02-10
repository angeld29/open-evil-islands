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

#ifndef CE_THREAD_HPP
#define CE_THREAD_HPP

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

#include "singleton.hpp"

namespace cursedearth
{
    size_t online_cpu_count();

    typedef unsigned long int ce_thread_id;

    struct ce_thread;

    ce_thread_id ce_thread_self(void);

    ce_thread* ce_thread_new(void (*proc)(), void* arg);
    void ce_thread_del(ce_thread* thread);

    ce_thread_id ce_thread_get_id(ce_thread* thread);

    void ce_thread_wait(ce_thread* thread);

    typedef struct ce_mutex ce_mutex;

    ce_mutex* ce_mutex_new(void);
    void ce_mutex_del(ce_mutex* mutex);

    void ce_mutex_lock(ce_mutex* mutex);
    void ce_mutex_unlock(ce_mutex* mutex);

    typedef struct ce_wait_condition ce_wait_condition;

    ce_wait_condition* ce_wait_condition_new(void);
    void ce_wait_condition_del(ce_wait_condition* wait_condition);

    void ce_wait_condition_wake_one(ce_wait_condition* wait_condition);
    void ce_wait_condition_wake_all(ce_wait_condition* wait_condition);
    void ce_wait_condition_wait(ce_wait_condition* wait_condition, ce_mutex* mutex);

    /**
     * @brief The once struct provides an once-only initialization.
     */
    typedef struct ce_once ce_once;

    ce_once* ce_once_new(void);
    void ce_once_del(ce_once* once);

    void ce_once_exec(ce_once* once, void (*proc)(), void* arg);

    /**
     * @brief The semaphore class provides a general counting semaphore.
     */
    struct semaphore_t
    {
        size_t available;
        ce_mutex* mutex;
        ce_wait_condition* wait_condition;
    };

    semaphore_t* ce_semaphore_new(size_t n);
    void ce_semaphore_del(semaphore_t* semaphore);

    size_t ce_semaphore_available(const semaphore_t* semaphore);

    void ce_semaphore_acquire(semaphore_t* semaphore, size_t n);
    void ce_semaphore_release(semaphore_t* semaphore, size_t n);

    bool ce_semaphore_try_acquire(semaphore_t* semaphore, size_t n);

    /**
     * @brief The thread pool class manages a collection of threads.
     *        It's a thread pool pattern implementation.
     *        All functions are thread-safe.
     */
    class thread_pool_t: public singleton_t<thread_pool_t>
    {
        typedef std::function<void ()> task_t;

    public:
        thread_pool_t();
        ~thread_pool_t();

        void enqueue(const task_t&);

        void wait_one();
        void wait_all();

    private:
        void execute();

    private:
        size_t m_idle_thread_count;
        std::atomic<bool> m_done;
        std::mutex m_mutex;
        std::condition_variable m_idle;
        std::condition_variable m_wait_one;
        std::condition_variable m_wait_all;
        std::vector<task_t> m_tasks;
        std::vector<std::thread> m_threads;
    };
}

#endif
