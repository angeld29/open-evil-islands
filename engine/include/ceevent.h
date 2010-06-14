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

#include "cevector.h"
#include "cethread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	CE_EVENT_TYPE_COUNT,
} ce_event_type;

typedef struct {
	ce_event_type type;
} ce_event;

extern ce_event* ce_event_new(ce_event_type type);
extern void ce_event_del(ce_event* event);

/*
 *  Per-thread event queue
*/

typedef struct {
	ce_thread_id id;
	ce_mutex* mutex;
	ce_vector* events;
} ce_event_queue;

extern ce_event_queue* ce_event_queue_new(ce_thread_id id);
extern void ce_event_queue_del(ce_event_queue* queue);

/*
 *  Thread-safe event manager
*/

typedef struct {
	ce_vector* queues;
} ce_event_manager;

extern ce_event_manager* ce_event_manager_new(void);
extern void ce_event_manager_del(ce_event_manager* manager);

extern void ce_event_manager_process(void);

#ifdef __cplusplus
}
#endif

#endif /* CE_EVENT_H */
