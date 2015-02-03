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

#ifndef CE_SYSTEMEVENT_HPP
#define CE_SYSTEMEVENT_HPP

namespace cursedearth
{
    typedef enum {
        CE_SYSTEM_EVENT_TYPE_INT,
        CE_SYSTEM_EVENT_TYPE_TERM,
        CE_SYSTEM_EVENT_TYPE_CTRLC,
        CE_SYSTEM_EVENT_TYPE_CTRLBREAK,
        CE_SYSTEM_EVENT_TYPE_CLOSE,
        CE_SYSTEM_EVENT_TYPE_LOGOFF,
        CE_SYSTEM_EVENT_TYPE_SHUTDOWN,
        CE_SYSTEM_EVENT_TYPE_COUNT
    } ce_system_event_type;

    extern void ce_system_event_register(void (*handler)(ce_system_event_type type));
}

#endif
