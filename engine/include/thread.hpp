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

#include <vector>

#include "atomic.hpp"

namespace cursedearth
{
    int online_cpu_count();
    void sleep(unsigned int ms);

    /**
     *  The thread struct provides platform-independent threads.
     */

    struct routine_t
    {
        void (*func)(void*);
        void* arg;
    };

    typedef unsigned long int thread_id_t;
    struct thread_t;

    thread_id_t ce_thread_self();

    thread_t* ce_thread_new(void (*proc)(), void* arg);
    void ce_thread_del(thread_t* thread);

    thread_id_t ce_thread_id(thread_t* thread);

    void ce_thread_wait(thread_t* thread);

    void ce_thread_exec(thread_t* thread);
    void ce_thread_exit(thread_t* thread);

    inline void ce_thread_exit_wait_del(thread_t* thread)
    {
        ce_thread_exit(thread);
        ce_thread_wait(thread);
        ce_thread_del(thread);
    }

    /**
     *  The mutex struct provides access serialization between threads.
     */

    struct mutex_t;

    mutex_t* ce_mutex_new(void);
    void ce_mutex_del(mutex_t* mutex);

    void ce_mutex_lock(mutex_t* mutex);
    void ce_mutex_unlock(mutex_t* mutex);

    /**
     *  The wait condition struct provides a condition variable for synchronizing threads.
     */

    struct wait_condition_t;

    wait_condition_t* ce_wait_condition_new(void);
    void ce_wait_condition_del(wait_condition_t* wait_condition);

    void ce_wait_condition_wake_one(wait_condition_t* wait_condition);
    void ce_wait_condition_wake_all(wait_condition_t* wait_condition);
    void ce_wait_condition_wait(wait_condition_t* wait_condition, mutex_t* mutex);

    /**
     *  The once struct provides an once-only initialization.
     */

    struct once_t;

    once_t* ce_once_new(void);
    void ce_once_del(once_t* once);

    void ce_once_exec(once_t* once, void (*proc)(), void* arg);

    /**
     *  The semaphore struct provides a general counting semaphore.
     */

    struct semaphore_t
    {
        size_t available;
        mutex_t* mutex;
        wait_condition_t* wait_condition;
    };

    semaphore_t* ce_semaphore_new(size_t n);
    void ce_semaphore_del(semaphore_t* semaphore);

    size_t ce_semaphore_available(const semaphore_t* semaphore);

    void ce_semaphore_acquire(semaphore_t* semaphore, size_t n);
    void ce_semaphore_release(semaphore_t* semaphore, size_t n);

    bool ce_semaphore_try_acquire(semaphore_t* semaphore, size_t n);

    /**
     *  The thread pool struct manages a collection of threads.
     *  It's a thread pool pattern implementation.
     *  All functions are thread-safe.
     */

    struct thread_pool_t
    {
        bool done;
        size_t idle_thread_count;
        std::vector<thread_t*> threads;
        std::vector<routine_t> pending_routines;
        std::vector<routine_t> free_routines;
        mutex_t* mutex;
        wait_condition_t* idle;
        wait_condition_t* wait_one;
        wait_condition_t* wait_all;
    }* thread_pool;

    void ce_thread_pool_init(size_t thread_count);
    void ce_thread_pool_term(void);

    void ce_thread_pool_enqueue(void (*proc)(), void* arg);

    void ce_thread_pool_wait_one(void);
    void ce_thread_pool_wait_all(void);
}

#endif /* CE_THREAD_HPP */
