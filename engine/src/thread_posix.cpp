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

#include <cstdio>
#include <cstring>
#include <cassert>

#include <unistd.h>
#include <pthread.h>

#ifndef _SC_NPROCESSORS_ONLN
#ifdef _SC_NPROC_ONLN
#define _SC_NPROCESSORS_ONLN _SC_NPROC_ONLN
#elif defined _SC_CRAY_NCPU
#define _SC_NPROCESSORS_ONLN _SC_CRAY_NCPU
#endif
#endif

#include "lib.hpp"
#include "alloc.hpp"
#include "error.hpp"
#include "thread.hpp"

int ce_online_cpu_count(void)
{
#ifdef _SC_NPROCESSORS_ONLN
    return ce_max(int, 1, sysconf(_SC_NPROCESSORS_ONLN));
#else
    return 1;
#endif
}

void ce_sleep(unsigned int msec)
{
    usleep(1000 * msec);
}

ce_thread_id ce_thread_self(void)
{
    return pthread_self();
}

typedef struct {
    pthread_t handle;
} ce_thread_posix;

static void* ce_thread_wrap(void* arg)
{
    ce_routine* routine = arg;
    (*routine->proc)(routine->arg);
    return arg;
}

ce_thread* ce_thread_new(void (*proc)(void*), void* arg)
{
    ce_thread* thread = ce_alloc_zero(sizeof(ce_thread) + sizeof(ce_thread_posix));
    ce_thread_posix* posix_thread = (ce_thread_posix*)thread->impl;

    thread->routine.proc = proc;
    thread->routine.arg = arg;

    int code = pthread_create(&posix_thread->handle, NULL, ce_thread_wrap, &thread->routine);
    if (0 != code) {
        ce_error_report_c_errno(code, "thread");
    }

    thread->id = posix_thread->handle;

    return thread;
}

void ce_thread_del(ce_thread* thread)
{
    ce_free(thread, sizeof(ce_thread) + sizeof(ce_thread_posix));
}

void ce_thread_wait(ce_thread* thread)
{
    ce_thread_posix* posix_thread = (ce_thread_posix*)thread->impl;
    int code = pthread_join(posix_thread->handle, NULL);
    if (0 != code) {
        ce_error_report_c_errno(code, "thread");
    }
}

struct ce_mutex {
    pthread_mutex_t handle;
};

ce_mutex* ce_mutex_new(void)
{
    ce_mutex* mutex = ce_alloc_zero(sizeof(ce_mutex));
    pthread_mutex_init(&mutex->handle, NULL);
    return mutex;
}

void ce_mutex_del(ce_mutex* mutex)
{
    if (NULL != mutex) {
        pthread_mutex_destroy(&mutex->handle);
        ce_free(mutex, sizeof(ce_mutex));
    }
}

void ce_mutex_lock(ce_mutex* mutex)
{
    pthread_mutex_lock(&mutex->handle);
}

void ce_mutex_unlock(ce_mutex* mutex)
{
    pthread_mutex_unlock(&mutex->handle);
}

struct ce_wait_condition {
    int waiter_count;
    int wakeup_count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

ce_wait_condition* ce_wait_condition_new(void)
{
    ce_wait_condition* wait_condition = ce_alloc_zero(sizeof(ce_wait_condition));
    wait_condition->waiter_count = 0;
    wait_condition->wakeup_count = 0;
    pthread_mutex_init(&wait_condition->mutex, NULL);
    pthread_cond_init(&wait_condition->cond, NULL);
    return wait_condition;
}

void ce_wait_condition_del(ce_wait_condition* wait_condition)
{
    if (NULL != wait_condition) {
        pthread_cond_destroy(&wait_condition->cond);
        pthread_mutex_destroy(&wait_condition->mutex);
        ce_free(wait_condition, sizeof(ce_wait_condition));
    }
}

void ce_wait_condition_wake_one(ce_wait_condition* wait_condition)
{
    pthread_mutex_lock(&wait_condition->mutex);
    wait_condition->wakeup_count = ce_min(int, wait_condition->wakeup_count + 1,
                                                wait_condition->waiter_count);
    pthread_cond_signal(&wait_condition->cond);
    pthread_mutex_unlock(&wait_condition->mutex);
}

void ce_wait_condition_wake_all(ce_wait_condition* wait_condition)
{
    pthread_mutex_lock(&wait_condition->mutex);
    wait_condition->wakeup_count = wait_condition->waiter_count;
    pthread_cond_broadcast(&wait_condition->cond);
    pthread_mutex_unlock(&wait_condition->mutex);
}

void ce_wait_condition_wait(ce_wait_condition* wait_condition, ce_mutex* mutex)
{
    pthread_mutex_lock(&wait_condition->mutex);
    ++wait_condition->waiter_count;

    ce_mutex_unlock(mutex);

    int code;
    do {
        code = pthread_cond_wait(&wait_condition->cond, &wait_condition->mutex);
        // many vendors warn of spurious wakeups from pthread_cond_wait,
        // even though POSIX doesn't allow for it...
    } while (0 == code && 0 == wait_condition->wakeup_count);

    assert(wait_condition->waiter_count > 0 && "internal error");
    --wait_condition->waiter_count;

    if (0 == code) {
        assert(wait_condition->wakeup_count > 0 && "internal error");
        --wait_condition->wakeup_count;
    } else {
        ce_error_report_c_errno(code, "wait condition");
    }

    pthread_mutex_unlock(&wait_condition->mutex);

    ce_mutex_lock(mutex);
}

struct ce_once {
    pthread_once_t handle;
};

ce_once* ce_once_new(void)
{
    ce_once* once = ce_alloc_zero(sizeof(ce_once));
    once->handle = PTHREAD_ONCE_INIT;
    return once;
}

void ce_once_del(ce_once* once)
{
    ce_free(once, sizeof(ce_once));
}

static pthread_once_t ce_once_once = PTHREAD_ONCE_INIT;
static pthread_key_t ce_once_key;

static void ce_once_key_init()
{
    pthread_key_create(&ce_once_key, NULL);
}

static void ce_once_wrap(void)
{
    ce_routine* routine = pthread_getspecific(ce_once_key);
    (*routine->proc)(routine->arg);
}

void ce_once_exec(ce_once* once, void (*proc)(void*), void* arg)
{
    pthread_once(&ce_once_once, ce_once_key_init);
    ce_routine routine = {proc, arg};
    pthread_setspecific(ce_once_key, &routine);
    pthread_once(&once->handle, ce_once_wrap);
}
