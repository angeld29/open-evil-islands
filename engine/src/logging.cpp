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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "logging.hpp"

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

void ce_logging_set_level(ce_logging_level level)
{
    ce_logging_context.level = level;
}

void ce_logging_report(ce_logging_level level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    ce_logging_report_va(level, format, args);
    va_end(args);
}

void ce_logging_report_va(ce_logging_level level, const char* format, va_list args)
{
    if (CE_LOGGING_LEVEL_NONE != ce_logging_context.level &&
            (level >= ce_logging_context.level ||
            CE_LOGGING_LEVEL_ALL == ce_logging_context.level)) {
        size_t length = strlen(format);
        char buffer[length + 16];
        snprintf(buffer, sizeof(buffer), "%s: %s%c",
            ce_logging_context.level_names[level], format,
            " \n"[0 == length || '\n' != format[length - 1]]);
        vfprintf(stderr, buffer, args);
        fflush(stderr);
    }
}

#define CE_LOGGING_DEF_PROC(name, level) \
void ce_logging_##name(const char* format, ...) \
{ \
    va_list args; \
    va_start(args, format); \
    ce_logging_report_va(level, format, args); \
    va_end(args); \
}

CE_LOGGING_DEF_PROC(debug, CE_LOGGING_LEVEL_DEBUG)
CE_LOGGING_DEF_PROC(info, CE_LOGGING_LEVEL_INFO)
CE_LOGGING_DEF_PROC(warning, CE_LOGGING_LEVEL_WARNING)
CE_LOGGING_DEF_PROC(error, CE_LOGGING_LEVEL_ERROR)
CE_LOGGING_DEF_PROC(critical, CE_LOGGING_LEVEL_CRITICAL)
CE_LOGGING_DEF_PROC(fatal, CE_LOGGING_LEVEL_FATAL)
CE_LOGGING_DEF_PROC(write, CE_LOGGING_LEVEL_WRITE)
