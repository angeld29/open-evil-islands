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

#include "timer.hpp"

#include <sys/time.h>

namespace cursedearth
{
    class posix_timer_t final: public timer_t
    {
    public:
        virtual void start() final
        {
            gettimeofday(&m_start, NULL);
        }

        virtual float advance() final
        {
            gettimeofday(&m_stop, NULL);

            timeval diff = {
                m_stop.tv_sec - m_start.tv_sec,
                m_stop.tv_usec - m_start.tv_usec,
            };

            if (diff.tv_usec < 0) {
                --diff.tv_sec;
                diff.tv_usec += 1000000;
            }

            m_elapsed = diff.tv_sec + diff.tv_usec * 1e-6f;
            m_start = m_stop;

            return m_elapsed;
        }

    private:
        timeval m_start;
        timeval m_stop;
    };

    timer_ptr_t make_timer()
    {
        return std::make_shared<posix_timer_t>();
    }
}
