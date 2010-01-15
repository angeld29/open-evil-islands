#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
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

void logging_open(void)
{
	assert(!opened);
	opened = true;
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
