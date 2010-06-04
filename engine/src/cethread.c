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
} ce_thread_cookie;

static ce_thread_cookie* ce_thread_cookie_new(void)
{
	return ce_alloc(sizeof(ce_thread_cookie));
}

static void ce_thread_cookie_del(ce_thread_cookie* cookie)
{
	ce_free(cookie, sizeof(ce_thread_cookie));
}

static void ce_thread_pool_work(void* arg)
{
	ce_thread_pool* pool = arg;
	ce_thread_mutex_lock(pool->mutex);

	while (!pool->done) {
		if (ce_vector_empty(pool->queue)) {
			if (pool->idle_thread_count == pool->threads->count) {
				ce_thread_cond_wake_one(pool->wait_all_cond);
			}
			ce_thread_cond_wait(pool->thread_cond, pool->mutex);
		} else {
			ce_thread_cookie* cookie = ce_vector_pop_back(pool->queue);
			ce_vector_push_back(pool->cache, cookie);

			void (*func)(void*) = cookie->func;
			void* arg = cookie->arg;

			--pool->idle_thread_count;
			ce_thread_mutex_unlock(pool->mutex);

			(*func)(arg);

			ce_thread_mutex_lock(pool->mutex);
			++pool->idle_thread_count;

			ce_thread_cond_wake_one(pool->wait_one_cond);
		}
	}

	ce_thread_mutex_unlock(pool->mutex);
}

ce_thread_pool* ce_thread_pool_new(size_t thread_count)
{
	ce_thread_pool* pool = ce_alloc(sizeof(ce_thread_pool));
	pool->done = false;
	pool->idle_thread_count = thread_count;
	pool->threads = ce_vector_new_reserved(thread_count);
	pool->queue = ce_vector_new();
	pool->cache = ce_vector_new();
	pool->mutex = ce_thread_mutex_new();
	pool->thread_cond = ce_thread_cond_new();
	pool->wait_one_cond = ce_thread_cond_new();
	pool->wait_all_cond = ce_thread_cond_new();

	for (size_t i = 0; i < thread_count; ++i) {
		ce_vector_push_back(pool->threads,
			ce_thread_new(ce_thread_pool_work, pool));
	}

	return pool;
}

void ce_thread_pool_del(ce_thread_pool* pool)
{
	if (NULL != pool) {
		ce_thread_mutex_lock(pool->mutex);
		pool->done = true;
		ce_thread_mutex_unlock(pool->mutex);

		ce_thread_cond_wake_all(pool->thread_cond);
		ce_vector_for_each(pool->threads, ce_thread_wait);

		if (!ce_vector_empty(pool->queue)) {
			ce_logging_warning("thread: "
				"pool is being destroyed while queue is not empty");
		}

		ce_thread_cond_del(pool->wait_all_cond);
		ce_thread_cond_del(pool->wait_one_cond);
		ce_thread_cond_del(pool->thread_cond);
		ce_thread_mutex_del(pool->mutex);

		ce_vector_for_each(pool->cache, ce_thread_cookie_del);
		ce_vector_for_each(pool->queue, ce_thread_cookie_del);
		ce_vector_for_each(pool->threads, ce_thread_del);

		ce_vector_del(pool->cache);
		ce_vector_del(pool->queue);
		ce_vector_del(pool->threads);

		ce_free(pool, sizeof(ce_thread_pool));
	}
}

void ce_thread_pool_enqueue(ce_thread_pool* pool, void (*func)(), void* arg)
{
	ce_thread_mutex_lock(pool->mutex);
	ce_thread_cookie* cookie = ce_vector_empty(pool->cache) ?
		ce_thread_cookie_new() : ce_vector_pop_back(pool->cache);
	cookie->func = func; cookie->arg = arg;
	ce_vector_push_back(pool->queue, cookie);
	ce_thread_cond_wake_one(pool->thread_cond);
	ce_thread_mutex_unlock(pool->mutex);
}

static void ce_thread_pool_wait(ce_thread_pool* pool, ce_thread_cond* cond)
{
	ce_thread_mutex_lock(pool->mutex);
	if (!ce_vector_empty(pool->queue) ||
			pool->idle_thread_count != pool->threads->count) {
		ce_thread_cond_wait(cond, pool->mutex);
	}
	ce_thread_mutex_unlock(pool->mutex);
}

void ce_thread_pool_wait_one(ce_thread_pool* pool)
{
	ce_thread_pool_wait(pool, pool->wait_one_cond);
}

void ce_thread_pool_wait_all(ce_thread_pool* pool)
{
	ce_thread_pool_wait(pool, pool->wait_all_cond);
}
