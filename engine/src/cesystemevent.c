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
#include <stdbool.h>
#include <signal.h>
#include <assert.h>

#if defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "cesystemevent.h"

static void (*ce_systemevent_handler)(ce_systemevent_type type);

static void ce_systemevent_signal_handler(int type)
{
	switch (type) {
	case SIGINT:
		ce_systemevent_handler(CE_SYSTEMEVENT_TYPE_INT);
		break;
	case SIGTERM:
		ce_systemevent_handler(CE_SYSTEMEVENT_TYPE_TERM);
		break;
	default:
		assert(false);
	}
}

#if defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32)
static BOOL CALLBACK ce_systemevent_console_handler(DWORD type)
{
	switch (type) {
	case CTRL_C_EVENT:
		ce_systemevent_handler(CE_SYSTEMEVENT_TYPE_CTRLC);
		break;
	case CTRL_BREAK_EVENT:
		ce_systemevent_handler(CE_SYSTEMEVENT_TYPE_CTRLBREAK);
		break;
	case CTRL_CLOSE_EVENT:
		ce_systemevent_handler(CE_SYSTEMEVENT_TYPE_CLOSE);
		break;
	case CTRL_LOGOFF_EVENT:
		ce_systemevent_handler(CE_SYSTEMEVENT_TYPE_LOGOFF);
		break;
	case CTRL_SHUTDOWN_EVENT:
		ce_systemevent_handler(CE_SYSTEMEVENT_TYPE_SHUTDOWN);
		break;
	default:
		assert(false);
	}
	return TRUE;
}
#endif

void ce_systemevent_register(void (*handler)(ce_systemevent_type type))
{
	assert(NULL == ce_systemevent_handler && "only one handler supported");
	ce_systemevent_handler = handler;

	signal(SIGINT, ce_systemevent_signal_handler);
	signal(SIGTERM, ce_systemevent_signal_handler);

#if defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32)
	SetConsoleCtrlHandler(ce_systemevent_console_handler, TRUE);
#endif
}