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

#include <sys/time.h>
#include <pthread.h>

#include "alloc.hpp"
#include "timer.hpp"

namespace cursedearth
{
    struct timer_t
    {
        float elapsed;
        struct timeval start;
        struct timeval stop;
    };

    timer_t* ce_timer_new(void)
    {
        return (timer_t*)ce_alloc(sizeof(timer_t));
    }

    void ce_timer_del(timer_t* timer)
    {
        ce_free(timer, sizeof(timer_t));
    }

    void ce_timer_start(timer_t* timer)
    {
        gettimeofday(&timer->start, NULL);
    }

    float ce_timer_advance(timer_t* timer)
    {
        gettimeofday(&timer->stop, NULL);

        struct timeval diff = {
            timer->stop.tv_sec - timer->start.tv_sec,
            timer->stop.tv_usec - timer->start.tv_usec,
        };

        if (diff.tv_usec < 0) {
            --diff.tv_sec;
            diff.tv_usec += 1000000;
        }

        timer->elapsed = diff.tv_sec + diff.tv_usec * 1e-6f;
        timer->start = timer->stop;

        return timer->elapsed;
    }

    float ce_timer_elapsed(timer_t* timer)
    {
        return timer->elapsed;
    }
}
