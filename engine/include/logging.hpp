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

#ifndef CE_LOGGING_HPP
#define CE_LOGGING_HPP

#include <cstdarg>

#include <boost/format.hpp>

#define CE_LOGGING_DEF_PROC_VA(name, level) \
inline void ce_logging_##name##_va(const char* format, va_list args) \
{ \
    ce_logging_report_va(level, format, args); \
}

namespace cursedearth
{
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

    void ce_logging_set_level(ce_logging_level level);

    void ce_logging_report(ce_logging_level level, const char* format, ...);
    void ce_logging_report_va(ce_logging_level level, const char* format, va_list args);

    void ce_logging_debug(const char* format, ...);
    void ce_logging_info(const char* format, ...);
    void ce_logging_warning(const char* format, ...);
    void ce_logging_error(const char* format, ...);
    void ce_logging_critical(const char* format, ...);
    void ce_logging_fatal(const char* format, ...);
    void ce_logging_write(const char* format, ...);

    CE_LOGGING_DEF_PROC_VA(debug, CE_LOGGING_LEVEL_DEBUG)
    CE_LOGGING_DEF_PROC_VA(info, CE_LOGGING_LEVEL_INFO)
    CE_LOGGING_DEF_PROC_VA(warning, CE_LOGGING_LEVEL_WARNING)
    CE_LOGGING_DEF_PROC_VA(error, CE_LOGGING_LEVEL_ERROR)
    CE_LOGGING_DEF_PROC_VA(critical, CE_LOGGING_LEVEL_CRITICAL)
    CE_LOGGING_DEF_PROC_VA(fatal, CE_LOGGING_LEVEL_FATAL)
    CE_LOGGING_DEF_PROC_VA(write, CE_LOGGING_LEVEL_WRITE)
}

#undef CE_LOGGING_DEF_PROC_VA

#endif
