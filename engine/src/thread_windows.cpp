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
#include <algorithm>

#include <windows.h>

#include "alloc.hpp"
#include "logging.hpp"
#include "vector.hpp"
#include "thread.hpp"

namespace cursedearth
{
    ce_thread_id ce_thread_self(void)
    {
        return static_cast<ce_thread_id>(GetCurrentThreadId());
    }

    struct routine_t
    {
        void (*proc)(void*);
        void* arg;
    };

    struct ce_thread
    {
        ce_thread_id id;
        routine_t routine;
        HANDLE handle;
    };

    DWORD WINAPI ce_thread_wrap(LPVOID arg)
    {
        routine_t* routine = (routine_t*)arg;
        (*routine->proc)(routine->arg);
        return 0;
    }

    ce_thread* ce_thread_new(void (*proc)(), void* arg)
    {
        ce_thread* thread = (ce_thread*)ce_alloc_zero(sizeof(ce_thread));
        thread->routine.proc = (void (*)(void*))proc;
        thread->routine.arg = arg;

        DWORD id;
        thread->handle = CreateThread(NULL, // default security attributes
                                      0,      // default stack size
                                      ce_thread_wrap,
                                      &thread->routine,
                                      0,      // default creation flags
                                      &id);   // thread identifier

        if (NULL == thread->handle) {
            ce_logging_error("thread: CreateThread failed");
        }

        thread->id = id;

        return thread;
    }

    void ce_thread_del(ce_thread* thread)
    {
        if (NULL != thread) {
            CloseHandle(thread->handle);
            ce_free(thread, sizeof(ce_thread));
        }
    }

    ce_thread_id ce_thread_get_id(ce_thread* thread)
    {
        return thread->id;
    }

    void ce_thread_wait(ce_thread* thread)
    {
        if (WAIT_OBJECT_0 != WaitForSingleObject(thread->handle, INFINITE)) {
            ce_logging_error("thread: WaitForSingleObject failed");
        }
    }

    struct ce_mutex {
        CRITICAL_SECTION handle;
    };

    ce_mutex* ce_mutex_new(void)
    {
        ce_mutex* mutex = (ce_mutex*)ce_alloc_zero(sizeof(ce_mutex));
        InitializeCriticalSection(&mutex->handle);
        return mutex;
    }

    void ce_mutex_del(ce_mutex* mutex)
    {
        if (NULL != mutex) {
            DeleteCriticalSection(&mutex->handle);
            ce_free(mutex, sizeof(ce_mutex));
        }
    }

    void ce_mutex_lock(ce_mutex* mutex)
    {
        EnterCriticalSection(&mutex->handle);
    }

    void ce_mutex_unlock(ce_mutex* mutex)
    {
        LeaveCriticalSection(&mutex->handle);
    }
}
