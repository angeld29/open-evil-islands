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

#include "cealloc.h"
#include "ceevent.h"

ce_event* ce_event_new(void (*notify)(ce_event*), size_t size)
{
	ce_event* event = ce_alloc_zero(sizeof(ce_event) + size);
	event->notify = notify;
	event->size = size;
	return event;
}

void ce_event_del(ce_event* event)
{
	if (NULL != event) {
		ce_free(event, sizeof(ce_event) + event->size);
	}
}
