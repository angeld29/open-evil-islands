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
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "ceevent.h"

ce_event* ce_event_new(bool (*notify)(ce_event*), size_t size)
{
	ce_event* event = ce_alloc_zero(sizeof(ce_event) + size);
	event->notify = notify;
	event->size = size;
	return event;
}

void ce_event_del(ce_event* event)
{
	if (NULL != event) {
		ce_free(event, sizeof(ce_event) + event->size);
	}
}

ce_event_queue* ce_event_queue_new(ce_thread_id thread_id)
{
	ce_event_queue* queue = ce_alloc(sizeof(ce_event_queue));
	queue->thread_id = thread_id;
	queue->mutex = ce_mutex_new();
	queue->prev_events = ce_vector_new();
	queue->next_events = ce_vector_new();
	return queue;
}

void ce_event_queue_del(ce_event_queue* queue)
{
	if (NULL != queue) {
		ce_vector_for_each(queue->next_events, ce_event_del);
		ce_vector_for_each(queue->prev_events, ce_event_del);
		ce_vector_del(queue->next_events);
		ce_vector_del(queue->prev_events);
		ce_mutex_del(queue->mutex);
		ce_free(queue, sizeof(ce_event_queue));
	}
}

void ce_event_queue_process_events(ce_event_queue* queue)
{
	ce_mutex_lock(queue->mutex);
	ce_swap_temp(ce_vector*, queue->prev_events, queue->next_events);
	ce_mutex_unlock(queue->mutex);

	for (size_t i = 0; i < queue->prev_events->count; ++i) {
		ce_event* event = queue->prev_events->items[i];
		if ((*event->notify)(event)) {
			ce_event_del(event);
			ce_vector_remove_unordered(queue->prev_events, i--);
		}
	}

	if (!ce_vector_empty(queue->prev_events)) {
		ce_mutex_lock(queue->mutex);
		while (!ce_vector_empty(queue->prev_events)) {
			ce_vector_push_back(queue->next_events,
				ce_vector_pop_back(queue->prev_events));
		}
		ce_mutex_unlock(queue->mutex);
	}
}

void ce_event_queue_push_event(ce_event_queue* queue, ce_event* event)
{
	ce_mutex_lock(queue->mutex);
	ce_vector_push_back(queue->next_events, event);
	ce_mutex_unlock(queue->mutex);
}

struct ce_event_manager ce_event_manager;

void ce_event_manager_init(void)
{
	ce_event_manager.mutex = ce_mutex_new();
	ce_event_manager.event_queues = ce_vector_new();
}

void ce_event_manager_term(void)
{
	ce_vector_for_each(ce_event_manager.event_queues, ce_event_queue_del);
	ce_vector_del(ce_event_manager.event_queues);
	ce_mutex_del(ce_event_manager.mutex);
}

void ce_event_manager_create_queue(void)
{
	ce_mutex_lock(ce_event_manager.mutex);
	ce_vector_push_back(ce_event_manager.event_queues,
						ce_event_queue_new(ce_thread_self()));
	ce_mutex_unlock(ce_event_manager.mutex);
}

void ce_event_manager_process_events(void)
{
	ce_thread_id thread_id = ce_thread_self();
	for (size_t i = 0; i < ce_event_manager.event_queues->count; ++i) {
		ce_event_queue* queue = ce_event_manager.event_queues->items[i];
		if (thread_id == queue->thread_id) {
			ce_event_queue_process_events(queue);
		}
	}
}

void ce_event_manager_post_event(ce_thread_id thread_id, ce_event* event)
{
	for (size_t i = 0; i < ce_event_manager.event_queues->count; ++i) {
		ce_event_queue* queue = ce_event_manager.event_queues->items[i];
		if (thread_id == queue->thread_id) {
			ce_event_queue_push_event(ce_event_manager.event_queues->items[i], event);
			break;
		}
	}
}

void ce_event_manager_post_raw(ce_thread_id thread_id,
								bool (*notify)(ce_event*),
								const void* impl, size_t size)
{
	ce_event* event = ce_event_new(notify, size);
	memcpy(event->impl, impl, size);
	ce_event_manager_post_event(thread_id, event);
}
