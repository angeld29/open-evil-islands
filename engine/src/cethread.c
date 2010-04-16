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

#include "cealloc.h"
#include "celogging.h"
#include "cethread.h"

typedef struct {
	void (*func)(void*);
	void* arg;
} ce_thread_job;

static ce_thread_job* ce_thread_job_new(void)
{
	ce_thread_job* job = ce_alloc(sizeof(ce_thread_job));
	return job;
}

static void ce_thread_job_del(ce_thread_job* job)
{
	ce_free(job, sizeof(ce_thread_job));
}

static void* ce_thread_pool_work(void* arg)
{
	ce_thread_pool* pool = arg;
	ce_thread_mutex_lock(pool->mutex);

	while (!pool->done) {
		if (ce_vector_empty(pool->jobs)) {
			if (pool->idle_thread_count == pool->threads->count) {
				ce_thread_cond_wake_one(pool->wait_cond);
			}
			ce_thread_cond_wait(pool->thread_cond, pool->mutex);
		} else {
			ce_thread_job* job = ce_vector_pop_back(pool->jobs);
			ce_vector_push_back(pool->free_jobs, job);

			void (*func)(void*) = job->func;
			void* arg = job->arg;

			--pool->idle_thread_count;
			ce_thread_mutex_unlock(pool->mutex);

			(*func)(arg);

			ce_thread_mutex_lock(pool->mutex);
			++pool->idle_thread_count;
		}
	}

	ce_thread_mutex_unlock(pool->mutex);
	return arg;
}

ce_thread_pool* ce_thread_pool_new(int thread_count)
{
	ce_thread_pool* pool = ce_alloc(sizeof(ce_thread_pool));
	pool->done = false;
	pool->idle_thread_count = thread_count;
	pool->threads = ce_vector_new_reserved(thread_count);
	pool->jobs = ce_vector_new();
	pool->free_jobs = ce_vector_new();
	pool->mutex = ce_thread_mutex_new();
	pool->thread_cond = ce_thread_cond_new();
	pool->wait_cond = ce_thread_cond_new();

	for (int i = 0; i < thread_count; ++i) {
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

		if (!ce_vector_empty(pool->jobs)) {
			ce_logging_warning("thread_pool: "
				"destroyed while jobs are still existing");
		}

		ce_vector_for_each(pool->free_jobs, ce_thread_job_del);
		ce_vector_for_each(pool->jobs, ce_thread_job_del);
		ce_vector_for_each(pool->threads, ce_thread_del);

		ce_thread_cond_del(pool->wait_cond);
		ce_thread_cond_del(pool->thread_cond);
		ce_thread_mutex_del(pool->mutex);
		ce_vector_del(pool->free_jobs);
		ce_vector_del(pool->jobs);
		ce_vector_del(pool->threads);
		ce_free(pool, sizeof(ce_thread_pool));
	}
}

void ce_thread_pool_enqueue(ce_thread_pool* pool, void (*func)(void*), void* arg)
{
	ce_thread_mutex_lock(pool->mutex);
	ce_thread_job* job = ce_vector_empty(pool->free_jobs) ?
		ce_thread_job_new() : ce_vector_pop_back(pool->free_jobs);
	job->func = func;
	job->arg = arg;
	ce_vector_push_back(pool->jobs, job);
	ce_thread_cond_wake_one(pool->thread_cond);
	ce_thread_mutex_unlock(pool->mutex);
}

void ce_thread_pool_wait(ce_thread_pool* pool)
{
	ce_thread_mutex_lock(pool->mutex);
	if (!ce_vector_empty(pool->jobs) ||
			pool->idle_thread_count != pool->threads->count) {
		ce_thread_cond_wait(pool->wait_cond, pool->mutex);
	}
	ce_thread_mutex_unlock(pool->mutex);
}
