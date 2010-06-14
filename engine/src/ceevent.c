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

#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "ceevent.h"

ce_event* ce_event_new(ce_event_vtable vtable)
{
	ce_event* event = ce_alloc_zero(sizeof(ce_event) + vtable.size);
	event->vtable = vtable;
	return event;
}

void ce_event_del(ce_event* event)
{
	if (NULL != event) {
		if (NULL != event->vtable.dtor) {
			(*event->vtable.dtor)(event);
		}
		ce_free(event, sizeof(ce_event) + event->vtable.size);
	}
}

ce_event_queue* ce_event_queue_new(ce_thread_id id)
{
	ce_event_queue* queue = ce_alloc(sizeof(ce_event_queue));
	queue->id = id;
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

void ce_event_queue_process(ce_event_queue* queue)
{
	ce_mutex_lock(queue->mutex);
	ce_swap_temp(ce_vector*, queue->prev_events, queue->next_events);
	ce_mutex_unlock(queue->mutex);

	for (size_t i = 0; i < queue->prev_events->count; ++i) {
		ce_event* event = queue->prev_events->items[i];
		if ((*event->vtable.notify)(event)) {
			ce_event_del(event);
			ce_vector_remove_unordered(queue->prev_events, i);
			--i;
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

void ce_event_queue_put(ce_event_queue* queue, ce_event* event)
{
	ce_mutex_lock(queue->mutex);
	ce_vector_push_back(queue->next_events, event);
	ce_mutex_unlock(queue->mutex);
}

ce_event_manager* ce_event_manager_new(void)
{
	ce_event_manager* manager = ce_alloc(sizeof(ce_event_manager));
	manager->mutex = ce_mutex_new();
	manager->queues = ce_vector_new();
	return manager;
}

void ce_event_manager_del(ce_event_manager* manager)
{
	if (NULL != manager) {
		ce_vector_for_each(manager->queues, ce_event_queue_del);
		ce_vector_del(manager->queues);
		ce_mutex_del(manager->mutex);
		ce_free(manager, sizeof(ce_event_manager));
	}
}

void ce_event_manager_process(ce_event_manager* manager)
{
	ce_thread_id id = ce_thread_self();

	for (size_t i = 0; i < manager->queues->count; ++i) {
		ce_event_queue* queue = manager->queues->items[i];
		if (id == queue->id) {
			ce_event_queue_process(queue);
		}
	}
}

void ce_event_manager_post(ce_event_manager* manager, ce_thread_id id, ce_event* event)
{
	ce_mutex_lock(manager->mutex);

	size_t index;
	for (index = 0; index < manager->queues->count; ++index) {
		ce_event_queue* queue = manager->queues->items[index];
		if (id == queue->id) {
			break;
		}
	}

	if (index == manager->queues->count) {
		ce_vector_push_back(manager->queues, ce_event_queue_new(id));
	}

	ce_mutex_unlock(manager->mutex);

	ce_event_queue_put(manager->queues->items[index], event);
}
