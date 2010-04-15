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
 *  Based on MSDN website.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "cealloc.h"
#include "celogging.h"
#include "ceerror.h"
#include "cetimer.h"

struct ce_timer {
	float frequency_inv;
	LARGE_INTEGER start;
	LARGE_INTEGER stop;
	float diff;
};

static LONGLONG ce_timer_query_frequency(void)
{
	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency)) {
		return frequency.QuadPart;
	}
	ce_error_report_last_windows_error("timer", __func__,
										"QueryPerformanceFrequency failed");
	ce_logging_warning("timer: %s: using default frequency", __func__);
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
	ce_timer* timer = ce_alloc(sizeof(ce_timer));
	timer->frequency_inv = 1.0f / ce_timer_query_frequency();
	ce_timer_query_counter(&timer->start);
	return timer;
}

void ce_timer_del(ce_timer* timer)
{
	ce_free(timer, sizeof(ce_timer));
}

void ce_timer_advance(ce_timer* timer)
{
	ce_timer_query_counter(&timer->stop);
	timer->diff = (timer->stop.QuadPart -
					timer->start.QuadPart) * timer->frequency_inv;
	timer->start = timer->stop;
}

float ce_timer_elapsed(ce_timer* timer)
{
	return timer->diff;
}
