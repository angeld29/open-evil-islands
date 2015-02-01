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

#include <windows.h>

#include "lib.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "vector.hpp"
#include "error_windows.hpp"
#include "thread.hpp"

namespace cursedearth
{
int online_cpu_count(void)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return ce_max(int, 1, info.dwNumberOfProcessors);
}

void sleep(unsigned int msec)
{
    Sleep(msec);
}

thread_id_t ce_thread_self(void)
{
    return GetCurrentThreadId();
}

struct thread_t
{
    thread_id_t id;
    routine_t routine;
    HANDLE handle;
};

static DWORD WINAPI ce_thread_wrap(LPVOID arg)
{
    routine_t* routine = arg;
    (*routine->func)(routine->arg);
    return 0;
}

thread_t* ce_thread_new(void (*proc)(void*), void* arg)
{
    thread_t* thread = ce_alloc_zero(sizeof(thread_t) + sizeof(ce_thread_windows));
    ce_thread_windows* windows_thread = (ce_thread_windows*)thread->impl;

    thread->routine.func = proc;
    thread->routine.arg = arg;

    DWORD id;
    windows_thread->handle = CreateThread(NULL, // default security attributes
                                        0,      // default stack size
                                        ce_thread_wrap,
                                        &thread->routine,
                                        0,      // default creation flags
                                        &id);   // thread identifier

    if (NULL == windows_thread->handle) {
        ce_error_report_windows_last("thread");
    }

    thread->id = id;

    return thread;
}

void ce_thread_del(thread_t* thread)
{
    if (NULL != thread) {
        ce_thread_windows* windows_thread = (ce_thread_windows*)thread->impl;
        CloseHandle(windows_thread->handle);
        ce_free(thread, sizeof(thread_t) + sizeof(ce_thread_windows));
    }
}

void ce_thread_wait(thread_t* thread)
{
    ce_thread_windows* windows_thread = (ce_thread_windows*)thread->impl;
    if (WAIT_OBJECT_0 != WaitForSingleObject(windows_thread->handle, INFINITE)) {
        ce_error_report_windows_last("thread");
    }
}

struct mutex_t {
    CRITICAL_SECTION handle;
};

mutex_t* ce_mutex_new(void)
{
    mutex_t* mutex = ce_alloc_zero(sizeof(mutex_t));
    InitializeCriticalSection(&mutex->handle);
    return mutex;
}

void ce_mutex_del(mutex_t* mutex)
{
    if (NULL != mutex) {
        DeleteCriticalSection(&mutex->handle);
        ce_free(mutex, sizeof(mutex_t));
    }
}

void ce_mutex_lock(mutex_t* mutex)
{
    EnterCriticalSection(&mutex->handle);
}

void ce_mutex_unlock(mutex_t* mutex)
{
    LeaveCriticalSection(&mutex->handle);
}

typedef struct {
    int priority;
    bool woken;
    HANDLE handle;
} ce_wait_condition_event;

static ce_wait_condition_event* ce_wait_condition_event_new(void)
{
    ce_wait_condition_event* event = ce_alloc_zero(sizeof(ce_wait_condition_event));
    event->handle = CreateEvent(NULL,  // default security attributes
                                TRUE,  // manual-reset event
                                FALSE, // initial state is nonsignaled
                                NULL); // unnamed
    if (NULL == event->handle) {
        ce_error_report_windows_last("wait condition event");
    }
    return event;
}

static void ce_wait_condition_event_del(ce_wait_condition_event* event)
{
    if (NULL != event) {
        CloseHandle(event->handle);
        ce_free(event, sizeof(ce_wait_condition_event));
    }
}

struct wait_condition_t {
    CRITICAL_SECTION mutex;
    ce_vector* events;
    ce_vector* cache;
};

wait_condition_t* ce_wait_condition_new(void)
{
    wait_condition_t* wait_condition = ce_alloc_zero(sizeof(wait_condition_t));
    InitializeCriticalSection(&wait_condition->mutex);
    wait_condition->events = ce_vector_new();
    wait_condition->cache = ce_vector_new();
    return wait_condition;
}

void ce_wait_condition_del(wait_condition_t* wait_condition)
{
    if (NULL != wait_condition) {
        if (!ce_vector_empty(wait_condition->events)) {
            ce_logging_warning("wait condition: destroyed while "
                                "threads are still waiting");
        }
        ce_vector_for_each(wait_condition->cache, ce_wait_condition_event_del);
        ce_vector_for_each(wait_condition->events, ce_wait_condition_event_del);
        ce_vector_del(wait_condition->cache);
        ce_vector_del(wait_condition->events);
        DeleteCriticalSection(&wait_condition->mutex);
        ce_free(wait_condition, sizeof(wait_condition_t));
    }
}

void ce_wait_condition_wake_one(wait_condition_t* wait_condition)
{
    EnterCriticalSection(&wait_condition->mutex);
    for (size_t i = 0; i < wait_condition->events->count; ++i) {
        ce_wait_condition_event* current = wait_condition->events->items[i];
        if (!current->woken) {
            current->woken = true;
            SetEvent(current->handle);
            break;
        }
    }
    LeaveCriticalSection(&wait_condition->mutex);
}

void ce_wait_condition_wake_all(wait_condition_t* wait_condition)
{
    EnterCriticalSection(&wait_condition->mutex);
    for (size_t i = 0; i < wait_condition->events->count; ++i) {
        ce_wait_condition_event* current = wait_condition->events->items[i];
        current->woken = true;
        SetEvent(current->handle);
    }
    LeaveCriticalSection(&wait_condition->mutex);
}

void ce_wait_condition_wait(wait_condition_t* wait_condition, mutex_t* mutex)
{
    EnterCriticalSection(&wait_condition->mutex);

    ce_wait_condition_event* event = ce_vector_empty(wait_condition->cache) ?
        ce_wait_condition_event_new() : ce_vector_pop_back(wait_condition->cache);
    event->priority = GetThreadPriority(GetCurrentThread());
    event->woken = false;

    // insert event into the queue (sorted by priority)
    size_t index = 0;
    for (; index < wait_condition->events->count; ++index) {
        ce_wait_condition_event* current = wait_condition->events->items[index];
        if (current->priority < event->priority) {
            break;
        }
    }
    ce_vector_insert(wait_condition->events, index, event);

    LeaveCriticalSection(&wait_condition->mutex);

    ce_mutex_unlock(mutex);
    if (WAIT_OBJECT_0 != WaitForSingleObject(event->handle, INFINITE)) {
        ce_error_report_windows_last("wait condition");
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

struct once_t {
    bool inited;
    CRITICAL_SECTION mutex;
};

once_t* ce_once_new(void)
{
    once_t* once = ce_alloc_zero(sizeof(once_t));
    InitializeCriticalSection(&once->mutex);
    return once;
}

void ce_once_del(once_t* once)
{
    if (NULL != once) {
        DeleteCriticalSection(&once->mutex);
        ce_free(once, sizeof(once_t));
    }
}

void ce_once_exec(once_t* once, void (*proc)(void*), void* arg)
{
    // double-checked locking
    // another solution ?
    if (!once->inited) {
        EnterCriticalSection(&once->mutex);
        if (!once->inited) {
            (*proc)(arg);
            once->inited = true;
        }
        LeaveCriticalSection(&once->mutex);
    }
}
}
