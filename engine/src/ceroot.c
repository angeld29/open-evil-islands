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

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"

struct ce_root ce_root;

bool ce_root_init(const char* ei_path)
{
	assert(!ce_root.inited && "the root subsystem has already been inited");

	ce_alloc_init();
	ce_logging_init();

	ce_root.renderwindow = ce_renderwindow_new();
	ce_root.rendersystem = ce_rendersystem_new();
	ce_root.scenemng = ce_scenemng_new(ei_path);

	ce_input_init();

	return ce_root.inited = true;
}

void ce_root_term(void)
{
	assert(ce_root.inited && "the root subsystem has not yet been inited");
	ce_root.inited = false;

	ce_input_term();

	ce_scenemng_del(ce_root.scenemng), ce_root.scenemng = NULL;
	ce_rendersystem_del(ce_root.rendersystem), ce_root.rendersystem = NULL;
	ce_renderwindow_del(ce_root.renderwindow), ce_root.renderwindow = NULL;

	ce_logging_term();
	ce_alloc_term();
}

static bool ce_root_done;

static void ce_root_exit(void)
{
	ce_logging_write("root: exiting sanely...");
	ce_root_done = true;
}

static void ce_root_signal_handler(int type)
{
	switch (type) {
	case SIGINT:
		ce_logging_warning("root: interactive attention signal received");
		break;
	case SIGTERM:
		ce_logging_warning("root: termination signal received");
		break;
	default:
		ce_logging_critical("root: unknown signal received (%d)", type);
		assert(false);
	}
	ce_root_exit();
}

// FIXME: refactoring needed
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static BOOL CALLBACK ce_root_console_handler(DWORD type)
{
	switch (type) {
	case CTRL_C_EVENT:
		ce_logging_warning("root: Ctrl-C event received");
		break;
	case CTRL_BREAK_EVENT:
		ce_logging_warning("root: Ctrl-Break event received");
		break;
	case CTRL_CLOSE_EVENT:
		ce_logging_warning("root: Ctrl-Close event received");
		break;
	case CTRL_LOGOFF_EVENT:
		ce_logging_warning("root: Ctrl-Logoff event received");
		break;
	case CTRL_SHUTDOWN_EVENT:
		ce_logging_warning("root: Ctrl-Shutdown event received");
		break;
	default:
		ce_logging_critical("root: unknown event received (%u)", type);
		assert(false);
	}
	ce_root_exit();
	return TRUE;
}
#endif

void ce_root_exec(void)
{
	assert(ce_root.inited && "the root subsystem has not yet been inited");

	signal(SIGINT, ce_root_signal_handler);
	signal(SIGTERM, ce_root_signal_handler);

#ifdef _WIN32
	SetConsoleCtrlHandler(ce_root_console_handler, TRUE);
#endif

	while (!ce_root_done && ce_renderwindow_pump(ce_root.renderwindow)) {
		ce_scenemng_advance(ce_root.scenemng);
		ce_scenemng_render(ce_root.scenemng);
		ce_renderwindow_swap(ce_root.renderwindow);
	}
}
