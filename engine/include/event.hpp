/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#ifndef CE_EVENT_HPP
#define CE_EVENT_HPP

#include <stddef.h>
#include <stdbool.h>

#include "vector.hpp"
#include "timer.hpp"
#include "thread.hpp"

typedef struct ce_event {
    void (*notify)(struct ce_event* event);
    size_t size;
    char impl[];
} ce_event;

extern ce_event* ce_event_new(void (*notify)(ce_event*), size_t size);
extern void ce_event_del(ce_event* event);

typedef struct {
    void* ptr;
} ce_event_ptr;

/*
 *  Per-thread event queue.
*/

enum {
    CE_EVENT_FLAG_ALL_EVENTS = 0x00,
    CE_EVENT_FLAG_WAIT_FOR_MORE_EVENTS = 0x01,
};

typedef struct {
    volatile bool interrupt;
    volatile size_t event_count;
    ce_thread_id thread_id;
    ce_timer* timer;
    ce_mutex* mutex;
    ce_wait_condition* wait_condition;
    ce_vector* pending_events;
    ce_vector* sending_events;
} ce_event_queue;

extern ce_event_queue* ce_event_queue_new(ce_thread_id thread_id);
extern void ce_event_queue_del(ce_event_queue* queue);

extern bool ce_event_queue_has_pending_events(ce_event_queue* queue);

// process all pending events
extern void ce_event_queue_process_events(ce_event_queue* queue, int flags);

// process pending events for a maximum of max_time milliseconds
extern void ce_event_queue_process_events_timeout(ce_event_queue* queue, int flags, int max_time);

extern void ce_event_queue_add_event(ce_event_queue* queue, ce_event* event);
extern void ce_event_queue_interrupt(ce_event_queue* queue);

/*
 *  Thread-safe event manager.
*/

extern struct ce_event_manager {
    ce_mutex* mutex;
    ce_vector* event_queues;
}* ce_event_manager;

extern void ce_event_manager_init(void);
extern void ce_event_manager_term(void);

extern bool ce_event_manager_has_pending_events(ce_thread_id thread_id);

// process all pending events for the current thread
extern void ce_event_manager_process_events(ce_thread_id thread_id, int flags);

// process pending events for the current thread with time limit in milliseconds
extern void ce_event_manager_process_events_timeout(ce_thread_id thread_id, int flags, int max_time);

extern void ce_event_manager_interrupt(ce_thread_id thread_id);

extern void ce_event_manager_post_event(ce_thread_id thread_id, ce_event* event);
extern void ce_event_manager_post_raw(ce_thread_id thread_id,
                                        void (*notify)(ce_event*),
                                        const void* impl, size_t size);

extern void ce_event_manager_post_ptr(ce_thread_id thread_id,
                                        void (*notify)(ce_event*), void* ptr);

static inline void ce_event_manager_post_call(ce_thread_id thread_id,
                                                void (*notify)(ce_event*))
{
    ce_event_manager_post_event(thread_id, ce_event_new(notify, 0));
}

#endif /* CE_EVENT_HPP */
