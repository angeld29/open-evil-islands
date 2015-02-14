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

#ifndef CE_CONCURRENCY_HPP
#define CE_CONCURRENCY_HPP

#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <chrono>

#include <cassert>

#include <boost/noncopyable.hpp>

namespace cursedearth
{
    typedef unsigned long int ce_thread_id;

    struct ce_thread;

    ce_thread_id ce_thread_self(void);

    ce_thread* ce_thread_new(void (*proc)(), void* arg);
    void ce_thread_del(ce_thread* thread);

    ce_thread_id ce_thread_get_id(ce_thread* thread);

    void ce_thread_wait(ce_thread* thread);

    typedef struct ce_mutex ce_mutex;

    ce_mutex* ce_mutex_new(void);
    void ce_mutex_del(ce_mutex* mutex);

    void ce_mutex_lock(ce_mutex* mutex);
    void ce_mutex_unlock(ce_mutex* mutex);

    typedef struct ce_wait_condition ce_wait_condition;

    ce_wait_condition* ce_wait_condition_new(void);
    void ce_wait_condition_del(ce_wait_condition* wait_condition);

    void ce_wait_condition_wake_one(ce_wait_condition* wait_condition);
    void ce_wait_condition_wake_all(ce_wait_condition* wait_condition);
    void ce_wait_condition_wait(ce_wait_condition* wait_condition, ce_mutex* mutex);

    /**
     * @brief The once struct provides an once-only initialization.
     */
    typedef struct ce_once ce_once;

    ce_once* ce_once_new(void);
    void ce_once_del(ce_once* once);

    void ce_once_exec(ce_once* once, void (*proc)(), void* arg);

    /**
     * @brief interruptible thread
     */
    class thread_interrupted_t {};

    extern thread_local class interrupt_thread_flag_t
    {
    public:
        interrupt_thread_flag_t(): m_flag(false) {}

        void operator =(bool flag) { m_flag = flag; }
        explicit operator bool() const { return m_flag; }

    private:
        std::atomic<bool> m_flag;
    } g_interrupt_thread_flag;

    inline void interruption_point()
    {
        if (g_interrupt_thread_flag) {
            throw thread_interrupted_t();
        }
    }

    class interruptible_thread_t
    {
    public:
        template <typename F>
        interruptible_thread_t(F f)
        {
            std::promise<interrupt_thread_flag_t*> promise;
            m_thread = std::thread([f, &promise] {
                promise.set_value(&g_interrupt_thread_flag);
                f();
            });
            m_flag = promise.get_future().get();
        }

        bool joinable() const { return m_thread.joinable(); }
        void join() { m_thread.join(); }
        void detach() { m_thread.detach(); }
        void interrupt() { assert(m_flag); *m_flag = true; }

    private:
        std::thread m_thread;
        interrupt_thread_flag_t* m_flag;
    };

    /**
     * @brief the semaphore class provides a general counting semaphore
     */
    class semaphore_t: boost::noncopyable
    {
    public:
        explicit semaphore_t(size_t n);

        size_t available() const { return m_available; }

        void acquire(size_t n = 1);
        void release(size_t n = 1);

        bool try_acquire(size_t n = 1);

    private:
        std::atomic<size_t> m_available;
        std::mutex m_mutex;
        std::condition_variable m_condition_variable;
    };

    typedef std::shared_ptr<semaphore_t> semaphore_ptr_t;

    semaphore_ptr_t make_semaphore(size_t n);
}

#endif
