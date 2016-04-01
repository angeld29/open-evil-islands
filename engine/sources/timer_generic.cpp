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

#include "timer.hpp"

#include <ctime>

namespace cursedearth
{
    class standard_timer_t final: public timer_t
    {
    public:
        standard_timer_t(): m_clocks_per_sec_inv(1.0f / CLOCKS_PER_SEC) {}

        virtual void start() final
        {
            m_start = clock();
        }

        virtual float advance() final
        {
            m_stop = clock();
            m_elapsed = (m_stop - m_start) * m_clocks_per_sec_inv;
            m_start = m_stop;
            return m_elapsed;
        }

    private:
        const float m_clocks_per_sec_inv;
        clock_t m_start;
        clock_t m_stop;
    };

    timer_ptr_t make_timer()
    {
        return std::make_shared<standard_timer_t>();
    }
}
