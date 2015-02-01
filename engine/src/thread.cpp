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
#include "event.hpp"
#include "thread.hpp"

namespace cursedearth
{
thread_pool_t* thread_pool;

void ce_thread_exec(thread_t* thread)
{
    ce_event_manager_process_events(thread->id, CE_EVENT_FLAG_WAIT_FOR_MORE_EVENTS);
}

void ce_thread_exit(thread_t* thread)
{
    ce_event_manager_interrupt(thread->id);
}

semaphore_t* ce_semaphore_new(size_t n)
{
    semaphore_t* semaphore = ce_alloc_zero(sizeof(semaphore_t));
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

static void ce_thread_pool_exec(thread_pool_t* thread_pool)
{
    ce_mutex_lock(thread_pool->mutex);

    while (!thread_pool->done) {
        if (ce_vector_empty(thread_pool->pending_routines)) {
            if (thread_pool->idle_thread_count == thread_pool->threads->count) {
                ce_wait_condition_wake_all(thread_pool->wait_all);
            }
            ce_wait_condition_wait(thread_pool->idle, thread_pool->mutex);
        } else {
            routine_t* routine = ce_vector_pop_back(thread_pool->pending_routines);
            ce_vector_push_back(thread_pool->free_routines, routine);

            void (*proc)(void*) = routine->func;
            void* arg = routine->arg;

            --thread_pool->idle_thread_count;
            ce_mutex_unlock(thread_pool->mutex);

            (*proc)(arg);

            ce_mutex_lock(thread_pool->mutex);
            ++thread_pool->idle_thread_count;

            ce_wait_condition_wake_all(thread_pool->wait_one);
        }
    }

    ce_mutex_unlock(thread_pool->mutex);
}

void ce_thread_pool_init(size_t thread_count)
{
    thread_pool = ce_alloc_zero(sizeof(thread_pool_t));
    thread_pool->idle_thread_count = thread_count;
    thread_pool->threads = ce_vector_new_reserved(thread_count);
    thread_pool->pending_routines = ce_vector_new();
    thread_pool->free_routines = ce_vector_new();
    thread_pool->mutex = ce_mutex_new();
    thread_pool->idle = ce_wait_condition_new();
    thread_pool->wait_one = ce_wait_condition_new();
    thread_pool->wait_all = ce_wait_condition_new();

    for (size_t i = 0; i < thread_count; ++i) {
        ce_vector_push_back(thread_pool->threads,
            ce_thread_new(ce_thread_pool_exec, thread_pool));
    }
}

void ce_thread_pool_term(void)
{
    if (NULL != thread_pool) {
        ce_mutex_lock(thread_pool->mutex);
        thread_pool->done = true;
        ce_mutex_unlock(thread_pool->mutex);

        ce_wait_condition_wake_all(thread_pool->idle);
        ce_vector_for_each(thread_pool->threads, ce_thread_wait);

        ce_wait_condition_del(thread_pool->wait_all);
        ce_wait_condition_del(thread_pool->wait_one);
        ce_wait_condition_del(thread_pool->idle);
        ce_mutex_del(thread_pool->mutex);

        if (!ce_vector_empty(thread_pool->pending_routines)) {
            ce_logging_warning("thread pool: pool is being "
                                "destroyed while queue is not empty");
        }

        ce_vector_for_each(thread_pool->free_routines, ce_routine_del);
        ce_vector_for_each(thread_pool->pending_routines, ce_routine_del);
        ce_vector_for_each(thread_pool->threads, ce_thread_del);

        ce_vector_del(thread_pool->free_routines);
        ce_vector_del(thread_pool->pending_routines);
        ce_vector_del(thread_pool->threads);

        ce_free(thread_pool, sizeof(thread_pool_t));
    }
}

void ce_thread_pool_enqueue(void (*proc)(), void* arg)
{
    ce_mutex_lock(thread_pool->mutex);
    routine_t* routine = ce_vector_empty(thread_pool->free_routines) ?
        ce_routine_new() : ce_vector_pop_back(thread_pool->free_routines);
    routine->func = proc;
    routine->arg = arg;
    ce_vector_push_back(thread_pool->pending_routines, routine);
    ce_wait_condition_wake_one(thread_pool->idle);
    ce_mutex_unlock(thread_pool->mutex);
}

static void ce_thread_pool_wait(wait_condition_t* wait_condition)
{
    ce_mutex_lock(thread_pool->mutex);
    if (!ce_vector_empty(thread_pool->pending_routines) ||
            thread_pool->idle_thread_count != thread_pool->threads->count) {
        ce_wait_condition_wait(wait_condition, thread_pool->mutex);
    }
    ce_mutex_unlock(thread_pool->mutex);
}

void ce_thread_pool_wait_one(void)
{
    ce_thread_pool_wait(thread_pool->wait_one);
}

void ce_thread_pool_wait_all(void)
{
    ce_thread_pool_wait(thread_pool->wait_all);
}
}
