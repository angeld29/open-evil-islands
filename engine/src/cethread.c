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

#include <assert.h>

#include "cealloc.h"
#include "celogging.h"
#include "cethread.h"

ce_semaphore* ce_semaphore_new(size_t n)
{
	ce_semaphore* semaphore = ce_alloc(sizeof(ce_semaphore));
	semaphore->available = n;
	semaphore->mutex = ce_thread_mutex_new();
	semaphore->cond = ce_thread_cond_new();
	return semaphore;
}

void ce_semaphore_del(ce_semaphore* semaphore)
{
	if (NULL != semaphore) {
		ce_thread_cond_del(semaphore->cond);
		ce_thread_mutex_del(semaphore->mutex);
		ce_free(semaphore, sizeof(ce_semaphore));
	}
}

size_t ce_semaphore_available(const ce_semaphore* semaphore)
{
	ce_thread_mutex_lock(semaphore->mutex);
	size_t n = semaphore->available;
	ce_thread_mutex_unlock(semaphore->mutex);
	return n;
}

void ce_semaphore_acquire(ce_semaphore* semaphore, size_t n)
{
	ce_thread_mutex_lock(semaphore->mutex);
	while (n > semaphore->available) {
		ce_thread_cond_wait(semaphore->cond, semaphore->mutex);
	}
	semaphore->available -= n;
	ce_thread_mutex_unlock(semaphore->mutex);
}

void ce_semaphore_release(ce_semaphore* semaphore, size_t n)
{
	ce_thread_mutex_lock(semaphore->mutex);
	semaphore->available += n;
	ce_thread_cond_wake_all(semaphore->cond);
	ce_thread_mutex_unlock(semaphore->mutex);
}

bool ce_semaphore_try_acquire(ce_semaphore* semaphore, size_t n)
{
	ce_thread_mutex_lock(semaphore->mutex);
	bool result = true;
	if (n > semaphore->available) {
		result = false;
	} else {
		semaphore->available -= n;
	}
	ce_thread_mutex_unlock(semaphore->mutex);
	return result;
}

typedef struct {
	void (*func)(void*);
	void* arg;
} ce_poolcookie;

static ce_poolcookie* ce_poolcookie_new(void)
{
	return ce_alloc(sizeof(ce_poolcookie));
}

static void ce_poolcookie_del(ce_poolcookie* poolcookie)
{
	ce_free(poolcookie, sizeof(ce_poolcookie));
}

static void ce_threadpool_work(ce_threadpool* threadpool)
{
	ce_thread_mutex_lock(threadpool->mutex);

	while (!threadpool->done) {
		if (ce_vector_empty(threadpool->queue)) {
			if (threadpool->idle_thread_count == threadpool->threads->count) {
				ce_thread_cond_wake_one(threadpool->wait_all_cond);
			}
			ce_thread_cond_wait(threadpool->thread_cond, threadpool->mutex);
		} else {
			ce_poolcookie* poolcookie = ce_vector_pop_back(threadpool->queue);
			ce_vector_push_back(threadpool->cache, poolcookie);

			void (*func)(void*) = poolcookie->func;
			void* arg = poolcookie->arg;

			--threadpool->idle_thread_count;
			ce_thread_mutex_unlock(threadpool->mutex);

			(*func)(arg);

			ce_thread_mutex_lock(threadpool->mutex);
			++threadpool->idle_thread_count;

			ce_thread_cond_wake_one(threadpool->wait_one_cond);
		}
	}

	ce_thread_mutex_unlock(threadpool->mutex);
}

ce_threadpool* ce_threadpool_new(size_t thread_count)
{
	ce_threadpool* threadpool = ce_alloc(sizeof(ce_threadpool));
	threadpool->done = false;
	threadpool->idle_thread_count = thread_count;
	threadpool->threads = ce_vector_new_reserved(thread_count);
	threadpool->queue = ce_vector_new();
	threadpool->cache = ce_vector_new();
	threadpool->mutex = ce_thread_mutex_new();
	threadpool->thread_cond = ce_thread_cond_new();
	threadpool->wait_one_cond = ce_thread_cond_new();
	threadpool->wait_all_cond = ce_thread_cond_new();

	for (size_t i = 0; i < thread_count; ++i) {
		ce_vector_push_back(threadpool->threads,
			ce_thread_new(ce_threadpool_work, threadpool));
	}

	return threadpool;
}

void ce_threadpool_del(ce_threadpool* threadpool)
{
	if (NULL != threadpool) {
		ce_thread_mutex_lock(threadpool->mutex);
		threadpool->done = true;
		ce_thread_mutex_unlock(threadpool->mutex);

		ce_thread_cond_wake_all(threadpool->thread_cond);
		ce_vector_for_each(threadpool->threads, ce_thread_wait);

		if (!ce_vector_empty(threadpool->queue)) {
			ce_logging_warning("threadpool: pool is being "
								"destroyed while queue is not empty");
		}

		ce_thread_cond_del(threadpool->wait_all_cond);
		ce_thread_cond_del(threadpool->wait_one_cond);
		ce_thread_cond_del(threadpool->thread_cond);
		ce_thread_mutex_del(threadpool->mutex);

		ce_vector_for_each(threadpool->cache, ce_poolcookie_del);
		ce_vector_for_each(threadpool->queue, ce_poolcookie_del);
		ce_vector_for_each(threadpool->threads, ce_thread_del);

		ce_vector_del(threadpool->cache);
		ce_vector_del(threadpool->queue);
		ce_vector_del(threadpool->threads);

		ce_free(threadpool, sizeof(ce_threadpool));
	}
}

void ce_threadpool_enqueue(ce_threadpool* threadpool, void (*func)(), void* arg)
{
	ce_thread_mutex_lock(threadpool->mutex);
	ce_poolcookie* poolcookie = ce_vector_empty(threadpool->cache) ?
		ce_poolcookie_new() : ce_vector_pop_back(threadpool->cache);
	poolcookie->func = func;
	poolcookie->arg = arg;
	ce_vector_push_back(threadpool->queue, poolcookie);
	ce_thread_cond_wake_one(threadpool->thread_cond);
	ce_thread_mutex_unlock(threadpool->mutex);
}

static void ce_threadpool_wait(ce_threadpool* threadpool, ce_thread_cond* cond)
{
	ce_thread_mutex_lock(threadpool->mutex);
	if (!ce_vector_empty(threadpool->queue) ||
			threadpool->idle_thread_count != threadpool->threads->count) {
		ce_thread_cond_wait(cond, threadpool->mutex);
	}
	ce_thread_mutex_unlock(threadpool->mutex);
}

void ce_threadpool_wait_one(ce_threadpool* threadpool)
{
	ce_threadpool_wait(threadpool, threadpool->wait_one_cond);
}

void ce_threadpool_wait_all(ce_threadpool* threadpool)
{
	ce_threadpool_wait(threadpool, threadpool->wait_all_cond);
}
