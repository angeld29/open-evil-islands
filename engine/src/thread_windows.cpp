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
    size_t online_cpu_count()
    {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return std::max<size_t>(1, info.dwNumberOfProcessors);
    }

    void sleep(unsigned int milliseconds)
    {
        Sleep(milliseconds);
    }

    ce_thread_id ce_thread_self(void)
    {
        return static_cast<ce_thread_id>(GetCurrentThreadId());
    }

    struct ce_thread
    {
        ce_thread_id id;
        ce_routine routine;
        HANDLE handle;
    };

    DWORD WINAPI ce_thread_wrap(LPVOID arg)
    {
        ce_routine* routine = (ce_routine*)arg;
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

    struct ce_wait_condition_event
    {
        int priority;
        bool woken;
        HANDLE handle;
    };

    ce_wait_condition_event* ce_wait_condition_event_new(void)
    {
        ce_wait_condition_event* event = (ce_wait_condition_event*)ce_alloc_zero(sizeof(ce_wait_condition_event));
        event->handle = CreateEvent(NULL,  // default security attributes
                                    TRUE,  // manual-reset event
                                    FALSE, // initial state is nonsignaled
                                    NULL); // unnamed
        if (NULL == event->handle) {
            ce_logging_error("thread: CreateEvent failed");
        }
        return event;
    }

    void ce_wait_condition_event_del(ce_wait_condition_event* event)
    {
        if (NULL != event) {
            CloseHandle(event->handle);
            ce_free(event, sizeof(ce_wait_condition_event));
        }
    }

    struct ce_wait_condition
    {
        CRITICAL_SECTION mutex;
        ce_vector* events;
        ce_vector* cache;
    };

    ce_wait_condition* ce_wait_condition_new(void)
    {
        ce_wait_condition* wait_condition = (ce_wait_condition*)ce_alloc_zero(sizeof(ce_wait_condition));
        InitializeCriticalSection(&wait_condition->mutex);
        wait_condition->events = ce_vector_new();
        wait_condition->cache = ce_vector_new();
        return wait_condition;
    }

    void ce_wait_condition_del(ce_wait_condition* wait_condition)
    {
        if (NULL != wait_condition) {
            if (!ce_vector_empty(wait_condition->events)) {
                ce_logging_warning("wait condition: destroyed while threads are still waiting");
            }
            ce_vector_for_each(wait_condition->cache, (void(*)(void*))ce_wait_condition_event_del);
            ce_vector_for_each(wait_condition->events, (void(*)(void*))ce_wait_condition_event_del);
            ce_vector_del(wait_condition->cache);
            ce_vector_del(wait_condition->events);
            DeleteCriticalSection(&wait_condition->mutex);
            ce_free(wait_condition, sizeof(ce_wait_condition));
        }
    }

    void ce_wait_condition_wake_one(ce_wait_condition* wait_condition)
    {
        EnterCriticalSection(&wait_condition->mutex);
        for (size_t i = 0; i < wait_condition->events->count; ++i) {
            ce_wait_condition_event* current = (ce_wait_condition_event*)wait_condition->events->items[i];
            if (!current->woken) {
                current->woken = true;
                SetEvent(current->handle);
                break;
            }
        }
        LeaveCriticalSection(&wait_condition->mutex);
    }

    void ce_wait_condition_wake_all(ce_wait_condition* wait_condition)
    {
        EnterCriticalSection(&wait_condition->mutex);
        for (size_t i = 0; i < wait_condition->events->count; ++i) {
            ce_wait_condition_event* current = (ce_wait_condition_event*)wait_condition->events->items[i];
            current->woken = true;
            SetEvent(current->handle);
        }
        LeaveCriticalSection(&wait_condition->mutex);
    }

    void ce_wait_condition_wait(ce_wait_condition* wait_condition, ce_mutex* mutex)
    {
        EnterCriticalSection(&wait_condition->mutex);

        ce_wait_condition_event* event = ce_vector_empty(wait_condition->cache) ?
            ce_wait_condition_event_new() : (ce_wait_condition_event*)ce_vector_pop_back(wait_condition->cache);
        event->priority = GetThreadPriority(GetCurrentThread());
        event->woken = false;

        // insert event into the queue (sorted by priority)
        size_t index = 0;
        for (; index < wait_condition->events->count; ++index) {
            ce_wait_condition_event* current = (ce_wait_condition_event*)wait_condition->events->items[index];
            if (current->priority < event->priority) {
                break;
            }
        }
        ce_vector_insert(wait_condition->events, index, event);

        LeaveCriticalSection(&wait_condition->mutex);

        ce_mutex_unlock(mutex);
        if (WAIT_OBJECT_0 != WaitForSingleObject(event->handle, INFINITE)) {
            ce_logging_error("thread: WaitForSingleObject failed");
        }
        ce_mutex_lock(mutex);

        EnterCriticalSection(&wait_condition->mutex);

        // do not remove by index because
        // order of the items may be changed by another thread
        ce_vector_remove_all(wait_condition->events, event);
        ce_vector_push_back(wait_condition->cache, event);

        ResetEvent(event->handle);

        LeaveCriticalSection(&wait_condition->mutex);
    }

    struct ce_once
    {
        bool inited;
        CRITICAL_SECTION mutex;
    };

    ce_once* ce_once_new(void)
    {
        ce_once* once = (ce_once*)ce_alloc_zero(sizeof(ce_once));
        InitializeCriticalSection(&once->mutex);
        return once;
    }

    void ce_once_del(ce_once* once)
    {
        if (NULL != once) {
            DeleteCriticalSection(&once->mutex);
            ce_free(once, sizeof(ce_once));
        }
    }

    void ce_once_exec(ce_once* once, void (*proc)(), void* arg)
    {
        // double-checked locking
        // another solution ?
        if (!once->inited) {
            EnterCriticalSection(&once->mutex);
            if (!once->inited) {
                ((void (*)(void*))(*proc))(arg);
                once->inited = true;
            }
            LeaveCriticalSection(&once->mutex);
        }
    }
}
