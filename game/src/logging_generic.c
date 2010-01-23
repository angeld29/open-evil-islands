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

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "logging.h"

static bool opened;
static logging_level level = LOGGING_NO_LEVELS;

static void report(logging_level lvl, const char* format, va_list args)
{
	if (LOGGING_NO_LEVELS != level &&
			(lvl >= level || LOGGING_ALL_LEVELS == level)) {
		vfprintf(stderr, format, args);
		fprintf(stderr, "\n");
	}
}

bool logging_open(void)
{
	assert(!opened);
	return opened = true;
}

void logging_close(void)
{
	assert(opened);
	opened = false;
}

void logging_set_level(logging_level lvl)
{
	assert(opened);
	level = lvl;
}

void logging_debug(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(LOGGING_DEBUG_LEVEL, format, args);
	va_end(args);
}

void logging_info(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(LOGGING_INFO_LEVEL, format, args);
	va_end(args);
}

void logging_warning(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(LOGGING_WARNING_LEVEL, format, args);
	va_end(args);
}

void logging_error(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(LOGGING_ERROR_LEVEL, format, args);
	va_end(args);
}

void logging_critical(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(LOGGING_CRITICAL_LEVEL, format, args);
	va_end(args);
}

void logging_fatal(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(LOGGING_FATAL_LEVEL, format, args);
	va_end(args);
}

void logging_write(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(LOGGING_WRITE_LEVEL, format, args);
	va_end(args);
}
