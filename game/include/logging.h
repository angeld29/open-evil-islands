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
