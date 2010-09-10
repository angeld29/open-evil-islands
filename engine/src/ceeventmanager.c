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

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceeventmanager.h"

struct ce_event_manager* ce_event_manager;

ce_event_queue* ce_event_queue_new(ce_thread_id thread_id)
{
	ce_event_queue* queue = ce_alloc_zero(sizeof(ce_event_queue));
	queue->thread_id = thread_id;
	queue->timer = ce_timer_new();
	queue->mutex = ce_mutex_new();
	queue->wait_condition = ce_wait_condition_new();
	queue->pending_events = ce_vector_new();
	queue->sending_events = ce_vector_new();
	return queue;
}

void ce_event_queue_del(ce_event_queue* queue)
{
	if (NULL != queue) {
		ce_vector_for_each(queue->sending_events, ce_event_del);
		ce_vector_for_each(queue->pending_events, ce_event_del);
		ce_vector_del(queue->sending_events);
		ce_vector_del(queue->pending_events);
		ce_wait_condition_del(queue->wait_condition);
		ce_mutex_del(queue->mutex);
		ce_timer_del(queue->timer);
		ce_free(queue, sizeof(ce_event_queue));
	}
}

bool ce_event_queue_has_pending_events(ce_event_queue* queue)
{
	ce_mutex_lock(queue->mutex);
	bool result = 0 != queue->event_count;
	ce_mutex_unlock(queue->mutex);
	return result;
}

void ce_event_queue_process_events(ce_event_queue* queue)
{
	ce_event_queue_process_events_timeout(queue, INT_MAX);
}

void ce_event_queue_process_events_timeout(ce_event_queue* queue, int max_time)
{
	if (ce_vector_empty(queue->sending_events)) {
		ce_mutex_lock(queue->mutex);
		ce_swap_pointer(&queue->pending_events, &queue->sending_events);
		ce_mutex_unlock(queue->mutex);
	}

	if (!ce_vector_empty(queue->sending_events)) {
		size_t sent_event_count = 0;
		ce_timer_start(queue->timer);

		// I (and timer) like seconds
		for (float time = 0.0f, limit = 1e-3f * max_time;
				!ce_vector_empty(queue->sending_events) && time < limit;
				time += queue->timer->elapsed, ++sent_event_count) {
			// TODO: linked list ?
			ce_event* event = ce_vector_pop_front(queue->sending_events);
			(*event->notify)(event);
			ce_event_del(event);
			ce_timer_advance(queue->timer);
		}

		ce_mutex_lock(queue->mutex);
		queue->event_count -= sent_event_count;
		ce_mutex_unlock(queue->mutex);
	}
}

void ce_event_queue_process_events2(ce_event_queue* queue, int flags)
{
	bool wait_for_more_events = CE_EVENT_FLAG_WAIT_FOR_MORE_EVENTS & flags;

	ce_mutex_lock(queue->mutex);

	for (;;) {
		if (ce_vector_empty(queue->sending_events)) {
			ce_swap_pointer(&queue->pending_events, &queue->sending_events);
		}

		if (!ce_vector_empty(queue->sending_events)) {
			queue->event_count -= queue->sending_events->count;

			ce_mutex_unlock(queue->mutex);

			while (!ce_vector_empty(queue->sending_events)) {
				ce_event* event = ce_vector_pop_front(queue->sending_events);
				(*event->notify)(event);
				ce_event_del(event);
			}

			ce_mutex_lock(queue->mutex);
		}

		if (queue->interrupt || !wait_for_more_events) {
			break;
		}

		if (ce_vector_empty(queue->pending_events)) {
			ce_wait_condition_wait(queue->wait_condition, queue->mutex);
		}
	}

	ce_mutex_unlock(queue->mutex);
}

void ce_event_queue_add_event(ce_event_queue* queue, ce_event* event)
{
	ce_mutex_lock(queue->mutex);
	ce_vector_push_back(queue->pending_events, event);
	++queue->event_count;
	ce_wait_condition_wake_all(queue->wait_condition);
	ce_mutex_unlock(queue->mutex);
}

