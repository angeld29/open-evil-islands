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

#include <cstdio>
#include <cstring>
#include <climits>
#include <algorithm>

#include "alloc.hpp"
#include "logging.hpp"
#include "event.hpp"

namespace cursedearth
{
    struct ce_event_manager* ce_event_manager;

    ce_event* ce_event_new(void (*notify)(ce_event*), size_t size)
    {
        ce_event* event = (ce_event*)ce_alloc_zero(sizeof(ce_event));
        event->impl = ce_alloc_zero(size);
        event->notify = notify;
        event->size = size;
        return event;
    }

    void ce_event_del(ce_event* event)
    {
        if (NULL != event) {
            ce_free(event->impl, event->size);
            ce_free(event, sizeof(ce_event));
        }
    }

    ce_event_queue* ce_event_queue_new(ce_thread_id thread_id)
    {
        ce_event_queue* queue = (ce_event_queue*)ce_alloc_zero(sizeof(ce_event_queue));
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
            ce_vector_for_each(queue->sending_events, (void(*)(void*))ce_event_del);
            ce_vector_for_each(queue->pending_events, (void(*)(void*))ce_event_del);
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
        ce_mutex_lock(queue->mutex);

        for (;;) {
            if (ce_vector_empty(queue->sending_events)) {
                std::swap(queue->pending_events, queue->sending_events);
            }

            if (!ce_vector_empty(queue->sending_events)) {
                ce_mutex_unlock(queue->mutex);

                size_t sent_event_count = 0;
                ce_timer_start(queue->timer);

                // I (and timer) like seconds
                for (float time = 0.0f, limit = 1e-3f * max_time; !ce_vector_empty(queue->sending_events) && time < limit; time += ce_timer_elapsed(queue->timer), ++sent_event_count) {
                    // TODO: linked list ?
                    ce_event* event = (ce_event*)ce_vector_pop_front(queue->sending_events);
                    (*event->notify)(event);
                    ce_event_del(event);
                    ce_timer_advance(queue->timer);
                }

                ce_mutex_lock(queue->mutex);
                queue->event_count -= sent_event_count;
            }

            if (queue->interrupt) {
                break;
            }

            if (0 == queue->event_count) {
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
        ce_event_manager = (struct ce_event_manager*)ce_alloc_zero(sizeof(struct ce_event_manager));
        ce_event_manager->mutex = ce_mutex_new();
        ce_event_manager->event_queues = ce_vector_new();
    }

    void ce_event_manager_term(void)
    {
        if (NULL != ce_event_manager) {
            ce_vector_for_each(ce_event_manager->event_queues, (void(*)(void*))ce_event_queue_del);
            ce_vector_del(ce_event_manager->event_queues);
            ce_mutex_del(ce_event_manager->mutex);
            ce_free(ce_event_manager, sizeof(struct ce_event_manager));
        }
    }

    void ce_event_manager_create_queue(ce_thread_id thread_id)
    {
        ce_mutex_lock(ce_event_manager->mutex);
        ce_vector_push_back(ce_event_manager->event_queues,
                            ce_event_queue_new(thread_id));
        ce_mutex_unlock(ce_event_manager->mutex);
    }

    bool ce_event_manager_has_pending_events(ce_thread_id thread_id)
    {
        for (size_t i = 0; i < ce_event_manager->event_queues->count; ++i) {
            ce_event_queue* queue = (ce_event_queue*)ce_event_manager->event_queues->items[i];
            if (thread_id == queue->thread_id) {
                return ce_event_queue_has_pending_events(queue);
            }
        }
        return false;
    }

    void ce_event_manager_process_events_timeout(ce_thread_id thread_id, int max_time)
    {
        for (size_t i = 0; i < ce_event_manager->event_queues->count; ++i) {
            ce_event_queue* queue = (ce_event_queue*)ce_event_manager->event_queues->items[i];
            if (thread_id == queue->thread_id) {
                ce_event_queue_process_events_timeout(queue, max_time);
                return;
            }
        }

        ce_event_manager_create_queue(thread_id);
        ce_event_manager_process_events_timeout(thread_id, max_time);
    }

    void ce_event_manager_post_event(ce_thread_id thread_id, ce_event* event)
    {
        for (size_t i = 0; i < ce_event_manager->event_queues->count; ++i) {
            ce_event_queue* queue = (ce_event_queue*)ce_event_manager->event_queues->items[i];
            if (thread_id == queue->thread_id) {
                ce_event_queue_add_event(queue, event);
                return;
            }
        }

        ce_event_manager_create_queue(thread_id);
        ce_event_manager_post_event(thread_id, event);
    }

    void ce_event_manager_post_raw(ce_thread_id thread_id, void (*notify)(ce_event*), const void* impl, size_t size)
    {
        ce_event* event = ce_event_new(notify, size);
        memcpy(event->impl, impl, size);
        ce_event_manager_post_event(thread_id, event);
    }

    void ce_event_manager_post_ptr(ce_thread_id thread_id, void (*notify)(ce_event*), void* ptr)
    {
        ce_event_ptr event = { ptr };
        ce_event_manager_post_raw(thread_id, notify, &event, sizeof(ce_event_ptr));
    }
}
