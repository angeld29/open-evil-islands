/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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
 *  Based on MSDN website.
 *
 *  Some ideas are from open source of the greatest Qt Toolkit.
 *  Copyright (C) 2009 Nokia Corporation.
*/

#include <assert.h>

#include <windows.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceerror.h"
#include "cevector.h"
#include "cethread.h"

int ce_thread_online_cpu_count(void)
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return ce_max(1, info.dwNumberOfProcessors);
}

typedef struct {
	void (*func)(void*);
	void* arg;
} ce_thread_cookie;

struct ce_thread {
	ce_thread_cookie cookie;
	HANDLE thread;
};

static DWORD WINAPI ce_thread_func_wrap(LPVOID arg)
{
	ce_thread_cookie* cookie = arg;
	(*cookie->func)(cookie->arg);
	return 0;
}

ce_thread* ce_thread_new(void (*func)(void*), void* arg)
{
	ce_thread* thread = ce_alloc(sizeof(ce_thread));
	thread->cookie.func = func;
	thread->cookie.arg = arg;
	thread->thread = CreateThread(NULL, // default security attributes
								0,      // default stack size
								ce_thread_func_wrap,
								&thread->cookie,
								0,      // default creation flags
								NULL);  // no thread identifier
	if (NULL == thread->thread) {
		ce_error_report_last_windows_error("thread", __func__,
											"CreateThread failed");
	}
	return thread;
}

void ce_thread_del(ce_thread* thread)
{
	if (NULL != thread) {
		CloseHandle(thread->thread);
		ce_free(thread, sizeof(ce_thread));
	}
}

void ce_thread_wait(ce_thread* thread)
{
	if (WAIT_OBJECT_0 != WaitForSingleObject(thread->thread, INFINITE)) {
		ce_error_report_last_windows_error("thread", __func__,
											"WaitForSingleObject failed");
	}
}

struct ce_thread_mutex {
	CRITICAL_SECTION cs;
};

ce_thread_mutex* ce_thread_mutex_new(void)
{
	ce_thread_mutex* mutex = ce_alloc(sizeof(ce_thread_mutex));
	InitializeCriticalSection(&mutex->cs);
	return mutex;
}

void ce_thread_mutex_del(ce_thread_mutex* mutex)
{
	if (NULL != mutex) {
		DeleteCriticalSection(&mutex->cs);
		ce_free(mutex, sizeof(ce_thread_mutex));
	}
}

void ce_thread_mutex_lock(ce_thread_mutex* mutex)
{
	EnterCriticalSection(&mutex->cs);
}

void ce_thread_mutex_unlock(ce_thread_mutex* mutex)
{
	LeaveCriticalSection(&mutex->cs);
}

typedef struct {
	int priority;
	bool woken;
	HANDLE event;
} ce_thread_cond_event;

static ce_thread_cond_event* ce_thread_cond_event_new(void)
{
	ce_thread_cond_event* event = ce_alloc(sizeof(ce_thread_cond_event));
	event->priority = 0;
	event->woken = false;
	event->event = CreateEvent(NULL,   // default security attributes
								TRUE,  // manual-reset event
								FALSE, // initial state is nonsignaled
								NULL); // unnamed
	if (NULL == event->event) {
		ce_error_report_last_windows_error("thread", __func__,
											"CreateEvent failed");
	}
	return event;
}

static void ce_thread_cond_event_del(ce_thread_cond_event* event)
{
	if (NULL != event) {
		CloseHandle(event->event);
		ce_free(event, sizeof(ce_thread_cond_event));
	}
}

struct ce_thread_cond {
	CRITICAL_SECTION cs;
	ce_vector* events;
	ce_vector* free_events;
};

ce_thread_cond* ce_thread_cond_new(void)
{
	ce_thread_cond* cond = ce_alloc(sizeof(ce_thread_cond));
	InitializeCriticalSection(&cond->cs);
	cond->events = ce_vector_new();
	cond->free_events = ce_vector_new();
	return cond;
}

void ce_thread_cond_del(ce_thread_cond* cond)
{
	if (NULL != cond) {
		if (!ce_vector_empty(cond->events)) {
			ce_logging_warning("thread: "
				"destroyed while threads are still waiting");
		}
		ce_vector_for_each(cond->free_events, ce_thread_cond_event_del);
		ce_vector_for_each(cond->events, ce_thread_cond_event_del);
		ce_vector_del(cond->free_events);
		ce_vector_del(cond->events);
		DeleteCriticalSection(&cond->cs);
		ce_free(cond, sizeof(ce_thread_cond));
	}
}

void ce_thread_cond_wake_one(ce_thread_cond* cond)
{
	EnterCriticalSection(&cond->cs);
	for (int i = 0; i < cond->events->count; ++i) {
		ce_thread_cond_event* current = cond->events->items[i];
		if (!current->woken) {
			current->woken = true;
			SetEvent(current->event);
			break;
		}
	}
	LeaveCriticalSection(&cond->cs);
}

void ce_thread_cond_wake_all(ce_thread_cond* cond)
{
	EnterCriticalSection(&cond->cs);
	for (int i = 0; i < cond->events->count; ++i) {
		ce_thread_cond_event* current = cond->events->items[i];
		current->woken = true;
		SetEvent(current->event);
	}
	LeaveCriticalSection(&cond->cs);
}

void ce_thread_cond_wait(ce_thread_cond* cond, ce_thread_mutex* mutex)
{
	EnterCriticalSection(&cond->cs);

	ce_thread_cond_event* event = ce_vector_empty(cond->free_events) ?
		ce_thread_cond_event_new() : ce_vector_pop_back(cond->free_events);
	event->priority = GetThreadPriority(GetCurrentThread());
	event->woken = false;

	// insert event into the queue (sorted by priority)
	int index = 0;
	for (; index < cond->events->count; ++index) {
		ce_thread_cond_event* current = cond->events->items[index];
		if (current->priority < event->priority) {
			break;
		}
	}
	ce_vector_insert(cond->events, index, event);

	LeaveCriticalSection(&cond->cs);

	ce_thread_mutex_unlock(mutex);
	if (WAIT_OBJECT_0 != WaitForSingleObject(event->event, INFINITE)) {
		ce_error_report_last_windows_error("thread", __func__,
											"WaitForSingleObject failed");
	}
	ce_thread_mutex_lock(mutex);

	EnterCriticalSection(&cond->cs);

	// do not remove by index because
	// order of the items may be changed by another thread
	ce_vector_remove_all(cond->events, event);
	ce_vector_push_back(cond->free_events, event);

	ResetEvent(event->event);

	LeaveCriticalSection(&cond->cs);
}

struct ce_thread_once {
	bool inited;
	CRITICAL_SECTION cs;
};

ce_thread_once* ce_thread_once_new(void)
{
	ce_thread_once* once = ce_alloc(sizeof(ce_thread_once));
	once->inited = false;
	InitializeCriticalSection(&once->cs);
	return once;
}

void ce_thread_once_del(ce_thread_once* once)
{
	if (NULL != once) {
		DeleteCriticalSection(&once->cs);
		ce_free(once, sizeof(ce_thread_once));
	}
}

void ce_thread_once_exec(ce_thread_once* once, void (*func)(void*), void* arg)
{
	// double-checked locking
	// another solution ?
	if (!once->inited) {
		EnterCriticalSection(&once->cs);
		if (!once->inited) {
			func(arg);
			once->inited = true;
		}
		LeaveCriticalSection(&once->cs);
	}
}