void ce_event_queue_interrupt(ce_event_queue* queue)
{
	ce_mutex_lock(queue->mutex);
	queue->interrupt = true;
	ce_wait_condition_wake_all(queue->wait_condition);
	ce_mutex_unlock(queue->mutex);
}

void ce_event_manager_init(void)
{
	ce_event_manager = ce_alloc_zero(sizeof(struct ce_event_manager));
	ce_event_manager->mutex = ce_mutex_new();
	ce_event_manager->event_queues = ce_vector_new();
}

void ce_event_manager_term(void)
{
	if (NULL != ce_event_manager) {
		ce_vector_for_each(ce_event_manager->event_queues, ce_event_queue_del);
		ce_vector_del(ce_event_manager->event_queues);
		ce_mutex_del(ce_event_manager->mutex);
		ce_free(ce_event_manager, sizeof(struct ce_event_manager));
	}
}

bool ce_event_manager_has_pending_events(void)
{
	ce_thread_id thread_id = ce_thread_self();
	for (size_t i = 0; i < ce_event_manager->event_queues->count; ++i) {
		ce_event_queue* queue = ce_event_manager->event_queues->items[i];
		if (thread_id == queue->thread_id) {
			return ce_event_queue_has_pending_events(queue);
		}
	}
	return false;
}

void ce_event_manager_process_events(void)
{
	ce_event_manager_process_events_timeout(INT_MAX);
}

void ce_event_manager_process_events_timeout(int max_time)
{
	ce_thread_id thread_id = ce_thread_self();
	for (size_t i = 0; i < ce_event_manager->event_queues->count; ++i) {
		ce_event_queue* queue = ce_event_manager->event_queues->items[i];
		if (thread_id == queue->thread_id) {
			ce_event_queue_process_events_timeout(queue, max_time);
		}
	}
}

static void ce_event_manager_create_queue(ce_thread_id thread_id)
{
	ce_mutex_lock(ce_event_manager->mutex);
	ce_vector_push_back(ce_event_manager->event_queues,
						ce_event_queue_new(thread_id));
	ce_mutex_unlock(ce_event_manager->mutex);
}

void ce_event_manager_process_events2(int flags)
{
	ce_thread_id thread_id = ce_thread_self();
	for (size_t i = 0; i < ce_event_manager->event_queues->count; ++i) {
		ce_event_queue* queue = ce_event_manager->event_queues->items[i];
		if (thread_id == queue->thread_id) {
			ce_event_queue_process_events2(queue, flags);
			return;
		}
	}

	ce_event_manager_create_queue(thread_id);
	ce_event_manager_process_events2(flags);
}

void ce_event_manager_interrupt(ce_thread_id thread_id)
{
	for (size_t i = 0; i < ce_event_manager->event_queues->count; ++i) {
		ce_event_queue* queue = ce_event_manager->event_queues->items[i];
		if (thread_id == queue->thread_id) {
			ce_event_queue_interrupt(queue);
		}
	}
}

void ce_event_manager_post_event(ce_thread_id thread_id, ce_event* event)
{
	for (size_t i = 0; i < ce_event_manager->event_queues->count; ++i) {
		ce_event_queue* queue = ce_event_manager->event_queues->items[i];
		if (thread_id == queue->thread_id) {
			ce_event_queue_add_event(queue, event);
			return;
		}
	}

	ce_event_manager_create_queue(thread_id);
	ce_event_manager_post_event(thread_id, event);
}

void ce_event_manager_post_raw(ce_thread_id thread_id,
								void (*notify)(ce_event*),
								const void* impl, size_t size)
{
	ce_event* event = ce_event_new(notify, size);
	memcpy(event->impl, impl, size);
	ce_event_manager_post_event(thread_id, event);
}

void ce_event_manager_post_ptr(ce_thread_id thread_id,
									void (*notify)(ce_event*), void* ptr)
{
	ce_event_manager_post_raw(thread_id, notify,
		&(ce_event_ptr){ptr}, sizeof(ce_event_ptr));
}
