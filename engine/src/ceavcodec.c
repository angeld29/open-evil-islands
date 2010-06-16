/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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
#include <assert.h>

#include <libavcodec/avcodec.h>

#include "celib.h"
#include "celogging.h"
#include "cethread.h"
#include "ceavcodec.h"

static void ce_avcodec_log(void* ptr, int av_level, const char* format, va_list args)
{
	ce_unused(ptr);
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

	char buffer[strlen(format) + 16];
	snprintf(buffer, sizeof(buffer), "avcodec: %s", format);

	ce_logging_report_va(level, buffer, args);
}

static int ce_avcodec_lock(void** mutex, enum AVLockOp op)
{
	switch (op) {
	case AV_LOCK_CREATE:
		*mutex = ce_mutex_new();
		break;
	case AV_LOCK_OBTAIN:
		ce_mutex_lock(*mutex);
		break;
	case AV_LOCK_RELEASE:
		ce_mutex_unlock(*mutex);
		break;
	case AV_LOCK_DESTROY:
		ce_mutex_del(*mutex);
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
