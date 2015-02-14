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

#include <windows.h>

namespace cursedearth
{
    class performance_timer_t final: public timer_t
    {
        static LONGLONG query_frequency()
        {
            LARGE_INTEGER frequency;
            if (QueryPerformanceFrequency(&frequency)) {
                return frequency.QuadPart;
            }
            ce_logging_warning("timer: using default frequency");
            return 1000000ll;
        }

        static void query_counter(LARGE_INTEGER& value)
        {
            DWORD_PTR old_mask = SetThreadAffinityMask(GetCurrentThread(), 0);
            QueryPerformanceCounter(&value);
            SetThreadAffinityMask(GetCurrentThread(), old_mask);
        }

    public:
        performance_timer_t(): m_frequency_inv(1.0f / query_frequency()) {}

        virtual void start() final
        {
            query_counter(m_start);
        }

        virtual float advance() final
        {
            query_counter(m_stop);
            m_elapsed = (m_stop.QuadPart - m_start.QuadPart) * m_frequency_inv;
            m_start = m_stop;
            return m_elapsed;
        }

    private:
        const float m_frequency_inv;
        LARGE_INTEGER m_start;
        LARGE_INTEGER m_stop;
    };

    timer_ptr_t make_timer()
    {
        return std::make_shared<performance_timer_t>();
    }
}
