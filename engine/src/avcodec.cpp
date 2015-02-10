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

#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "logging.hpp"
#include "thread.hpp"
#include "avcodec.hpp"

namespace cursedearth
{
    std::mutex g_avcodec_mutex;

    int avcodec_lock(void**, enum AVLockOp operation)
    {
        switch (operation) {
        case AV_LOCK_OBTAIN:
            g_avcodec_mutex.lock();
            break;
        case AV_LOCK_RELEASE:
            g_avcodec_mutex.unlock();
            break;
        case AV_LOCK_CREATE:
        case AV_LOCK_DESTROY:
            // do nothing
            break;
        }
        return 0;
    }

    void avcodec_log(void*, int av_level, const char* format, va_list args)
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
        case AV_LOG_DEBUG:
            level = CE_LOGGING_LEVEL_DEBUG;
            break;
        case AV_LOG_INFO:
        default:
            level = CE_LOGGING_LEVEL_INFO;
        }
        const std::string message = std::string("avcodec: ") + format;
        ce_logging_report_va(level, message.c_str(), args);
    }

    void initialize_avcodec()
    {
        av_lockmgr_register(avcodec_lock);
        av_log_set_callback(avcodec_log);
        avcodec_init();
        avcodec_register_all();
    }

    void terminate_avcodec()
    {
        av_log_set_callback(av_log_default_callback);
        av_lockmgr_register(NULL);
    }
}
