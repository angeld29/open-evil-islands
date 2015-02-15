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

#ifndef CE_THREADPOOL_HPP
#define CE_THREADPOOL_HPP

#include "commonheaders.hpp"

namespace cursedearth
{
    /**
     * @brief the thread pool class manages a collection of threads
     *        it's a thread pool pattern implementation
     *        all functions are thread-safe
     */
    class thread_pool_t final: public singleton_t<thread_pool_t>
    {
        typedef std::function<void ()> task_t;

    public:
        thread_pool_t();
        ~thread_pool_t();

        void enqueue(const task_t&);

    private:
        void execute();

    private:
        size_t m_idle_thread_count;
        std::mutex m_mutex;
        condition_variable_t m_idle;
        std::vector<task_t> m_tasks;
        std::vector<thread_ptr_t> m_threads;
    };

    typedef std::unique_ptr<thread_pool_t> thread_pool_ptr_t;

    thread_pool_ptr_t make_thread_pool();
}

#endif
