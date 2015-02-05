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

#include <cstdio>
#include <cstring>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "logging.hpp"
#include "thread.hpp"
#include "avcodec.hpp"

namespace cursedearth
{
    void ce_avcodec_log(void*, int av_level, const char* format, va_list args)
    {
        ce_logging_level level;

        switch (av_level) {
        case AV_LOG_PANIC:
        case AV_LOG_FATAL:
            level = CE_LOGGING_LEVEL_FATAL;
            break;
        case AV_LOG_ERROR:
            level = CE_LOGGING_LEVEL_ERROR;
            break;
        case AV_LOG_WARNING:
            level = CE_LOGGING_LEVEL_WARNING;
            break;
        case AV_LOG_INFO:
            level = CE_LOGGING_LEVEL_INFO;
            break;
        case AV_LOG_DEBUG:
            level = CE_LOGGING_LEVEL_DEBUG;
            break;
        default:
            level = CE_LOGGING_LEVEL_WRITE;
            break;
        }

        std::vector<char> buffer(strlen(format) + 16);
        snprintf(buffer.data(), buffer.size(), "avcodec: %s", format);

        ce_logging_report_va(level, buffer.data(), args);
    }

    int ce_avcodec_lock(void** mutex, enum AVLockOp op)
    {
        switch (op) {
        case AV_LOCK_CREATE:
            *mutex = ce_mutex_new();
            break;
        case AV_LOCK_OBTAIN:
            ce_mutex_lock((ce_mutex*)*mutex);
            break;
        case AV_LOCK_RELEASE:
            ce_mutex_unlock((ce_mutex*)*mutex);
            break;
        case AV_LOCK_DESTROY:
            ce_mutex_del((ce_mutex*)*mutex);
            break;
        }

        return 0;
    }

    void ce_avcodec_init(void)
    {
        av_log_set_callback(ce_avcodec_log);
        av_lockmgr_register(ce_avcodec_lock);
        avcodec_init();
        avcodec_register_all();
    }

    void ce_avcodec_term(void)
    {
        av_lockmgr_register(NULL);
        av_log_set_callback(av_log_default_callback);
    }
}
