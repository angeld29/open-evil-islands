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

#ifndef CE_SEMAPHORE_HPP
#define CE_SEMAPHORE_HPP

#include "commonheaders.hpp"

namespace cursedearth
{
    /**
     * @brief the semaphore class provides a general counting semaphore
     */
    class semaphore_t: untransferable_t
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
        condition_variable_t m_condition_variable;
    };

    typedef std::shared_ptr<semaphore_t> semaphore_ptr_t;

    semaphore_ptr_t make_semaphore(size_t n);
}

#endif
