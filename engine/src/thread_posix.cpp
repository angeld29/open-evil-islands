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

#include <cassert>
#include <cstdio>
#include <cstring>
#include <algorithm>

#include <unistd.h>
#include <pthread.h>

#include "alloc.hpp"
#include "thread.hpp"

namespace cursedearth
{
    ce_thread_id ce_thread_self(void)
    {
        return pthread_self();
    }

    struct routine_t
    {
        void (*proc)(void*);
        void* arg;
    };

    struct ce_thread
    {
        routine_t routine;
        pthread_t handle;
    };

    void* ce_thread_wrap(void* arg)
    {
        routine_t* routine = (routine_t*)arg;
        (*routine->proc)(routine->arg);
        return arg;
    }

    ce_thread* ce_thread_new(void (*proc)(), void* arg)
    {
        ce_thread* thread = (ce_thread*)ce_alloc_zero(sizeof(ce_thread));
        thread->routine.proc = (void(*)(void*))proc;
        thread->routine.arg = arg;

        int code = pthread_create(&thread->handle, NULL, ce_thread_wrap, &thread->routine);
        if (0 != code) {
            ce_logging_error("thread: pthread_create failed");
        }

        return thread;
    }

    void ce_thread_del(ce_thread* thread)
    {
        ce_free(thread, sizeof(ce_thread));
    }

    ce_thread_id ce_thread_get_id(ce_thread* thread)
    {
        return thread->handle;
    }

    void ce_thread_wait(ce_thread* thread)
    {
        int code = pthread_join(thread->handle, NULL);
        if (0 != code) {
            ce_logging_error("thread: pthread_join failed");
        }
    }

    struct ce_mutex {
        pthread_mutex_t handle;
    };

    ce_mutex* ce_mutex_new(void)
    {
        ce_mutex* mutex = (ce_mutex*)ce_alloc_zero(sizeof(ce_mutex));
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
}
