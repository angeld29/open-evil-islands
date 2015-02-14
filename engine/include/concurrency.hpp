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

#include "logging.hpp"
#include "untransferable.hpp"

namespace cursedearth
{
    /**
     * @brief thread with interruption support
     */
    class thread_interrupted_t {};

    extern thread_local class thread_interrupt_flag_t
    {
    public:
        thread_interrupt_flag_t(): m_flag(false) {}

        void operator =(bool flag) { m_flag = flag; }
        explicit operator bool() const { return m_flag; }

    private:
        std::atomic<bool> m_flag;
    } g_thread_interrupt_flag;

    inline void interruption_point()
    {
        if (g_thread_interrupt_flag) {
            throw thread_interrupted_t();
        }
    }

    class thread_t: untransferable_t
    {
        struct wrap_t
        {
            std::string token;
            std::function<void ()> function;

            wrap_t(const std::string& token, const std::function<void ()>& function): token(token), function(function) {}

            void operator ()() const
            {
                try {
                    function();
                } catch (const thread_interrupted_t&) {
                    ce_logging_debug("%s: interrupted", token.c_str());
                } catch (const std::exception& error) {
                    ce_logging_fatal("%s: %s", token.c_str(), error.what());
                } catch (...) {
                    ce_logging_fatal("%s: unknown error", token.c_str());
                }
            }
        };

    public:
        template <typename function_t>
        thread_t(function_t function)
        {
            std::promise<thread_interrupt_flag_t*> promise;
            m_thread = std::thread([function, &promise] {
                promise.set_value(&g_thread_interrupt_flag);
                function();
            });
            m_flag = promise.get_future().get();
        }

        template <typename function_t>
        thread_t(const std::string& token, function_t function):
            thread_t(wrap_t(token, function)) {}

        ~thread_t()
        {
            interrupt();
            if (joinable()) {
                join();
            }
        }

        bool joinable() const { return m_thread.joinable(); }
        void join() { m_thread.join(); }
        void detach() { m_thread.detach(); }
        void interrupt() { *m_flag = true; }

    private:
        std::thread m_thread;
        thread_interrupt_flag_t* m_flag;
    };

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
}

#endif
