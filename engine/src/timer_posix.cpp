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
    struct ce_timer
    {
        float elapsed;
        struct timeval start;
        struct timeval stop;
    };

    ce_timer* ce_timer_new(void)
    {
        return (ce_timer*)ce_alloc(sizeof(ce_timer));
    }

    void ce_timer_del(ce_timer* timer)
    {
        ce_free(timer, sizeof(ce_timer));
    }

    void ce_timer_start(ce_timer* timer)
    {
        gettimeofday(&timer->start, NULL);
    }

    float ce_timer_advance(ce_timer* timer)
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

    float ce_timer_elapsed(ce_timer* timer)
    {
        return timer->elapsed;
    }
}
