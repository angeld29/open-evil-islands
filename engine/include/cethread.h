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

#ifndef CE_THREAD_H
#define CE_THREAD_H

#include <stddef.h>
#include <stdbool.h>

#include "cevector.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int ce_thread_online_cpu_count(void);

/*
 *  The thread struct provides platform-independent threads.
*/

typedef struct ce_thread ce_thread;

extern ce_thread* ce_thread_new(void (*func)(), void* arg);
extern void ce_thread_del(ce_thread* thread);

extern void ce_thread_wait(ce_thread* thread);

/*
 *  The mutex struct provides access serialization between threads.
*/

typedef struct ce_thread_mutex ce_thread_mutex;

extern ce_thread_mutex* ce_thread_mutex_new(void);
extern void ce_thread_mutex_del(ce_thread_mutex* mutex);

extern void ce_thread_mutex_lock(ce_thread_mutex* mutex);
extern void ce_thread_mutex_unlock(ce_thread_mutex* mutex);

/*
 *  The waitcond struct provides a condition variable for synchronizing threads.
*/

typedef struct ce_thread_cond ce_thread_cond;

extern ce_thread_cond* ce_thread_cond_new(void);
extern void ce_thread_cond_del(ce_thread_cond* cond);

extern void ce_thread_cond_wake_one(ce_thread_cond* cond);
extern void ce_thread_cond_wake_all(ce_thread_cond* cond);
extern void ce_thread_cond_wait(ce_thread_cond* cond, ce_thread_mutex* mutex);

/*
 *  The once struct provides an once-only initialization.
*/

typedef struct ce_thread_once ce_thread_once;

extern ce_thread_once* ce_thread_once_new(void);
extern void ce_thread_once_del(ce_thread_once* once);

extern void ce_thread_once_exec(ce_thread_once* once, void (*func)(), void* arg);

/*
 *  The semaphore struct provides a general counting semaphore.
*/

typedef struct {
	size_t available;
	ce_thread_mutex* mutex;
	ce_thread_cond* cond;
} ce_semaphore;

extern ce_semaphore* ce_semaphore_new(size_t n);
extern void ce_semaphore_del(ce_semaphore* semaphore);

extern size_t ce_semaphore_available(const ce_semaphore* semaphore);

extern void ce_semaphore_acquire(ce_semaphore* semaphore, size_t n);
extern void ce_semaphore_release(ce_semaphore* semaphore, size_t n);

extern bool ce_semaphore_try_acquire(ce_semaphore* semaphore, size_t n);

/*
 *  The threadpool struct manages a collection of threads.
 *  It's a thread pool pattern implementation.
*/

typedef struct {
	bool done;
	size_t idle_thread_count;
	ce_vector* threads;
	ce_vector* queue;
	ce_vector* cache;
	ce_thread_mutex* mutex;
	ce_thread_cond* thread_cond;
	ce_thread_cond* wait_one_cond;
	ce_thread_cond* wait_all_cond;
} ce_threadpool;

extern ce_threadpool* ce_threadpool_new(size_t thread_count);
extern void ce_threadpool_del(ce_threadpool* threadpool);

extern void ce_threadpool_enqueue(ce_threadpool* threadpool, void (*func)(), void* arg);

extern void ce_threadpool_wait_one(ce_threadpool* threadpool);
extern void ce_threadpool_wait_all(ce_threadpool* threadpool);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_THREAD_H */
