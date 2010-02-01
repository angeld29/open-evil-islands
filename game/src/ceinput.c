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

#include "celib.h"
#include "ceinput.h"

void ce_input_button_event_init(ce_input_button button,
									ce_input_button_event* event)
{
	event->button = button;
	event->triggered = false;
}

void ce_input_button_event_advance(float elapsed,
									ce_input_button_event* event)
{
	ce_unused(elapsed);
	event->triggered = ce_input_test(event->button);
}

void ce_input_single_front_event_init(ce_input_button button,
										ce_input_single_front_event* event)
{
	event->button = button;
	event->activated = false;
	event->triggered = false;
}

void ce_input_single_front_event_advance(float elapsed,
										ce_input_single_front_event* event)
{
	ce_unused(elapsed);
	bool triggered = ce_input_test(event->button);
	event->triggered = !event->activated && triggered;
	event->activated = triggered;
}

void ce_input_single_back_event_init(ce_input_button button,
										ce_input_single_back_event* event)
{
	event->button = button;
	event->activated = false;
	event->triggered = false;
}

void ce_input_single_back_event_advance(float elapsed,
										ce_input_single_back_event* event)
{
	ce_unused(elapsed);
	bool triggered = ce_input_test(event->button);
	event->triggered = event->activated && !triggered;
	event->activated = triggered;
}
