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

#ifndef CE_CONDITIONVARIABLE_HPP
#define CE_CONDITIONVARIABLE_HPP

#include "thread.hpp"

#include <condition_variable>

namespace cursedearth
{
    class condition_variable_t: untransferable_t
    {
    public:
        template <class lock_t>
        void wait(lock_t& lock)
        {
            interruption_point();
            m_condition_variable.wait(lock);
            interruption_point();
        }

        void notify_one() { m_condition_variable.notify_one(); }
        void notify_all() { m_condition_variable.notify_all(); }

    private:
        std::condition_variable_any m_condition_variable;
    };

    typedef std::shared_ptr<condition_variable_t> condition_variable_ptr_t;
    typedef std::weak_ptr<condition_variable_t> condition_variable_weak_ptr_t;

    inline condition_variable_ptr_t make_condition_variable()
    {
        return std::make_shared<condition_variable_t>();
    }
}

#endif
