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

#ifndef CE_EVENTMANAGER_H
#define CE_EVENTMANAGER_H

#include <stddef.h>
#include <stdbool.h>

#include "cevector.h"
#include "cetimer.h"
#include "cethread.h"
#include "ceevent.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Per-thread event queue.
*/

typedef struct {
	size_t event_count;
	ce_thread_id thread_id;
	ce_timer* timer;
	ce_mutex* mutex;
	ce_vector* pending_events;
	ce_vector* sending_events;
} ce_event_queue;

extern ce_event_queue* ce_event_queue_new(ce_thread_id thread_id);
extern void ce_event_queue_del(ce_event_queue* queue);

extern bool ce_event_queue_has_pending_events(ce_event_queue* queue);

// process all pending events
extern void ce_event_queue_process_events(ce_event_queue* queue);

// process pending events for a maximum of max_time milliseconds
extern void ce_event_queue_process_events_timeout(ce_event_queue* queue, int max_time);

extern void ce_event_queue_add_event(ce_event_queue* queue, ce_event* event);

/*
 *  Thread-safe event manager.
*/

extern struct ce_event_manager {
	ce_mutex* mutex;
	ce_vector* event_queues;
}* ce_event_manager;

extern void ce_event_manager_init(void);
extern void ce_event_manager_term(void);

// create a queue for the current thread
extern void ce_event_manager_create_queue(void);

extern bool ce_event_manager_has_pending_events(void);

// process all pending events for the current thread
extern void ce_event_manager_process_events(void);

// process pending events for the current thread with time limit in milliseconds
extern void ce_event_manager_process_events_timeout(int max_time);

extern void ce_event_manager_post_event(ce_thread_id thread_id, ce_event* event);
extern void ce_event_manager_post_raw(ce_thread_id thread_id,
										void (*notify)(ce_event*),
										const void* impl, size_t size);

extern void ce_event_manager_post_pointer(ce_thread_id thread_id,
										void (*notify)(ce_event*), void* ptr);

static inline void ce_event_manager_post_call(ce_thread_id thread_id,
												void (*notify)(ce_event*))
{
	ce_event_manager_post_event(thread_id, ce_event_new(notify, 0));
}

#ifdef __cplusplus
}
#endif

#endif /* CE_EVENTMANAGER_H */
