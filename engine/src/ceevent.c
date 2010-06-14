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

#include "cealloc.h"
#include "ceevent.h"

ce_event* ce_event_new(ce_event_type type)
{
	ce_event* event = ce_alloc(sizeof(ce_event));
	event->type = type;
	return event;
}

void ce_event_del(ce_event* event)
{
	if (NULL != event) {
		ce_free(event, sizeof(ce_event));
	}
}

ce_event_queue* ce_event_queue_new(ce_thread_id id)
{
	ce_event_queue* queue = ce_alloc(sizeof(ce_event_queue));
	queue->id = id;
	queue->mutex = ce_mutex_new();
	queue->events = ce_vector_new();
	return queue;
}

void ce_event_queue_del(ce_event_queue* queue)
{
	if (NULL != queue) {
		ce_vector_for_each(queue->events, ce_event_del);
		ce_vector_del(queue->events);
		ce_mutex_del(queue->mutex);
		ce_free(queue, sizeof(ce_event_queue));
	}
}

ce_event_manager* ce_event_manager_new(void)
{
	ce_event_manager* manager = ce_alloc(sizeof(ce_event_manager));
	manager->queues = ce_vector_new();
	return manager;
}

void ce_event_manager_del(ce_event_manager* manager)
{
	if (NULL != manager) {
		ce_vector_for_each(manager->queues, ce_event_queue_del);
		ce_vector_del(manager->queues);
		ce_free(manager, sizeof(ce_event_manager));
	}
}
