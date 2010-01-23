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

#include <stddef.h>
#include <stdarg.h>
#include <assert.h>

#include <syslog.h>

#include "logging.h"

static bool opened;

bool logging_open(void)
{
	assert(!opened);
	openlog(NULL, LOG_PERROR, LOG_USER);
	return opened = true;
}

void logging_close(void)
{
	assert(opened);
	closelog();
	opened = false;
}

void logging_set_level(logging_level level)
{
	assert(opened);
	switch (level) {
	case LOGGING_NO_LEVELS:
		setlogmask(LOG_MASK(0x10));
		break;
	case LOGGING_DEBUG_LEVEL:
	case LOGGING_ALL_LEVELS:
		setlogmask(LOG_UPTO(LOG_DEBUG));
		break;
	case LOGGING_INFO_LEVEL:
		setlogmask(LOG_UPTO(LOG_INFO));
		break;
	case LOGGING_WARNING_LEVEL:
		setlogmask(LOG_UPTO(LOG_WARNING));
		break;
	case LOGGING_ERROR_LEVEL:
		setlogmask(LOG_UPTO(LOG_ERR));
		break;
	case LOGGING_CRITICAL_LEVEL:
		setlogmask(LOG_UPTO(LOG_CRIT));
		break;
	case LOGGING_FATAL_LEVEL:
		setlogmask(LOG_UPTO(LOG_ALERT));
		break;
	case LOGGING_WRITE_LEVEL:
		setlogmask(LOG_UPTO(LOG_EMERG));
		break;
	default:
		assert(false);
	};
}

void logging_debug(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	vsyslog(LOG_DEBUG, format, args);
	va_end(args);
}

void logging_info(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	vsyslog(LOG_INFO, format, args);
	va_end(args);
}

void logging_warning(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	vsyslog(LOG_WARNING, format, args);
	va_end(args);
}

void logging_error(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	vsyslog(LOG_ERR, format, args);
	va_end(args);
}

void logging_critical(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	vsyslog(LOG_CRIT, format, args);
	va_end(args);
}

void logging_fatal(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	vsyslog(LOG_ALERT, format, args);
	va_end(args);
}

void logging_write(const char* format, ...)
{
	assert(opened);
	va_list args;
	va_start(args, format);
	vsyslog(LOG_EMERG, format, args);
	va_end(args);
}
