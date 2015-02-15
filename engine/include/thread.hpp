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

#ifndef CE_THREAD_HPP
#define CE_THREAD_HPP

#include "untransferable.hpp"
#include "logging.hpp"

namespace cursedearth
{
    class thread_flag_t;
    extern thread_local thread_flag_t g_thread_flag;

    class thread_interrupted_t {};

    void interruption_point();

    /**
     * @brief thread with interruption support
     */
    class thread_t: untransferable_t
    {
        struct wrap_t
        {
            const std::string token;
            const std::function<void ()> function;

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

        template <typename function_t>
        thread_t(function_t function)
        {
            std::promise<thread_flag_t*> promise;
            m_thread = std::thread([function, &promise] {
                promise.set_value(&g_thread_flag);
                function();
            });
            m_flag = promise.get_future().get();
        }

    public:
        template <typename function_t>
        thread_t(const std::string& token, function_t function):
            thread_t(wrap_t(token, function)) {}

        ~thread_t()
        {
            interrupt();
            join();
        }

        void interrupt();
        void join() { if (m_thread.joinable()) m_thread.join(); }

        std::thread::id id() const { return m_thread.get_id(); }

    private:
        std::thread m_thread;
        thread_flag_t* m_flag = nullptr;
    };

    typedef std::shared_ptr<thread_t> thread_ptr_t;

    template <typename... A>
    inline thread_ptr_t make_thread(A&&... args)
    {
        return std::make_shared<thread_t>(std::forward<A>(args)...);
    }

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
}

#endif
