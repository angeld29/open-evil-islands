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

#include "vector.hpp"

namespace cursedearth
{
    size_t online_cpu_count();
    void sleep(unsigned int milliseconds);

    typedef struct {
        void (*proc)(void*);
        void* arg;
    } ce_routine;

    ce_routine* ce_routine_new(void);
    void ce_routine_del(ce_routine* routine);

    /*
     *  The thread struct provides platform-independent threads.
    */

    typedef unsigned long int ce_thread_id;

    struct ce_thread;

    ce_thread_id ce_thread_self(void);

    ce_thread* ce_thread_new(void (*proc)(), void* arg);
    void ce_thread_del(ce_thread* thread);

    ce_thread_id ce_thread_get_id(ce_thread* thread);

    void ce_thread_wait(ce_thread* thread);

    void ce_thread_exec(ce_thread* thread);
    void ce_thread_exit(ce_thread* thread);

    inline void ce_thread_exit_wait_del(ce_thread* thread)
    {
        ce_thread_exit(thread);
        ce_thread_wait(thread);
        ce_thread_del(thread);
    }

    /*
     *  The mutex struct provides access serialization between threads.
    */

    typedef struct ce_mutex ce_mutex;

    ce_mutex* ce_mutex_new(void);
    void ce_mutex_del(ce_mutex* mutex);

    void ce_mutex_lock(ce_mutex* mutex);
    void ce_mutex_unlock(ce_mutex* mutex);

    /*
     *  The wait condition struct provides a condition variable for synchronizing threads.
    */

    typedef struct ce_wait_condition ce_wait_condition;

    ce_wait_condition* ce_wait_condition_new(void);
    void ce_wait_condition_del(ce_wait_condition* wait_condition);

    void ce_wait_condition_wake_one(ce_wait_condition* wait_condition);
    void ce_wait_condition_wake_all(ce_wait_condition* wait_condition);
    void ce_wait_condition_wait(ce_wait_condition* wait_condition, ce_mutex* mutex);

    /*
     *  The once struct provides an once-only initialization.
    */

    typedef struct ce_once ce_once;

    ce_once* ce_once_new(void);
    void ce_once_del(ce_once* once);

    void ce_once_exec(ce_once* once, void (*proc)(), void* arg);

    /*
     *  The semaphore struct provides a general counting semaphore.
    */

    typedef struct {
        size_t available;
        ce_mutex* mutex;
        ce_wait_condition* wait_condition;
    } ce_semaphore;

    ce_semaphore* ce_semaphore_new(size_t n);
    void ce_semaphore_del(ce_semaphore* semaphore);

    size_t ce_semaphore_available(const ce_semaphore* semaphore);

    void ce_semaphore_acquire(ce_semaphore* semaphore, size_t n);
    void ce_semaphore_release(ce_semaphore* semaphore, size_t n);

    bool ce_semaphore_try_acquire(ce_semaphore* semaphore, size_t n);

    /*
     *  The thread pool struct manages a collection of threads.
     *  It's a thread pool pattern implementation.
     *  All functions are thread-safe.
    */

    extern struct ce_thread_pool {
        bool done;
        size_t idle_thread_count;
        ce_vector* threads;
        ce_vector* pending_routines;
        ce_vector* free_routines;
        ce_mutex* mutex;
        ce_wait_condition* idle;
        ce_wait_condition* wait_one;
        ce_wait_condition* wait_all;
    }* ce_thread_pool;

    void ce_thread_pool_init();
    void ce_thread_pool_term();

    void ce_thread_pool_enqueue(void (*proc)(), void* arg);

    void ce_thread_pool_wait_one();
    void ce_thread_pool_wait_all();
}

#endif
