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

#ifndef CE_THREAD_H
#define CE_THREAD_H

#include <stddef.h>
#include <stdbool.h>

#include "cevector.h"
#include "ceatomic.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int ce_online_cpu_count(void);
extern void ce_sleep(unsigned int msec);

/*
 *  Useful struct for other thread and non-thread modules.
*/

typedef struct {
	void (*proc)(void*);
	void* arg;
} ce_routine;

extern ce_routine* ce_routine_new(void);
extern void ce_routine_del(ce_routine* routine);

/*
 *  The thread struct provides platform-independent threads.
*/

typedef unsigned long int ce_thread_id;

typedef struct {
	ce_thread_id id;
	ce_routine routine;
	char impl[];
} ce_thread;

extern ce_thread_id ce_thread_self(void);

extern ce_thread* ce_thread_new(void (*proc)(), void* arg);
extern void ce_thread_del(ce_thread* thread);

extern void ce_thread_wait(ce_thread* thread);

extern void ce_thread_exec(ce_thread* thread);
extern void ce_thread_exit(ce_thread* thread);

static inline void ce_thread_exit_wait_del(ce_thread* thread)
{
	ce_thread_exit(thread);
	ce_thread_wait(thread);
	ce_thread_del(thread);
}

/*
 *  The mutex struct provides access serialization between threads.
*/

typedef struct ce_mutex ce_mutex;

extern ce_mutex* ce_mutex_new(void);
extern void ce_mutex_del(ce_mutex* mutex);

extern void ce_mutex_lock(ce_mutex* mutex);
extern void ce_mutex_unlock(ce_mutex* mutex);

/*
 *  The wait condition struct provides a condition variable for synchronizing threads.
*/

typedef struct ce_wait_condition ce_wait_condition;

extern ce_wait_condition* ce_wait_condition_new(void);
extern void ce_wait_condition_del(ce_wait_condition* wait_condition);

extern void ce_wait_condition_wake_one(ce_wait_condition* wait_condition);
extern void ce_wait_condition_wake_all(ce_wait_condition* wait_condition);
extern void ce_wait_condition_wait(ce_wait_condition* wait_condition, ce_mutex* mutex);

/*
 *  The once struct provides an once-only initialization.
*/

typedef struct ce_once ce_once;

extern ce_once* ce_once_new(void);
extern void ce_once_del(ce_once* once);

extern void ce_once_exec(ce_once* once, void (*proc)(), void* arg);

/*
 *  The semaphore struct provides a general counting semaphore.
*/

typedef struct {
	size_t available;
	ce_mutex* mutex;
	ce_wait_condition* wait_condition;
} ce_semaphore;

extern ce_semaphore* ce_semaphore_new(size_t n);
extern void ce_semaphore_del(ce_semaphore* semaphore);

extern size_t ce_semaphore_available(const ce_semaphore* semaphore);

extern void ce_semaphore_acquire(ce_semaphore* semaphore, size_t n);
extern void ce_semaphore_release(ce_semaphore* semaphore, size_t n);

extern bool ce_semaphore_try_acquire(ce_semaphore* semaphore, size_t n);

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

extern void ce_thread_pool_init(size_t thread_count);
extern void ce_thread_pool_term(void);

extern void ce_thread_pool_enqueue(void (*proc)(), void* arg);

extern void ce_thread_pool_wait_one(void);
extern void ce_thread_pool_wait_all(void);

#ifdef __cplusplus
}
#endif

#endif /* CE_THREAD_H */
