/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include "systemevent.hpp"

#include <cassert>
#include <csignal>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace cursedearth
{
    std::vector<system_event_handler_t> g_system_event_handlers;

    void emit_signal(system_event_type_t type)
    {
        for (const auto& handler: g_system_event_handlers) {
            handler(type);
        }
    }

    void signal_handler(int type)
    {
        switch (type) {
        case SIGINT:
            emit_signal(system_event_type_t::interrupt);
            break;
        case SIGTERM:
            emit_signal(system_event_type_t::terminate);
            break;
        default:
            assert(false);
        }
    }

#ifdef _WIN32
    BOOL CALLBACK console_handler(DWORD type)
    {
        switch (type) {
        case CTRL_C_EVENT:
            emit_signal(system_event_type_t::ctrlc);
            break;
        case CTRL_BREAK_EVENT:
            emit_signal(system_event_type_t::ctrlbreak);
            break;
        case CTRL_CLOSE_EVENT:
            emit_signal(system_event_type_t::close);
            break;
        case CTRL_LOGOFF_EVENT:
            emit_signal(system_event_type_t::logoff);
            break;
        case CTRL_SHUTDOWN_EVENT:
            emit_signal(system_event_type_t::shutdown);
            break;
        default:
            assert(false);
        }
        return TRUE;
    }
#endif

    void add_system_event_handler(const system_event_handler_t& handler)
    {
        g_system_event_handlers.push_back(handler);

        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);

#ifdef _WIN32
        SetConsoleCtrlHandler(console_handler, TRUE);
#endif
    }
}
