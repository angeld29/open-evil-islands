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
#include <string.h>
#include <assert.h>

#include "celogging.h"

static struct {
	ce_logging_level level;
	const char* level_names[CE_LOGGING_LEVEL_ALL];
} ce_logging_context = {
#ifdef NDEBUG
	.level = CE_LOGGING_LEVEL_INFO,
#else
	.level = CE_LOGGING_LEVEL_DEBUG,
#endif
	.level_names = {
		"UNUSED",
		"DEBUG",
		"INFO",
		"WARNING",
		"ERROR",
		"CRITICAL",
		"FATAL",
		"WRITE",
	},
};

static void ce_logging_report(ce_logging_level level, const char* format, va_list args)
{
	if (CE_LOGGING_LEVEL_NONE != ce_logging_context.level &&
			(level >= ce_logging_context.level ||
			CE_LOGGING_LEVEL_ALL == ce_logging_context.level)) {
		// FIXME: threads!
		fprintf(stderr, "%s: ", ce_logging_context.level_names[level]);
		vfprintf(stderr, format, args);
		if (0 == strlen(format) || '\n' != format[strlen(format) - 1]) {
			putc('\n', stderr);
		}
		fflush(stderr);
	}
}

void ce_logging_set_level(ce_logging_level level)
{
	ce_logging_context.level = level;
}

#define CE_LOGGING_PROC(name, level) \
void ce_logging_##name(const char* format, ...) \
{ \
	va_list args; \
	va_start(args, format); \
	ce_logging_report(level, format, args); \
	va_end(args); \
}

CE_LOGGING_PROC(debug, CE_LOGGING_LEVEL_DEBUG)
CE_LOGGING_PROC(info, CE_LOGGING_LEVEL_INFO)
CE_LOGGING_PROC(warning, CE_LOGGING_LEVEL_WARNING)
CE_LOGGING_PROC(error, CE_LOGGING_LEVEL_ERROR)
CE_LOGGING_PROC(critical, CE_LOGGING_LEVEL_CRITICAL)
CE_LOGGING_PROC(fatal, CE_LOGGING_LEVEL_FATAL)
CE_LOGGING_PROC(write, CE_LOGGING_LEVEL_WRITE)
