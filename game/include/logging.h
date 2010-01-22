#ifndef CE_LOGGING_H
#define CE_LOGGING_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	LOGGING_NO_LEVELS,
	LOGGING_DEBUG_LEVEL,
	LOGGING_INFO_LEVEL,
	LOGGING_WARNING_LEVEL,
	LOGGING_ERROR_LEVEL,
	LOGGING_CRITICAL_LEVEL,
	LOGGING_FATAL_LEVEL,
	LOGGING_WRITE_LEVEL,
	LOGGING_ALL_LEVELS
} logging_level;

extern bool logging_open(void);
extern void logging_close(void);

extern void logging_set_level(logging_level level);

extern void logging_debug(const char* format, ...);
extern void logging_info(const char* format, ...);
extern void logging_warning(const char* format, ...);
extern void logging_error(const char* format, ...);
extern void logging_critical(const char* format, ...);
extern void logging_fatal(const char* format, ...);
extern void logging_write(const char* format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_LOGGING_H */
