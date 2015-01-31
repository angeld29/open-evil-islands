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

#include <ctime>

#include "alloc.hpp"
#include "timer.hpp"

namespace cursedearth
{
static const float CE_TIMER_CLOCKS_PER_SEC_INV = 1.0f / CLOCKS_PER_SEC;

typedef struct {
    clock_t start;
    clock_t stop;
} ce_timer_std;

ce_timer* ce_timer_new(void)
{
    return ce_alloc(sizeof(ce_timer) + sizeof(ce_timer_std));
}

void ce_timer_del(ce_timer* timer)
{
    ce_free(timer, sizeof(ce_timer) + sizeof(ce_timer_std));
}

void ce_timer_start(ce_timer* timer)
{
    ce_timer_std* std_timer = (ce_timer_std*)timer->impl;
    std_timer->start = clock();
}

float ce_timer_advance(ce_timer* timer)
{
    ce_timer_std* std_timer = (ce_timer_std*)timer->impl;
    std_timer->stop = clock();
    timer->elapsed = (std_timer->stop - std_timer->start) * CE_TIMER_CLOCKS_PER_SEC_INV;
    std_timer->start = std_timer->stop;
    return timer->elapsed;
}
}
