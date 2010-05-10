/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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

/*
 *  Based on MSDN website
*/

#include <windows.h>

#include "cealloc.h"
#include "celogging.h"
#include "cetimer.h"

#include "ceerror_win32.h"

typedef struct {
	float frequency_inv;
	LARGE_INTEGER start;
	LARGE_INTEGER stop;
} ce_timer_win;

static LONGLONG ce_timer_query_frequency(void)
{
	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency)) {
		return frequency.QuadPart;
	}
	ce_error_report_windows_last("timer");
	ce_logging_warning("timer: using default frequency");
	return 1000000;
}

static void ce_timer_query_counter(LARGE_INTEGER* value)
{
	DWORD_PTR old_mask = SetThreadAffinityMask(GetCurrentThread(), 0);
	QueryPerformanceCounter(value);
	SetThreadAffinityMask(GetCurrentThread(), old_mask);
}

ce_timer* ce_timer_new(void)
{
	ce_timer* timer = ce_alloc(sizeof(ce_timer) + sizeof(ce_timer_win));
	ce_timer_win* win_timer = (ce_timer_win*)timer->impl;
	win_timer->frequency_inv = 1.0f / ce_timer_query_frequency();
	return timer;
}

void ce_timer_del(ce_timer* timer)
{
	ce_free(timer, sizeof(ce_timer) + sizeof(ce_timer_win));
}

void ce_timer_start(ce_timer* timer)
{
	ce_timer_win* win_timer = (ce_timer_win*)timer->impl;
	ce_timer_query_counter(&win_timer->start);
}

float ce_timer_advance(ce_timer* timer)
{
	ce_timer_win* win_timer = (ce_timer_win*)timer->impl;
	ce_timer_query_counter(&win_timer->stop);
	timer->elapsed = (win_timer->stop.QuadPart -
					win_timer->start.QuadPart) * win_timer->frequency_inv;
	win_timer->start = win_timer->stop;
	return timer->elapsed;
}
