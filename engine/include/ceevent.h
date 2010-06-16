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

#ifndef CE_EVENT_H
#define CE_EVENT_H

#include <stddef.h>

#include "cevector.h"
#include "cethread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ce_event {
	void (*notify)(struct ce_event* event);
	size_t size;
	char impl[];
} ce_event;

extern ce_event* ce_event_new(void (*notify)(ce_event*), size_t size);
extern void ce_event_del(ce_event* event);

/*
 *  Per-thread event queue.
*/

typedef struct {
	ce_thread_id thread_id;
	ce_mutex* mutex;
	ce_vector* pending_events;
	ce_vector* sent_events;
} ce_event_queue;

extern ce_event_queue* ce_event_queue_new(ce_thread_id thread_id);
extern void ce_event_queue_del(ce_event_queue* queue);

extern void ce_event_queue_process(ce_event_queue* queue);
extern void ce_event_queue_push(ce_event_queue* queue, ce_event* event);

/*
 *  Thread-safe event manager.
*/

extern struct ce_event_manager {
	ce_mutex* mutex;
	ce_vector* event_queues;
} ce_event_manager;

extern void ce_event_manager_init(void);
extern void ce_event_manager_term(void);

extern void ce_event_manager_create(void);
extern void ce_event_manager_process(void);

extern void ce_event_manager_post_event(ce_thread_id thread_id, ce_event* event);
extern void ce_event_manager_post(ce_thread_id thread_id,
									void (*notify)(ce_event*),
									const void* impl, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* CE_EVENT_H */
