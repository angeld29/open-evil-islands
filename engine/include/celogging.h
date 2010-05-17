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

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CE_LOGGING_LEVEL_NONE,
	CE_LOGGING_LEVEL_DEBUG,
	CE_LOGGING_LEVEL_INFO,
	CE_LOGGING_LEVEL_WARNING,
	CE_LOGGING_LEVEL_ERROR,
	CE_LOGGING_LEVEL_CRITICAL,
	CE_LOGGING_LEVEL_FATAL,
	CE_LOGGING_LEVEL_WRITE,
	CE_LOGGING_LEVEL_ALL
} ce_logging_level;

extern void ce_logging_set_level(ce_logging_level level);

extern void ce_logging_debug(const char* format, ...);
extern void ce_logging_info(const char* format, ...);
extern void ce_logging_warning(const char* format, ...);
extern void ce_logging_error(const char* format, ...);
extern void ce_logging_critical(const char* format, ...);
extern void ce_logging_fatal(const char* format, ...);
extern void ce_logging_write(const char* format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_LOGGING_H */
