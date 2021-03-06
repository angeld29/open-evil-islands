/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include "thread.hpp"
#include "vector.hpp"
#include "timer.hpp"

#include <atomic>

namespace cursedearth
{
    typedef struct ce_event {
        void (*notify)(struct ce_event* event);
        size_t size;
        void* impl;
    } ce_event;

    ce_event* ce_event_new(void (*notify)(ce_event*), size_t size);
    void ce_event_del(ce_event* event);

    typedef struct {
        void* ptr;
    } ce_event_ptr;

    typedef struct {
        std::atomic<size_t> event_count;
        ce_thread_id thread_id;
        timer_ptr_t timer;
        ce_mutex* mutex;
        ce_vector* pending_events;
        ce_vector* sending_events;
    } ce_event_queue;

    ce_event_queue* ce_event_queue_new(ce_thread_id thread_id);
    void ce_event_queue_del(ce_event_queue* queue);

    bool ce_event_queue_has_pending_events(ce_event_queue* queue);

    // process pending events for a maximum of max_time milliseconds
    void ce_event_queue_process_events_timeout(ce_event_queue* queue, int max_time);

    void ce_event_queue_add_event(ce_event_queue* queue, ce_event* event);

    extern struct ce_event_manager {
        ce_mutex* mutex;
        ce_vector* event_queues;
    }* ce_event_manager;

    void ce_event_manager_init(void);
    void ce_event_manager_term(void);

    bool ce_event_manager_has_pending_events(ce_thread_id thread_id);

    // process pending events for the current thread with time limit in milliseconds
    void ce_event_manager_process_events_timeout(ce_thread_id thread_id, int max_time);

    void ce_event_manager_post_event(ce_thread_id thread_id, ce_event* event);
    void ce_event_manager_post_raw(ce_thread_id thread_id, void (*notify)(ce_event*), const void* impl, size_t size);
    void ce_event_manager_post_ptr(ce_thread_id thread_id, void (*notify)(ce_event*), void* ptr);
}

#endif
