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

#include "celogging.h"

static bool opened;
static ce_logging_level level = CE_LOGGING_LEVEL_NONE;

static const char* level_names[CE_LOGGING_LEVEL_ALL] = {
	"UNUSED",
	"DEBUG",
	"INFO",
	"WARNING",
	"ERROR",
	"CRITICAL",
	"FATAL",
	"WRITE"
};

static void report(ce_logging_level lvl, const char* format, va_list args)
{
	if (CE_LOGGING_LEVEL_NONE != level &&
			(lvl >= level || CE_LOGGING_LEVEL_ALL == level)) {
		fprintf(stderr, "%s: ", level_names[lvl]);
		vfprintf(stderr, format, args);
		putc('\n', stderr);
	}
}

bool ce_logging_open(void)
{
	assert(!opened);
	return opened = true;
}

void ce_logging_close(void)
{
	assert(opened);
	opened = false;
}

void ce_logging_set_level(ce_logging_level lvl)
{
	assert(opened);
	level = lvl;
}

void ce_logging_debug(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(CE_LOGGING_LEVEL_DEBUG, format, args);
	va_end(args);
}

void ce_logging_info(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(CE_LOGGING_LEVEL_INFO, format, args);
	va_end(args);
}

void ce_logging_warning(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(CE_LOGGING_LEVEL_WARNING, format, args);
	va_end(args);
}

void ce_logging_error(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(CE_LOGGING_LEVEL_ERROR, format, args);
	va_end(args);
}

void ce_logging_critical(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(CE_LOGGING_LEVEL_CRITICAL, format, args);
	va_end(args);
}

void ce_logging_fatal(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(CE_LOGGING_LEVEL_FATAL, format, args);
	va_end(args);
}

void ce_logging_write(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	report(CE_LOGGING_LEVEL_WRITE, format, args);
	va_end(args);
}