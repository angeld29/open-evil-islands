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
#include "celogging.h"
#include "ceeventmanager.h"

ce_event_queue* ce_event_queue_new(ce_thread_id thread_id)
{
	ce_event_queue* queue = ce_alloc(sizeof(ce_event_queue));
	queue->thread_id = thread_id;
	queue->mutex = ce_mutex_new();
	queue->pending_events = ce_vector_new();
	queue->sent_events = ce_vector_new();
	return queue;
}

void ce_event_queue_del(ce_event_queue* queue)
{
	if (NULL != queue) {
		ce_vector_for_each(queue->sent_events, ce_event_del);
		ce_vector_for_each(queue->pending_events, ce_event_del);
		ce_vector_del(queue->sent_events);
		ce_vector_del(queue->pending_events);
		ce_mutex_del(queue->mutex);
		ce_free(queue, sizeof(ce_event_queue));
	}
}

void ce_event_queue_process(ce_event_queue* queue)
{
	ce_mutex_lock(queue->mutex);
	ce_swap_temp(ce_vector*, queue->pending_events, queue->sent_events);
	ce_mutex_unlock(queue->mutex);

	for (size_t i = 0; i < queue->sent_events->count; ++i) {
		ce_event* event = queue->sent_events->items[i];
		(*event->notify)(event);
	}

	ce_vector_for_each(queue->sent_events, ce_event_del);
	ce_vector_clear(queue->sent_events);
}

void ce_event_queue_push(ce_event_queue* queue, ce_event* event)
{
	ce_mutex_lock(queue->mutex);
	ce_vector_push_back(queue->pending_events, event);
	ce_mutex_unlock(queue->mutex);
}

struct ce_event_manager* ce_event_manager;

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

void ce_event_manager_create_queue(void)
{
	ce_mutex_lock(ce_event_manager->mutex);
	ce_vector_push_back(ce_event_manager->event_queues,
						ce_event_queue_new(ce_thread_self()));
	ce_mutex_unlock(ce_event_manager->mutex);
}

void ce_event_manager_process_events(void)
{
	ce_thread_id thread_id = ce_thread_self();
	for (size_t i = 0; i < ce_event_manager->event_queues->count; ++i) {
		ce_event_queue* queue = ce_event_manager->event_queues->items[i];
		if (thread_id == queue->thread_id) {
			ce_event_queue_process(queue);
		}
	}
}

void ce_event_manager_post_event(ce_thread_id thread_id, ce_event* event)
{
	for (size_t i = 0; i < ce_event_manager->event_queues->count; ++i) {
		ce_event_queue* queue = ce_event_manager->event_queues->items[i];
		if (thread_id == queue->thread_id) {
			ce_event_queue_push(ce_event_manager->event_queues->items[i], event);
			return;
		}
	}

	assert(false && "could not found queue");
	ce_logging_critical("event manager: could not found queue");

	ce_event_del(event);
}

void ce_event_manager_post_raw(ce_thread_id thread_id,
								void (*notify)(ce_event*),
								const void* impl, size_t size)
{
	ce_event* event = ce_event_new(notify, size);
	memcpy(event->impl, impl, size);
	ce_event_manager_post_event(thread_id, event);
}
