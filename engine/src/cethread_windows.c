/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

/*
 *  Based on:
 *  1. MSDN website.
 *  2. Some ideas are from open source of the greatest Qt toolkit.
 *     Copyright (C) 2009 Nokia Corporation.
*/

#include <assert.h>

#include <windows.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cevector.h"
#include "cethread.h"

#include "ceerror_windows.h"

int ce_online_cpu_count(void)
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return ce_max(int, 1, info.dwNumberOfProcessors);
}

ce_thread_id ce_thread_self(void)
{
	return GetCurrentThreadId();
}

struct ce_thread {
	ce_routine routine;
	HANDLE handle;
};

static DWORD WINAPI ce_thread_wrap(LPVOID arg)
{
	ce_routine* routine = arg;
	(*routine->proc)(routine->arg);
	return 0;
}

ce_thread* ce_thread_new(void (*proc)(void*), void* arg)
{
	ce_thread* thread = ce_alloc(sizeof(ce_thread));
	thread->routine.proc = proc;
	thread->routine.arg = arg;
	thread->handle = CreateThread(NULL, // default security attributes
								0,      // default stack size
								ce_thread_wrap,
								&thread->routine,
								0,      // default creation flags
								NULL);  // no thread identifier
	if (NULL == thread->handle) {
		ce_error_report_windows_last("thread");
	}
	return thread;
}

void ce_thread_del(ce_thread* thread)
{
	if (NULL != thread) {
		CloseHandle(thread->handle);
		ce_free(thread, sizeof(ce_thread));
	}
}

void ce_thread_wait(ce_thread* thread)
{
	if (WAIT_OBJECT_0 != WaitForSingleObject(thread->handle, INFINITE)) {
		ce_error_report_windows_last("thread");
	}
}

struct ce_mutex {
	CRITICAL_SECTION handle;
};

ce_mutex* ce_mutex_new(void)
{
	ce_mutex* mutex = ce_alloc(sizeof(ce_mutex));
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

typedef struct {
	int priority;
	bool woken;
	HANDLE handle;
} ce_waitcond_event;

static ce_waitcond_event* ce_waitcond_event_new(void)
{
	ce_waitcond_event* event = ce_alloc_zero(sizeof(ce_waitcond_event));
	event->handle = CreateEvent(NULL,  // default security attributes
								TRUE,  // manual-reset event
								FALSE, // initial state is nonsignaled
								NULL); // unnamed
	if (NULL == event->handle) {
		ce_error_report_windows_last("waitcond");
	}
	return event;
}

static void ce_waitcond_event_del(ce_waitcond_event* event)
{
	if (NULL != event) {
		CloseHandle(event->handle);
		ce_free(event, sizeof(ce_waitcond_event));
	}
}

struct ce_waitcond {
	CRITICAL_SECTION mutex;
	ce_vector* events;
	ce_vector* cache;
};

ce_waitcond* ce_waitcond_new(void)
{
	ce_waitcond* waitcond = ce_alloc(sizeof(ce_waitcond));
	InitializeCriticalSection(&waitcond->mutex);
	waitcond->events = ce_vector_new();
	waitcond->cache = ce_vector_new();
	return waitcond;
}

void ce_waitcond_del(ce_waitcond* waitcond)
{
	if (NULL != waitcond) {
		if (!ce_vector_empty(waitcond->events)) {
			ce_logging_warning("waitcond: destroyed while "
								"threads are still waiting");
		}
		ce_vector_for_each(waitcond->cache, ce_waitcond_event_del);
		ce_vector_for_each(waitcond->events, ce_waitcond_event_del);
		ce_vector_del(waitcond->cache);
		ce_vector_del(waitcond->events);
		DeleteCriticalSection(&waitcond->mutex);
		ce_free(waitcond, sizeof(ce_waitcond));
	}
}

void ce_waitcond_wake_one(ce_waitcond* waitcond)
{
	EnterCriticalSection(&waitcond->mutex);
	for (size_t i = 0; i < waitcond->events->count; ++i) {
		ce_waitcond_event* current = waitcond->events->items[i];
		if (!current->woken) {
			current->woken = true;
			SetEvent(current->handle);
			break;
		}
	}
	LeaveCriticalSection(&waitcond->mutex);
}

void ce_waitcond_wake_all(ce_waitcond* waitcond)
{
	EnterCriticalSection(&waitcond->mutex);
	for (size_t i = 0; i < waitcond->events->count; ++i) {
		ce_waitcond_event* current = waitcond->events->items[i];
		current->woken = true;
		SetEvent(current->handle);
	}
	LeaveCriticalSection(&waitcond->mutex);
}

void ce_waitcond_wait(ce_waitcond* waitcond, ce_mutex* mutex)
{
	EnterCriticalSection(&waitcond->mutex);

	ce_waitcond_event* event = ce_vector_empty(waitcond->cache) ?
		ce_waitcond_event_new() : ce_vector_pop_back(waitcond->cache);
	event->priority = GetThreadPriority(GetCurrentThread());
	event->woken = false;

	// insert event into the queue (sorted by priority)
	size_t index = 0;
	for (; index < waitcond->events->count; ++index) {
		ce_waitcond_event* current = waitcond->events->items[index];
		if (current->priority < event->priority) {
			break;
		}
	}
	ce_vector_insert(waitcond->events, index, event);

	LeaveCriticalSection(&waitcond->mutex);

	ce_mutex_unlock(mutex);
	if (WAIT_OBJECT_0 != WaitForSingleObject(event->handle, INFINITE)) {
		ce_error_report_windows_last("waitcond");
	}
	ce_mutex_lock(mutex);

	EnterCriticalSection(&waitcond->mutex);

	// do not remove by index because
	// order of the items may be changed by another thread
	ce_vector_remove_all(waitcond->events, event);
	ce_vector_push_back(waitcond->cache, event);

	ResetEvent(event->handle);

	LeaveCriticalSection(&waitcond->mutex);
}

struct ce_once {
	bool inited;
	CRITICAL_SECTION mutex;
};

ce_once* ce_once_new(void)
{
	ce_once* once = ce_alloc_zero(sizeof(ce_once));
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

void ce_once_exec(ce_once* once, void (*proc)(void*), void* arg)
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
