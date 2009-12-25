#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>

#include <syslog.h>

#include "logging.h"

void logging_open(void)
{
	openlog(NULL, LOG_PERROR, LOG_USER);
}

void logging_close(void)
{
	closelog();
}

void logging_set_level(logging_level level)
{
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
	va_list args;
	va_start(args, format);
	vsyslog(LOG_DEBUG, format, args);
	va_end(args);
}

void logging_info(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsyslog(LOG_INFO, format, args);
	va_end(args);
}

void logging_warning(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsyslog(LOG_WARNING, format, args);
	va_end(args);
}

void logging_error(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsyslog(LOG_ERR, format, args);
	va_end(args);
}

void logging_critical(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsyslog(LOG_CRIT, format, args);
	va_end(args);
}

void logging_fatal(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsyslog(LOG_ALERT, format, args);
	va_end(args);
}

void logging_write(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsyslog(LOG_EMERG, format, args);
	va_end(args);
}
