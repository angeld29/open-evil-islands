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

#include <string.h>

#include "celib.h"
#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceinput.h"

// level 0 input API implementation

bool ce_input_buttons[CE_IB_COUNT];
ce_vec2 ce_input_mouse_offset;

// level 1 input API implementation

ce_input_event_supply* ce_input_event_supply_new(void)
{
	ce_input_event_supply* supply = ce_alloc(sizeof(ce_input_event_supply));
	supply->events = ce_vector_new();
	return supply;
}

void ce_input_event_supply_del(ce_input_event_supply* supply)
{
	if (NULL != supply) {
		for (int i = 0; i < supply->events->count; ++i) {
			ce_input_event* event = supply->events->items[i];
			if (NULL != event->vtable.dtor) {
				(*event->vtable.dtor)(event);
			}
			ce_free(event, sizeof(ce_input_event) + event->size);
		}
		ce_vector_del(supply->events);
		ce_free(supply, sizeof(ce_input_event_supply));
	}
}

void ce_input_event_supply_advance(ce_input_event_supply* supply, float elapsed)
{
	for (int i = 0; i < supply->events->count; ++i) {
		ce_input_event* event = supply->events->items[i];
		(*event->vtable.advance)(event, elapsed);
	}
}

static ce_input_event* ce_input_create_event(ce_input_event_supply* supply,
	ce_input_event_vtable vtable, size_t size, ...)
{
	ce_input_event* event = ce_alloc(sizeof(ce_input_event) + size);

	event->triggered = false;
	event->vtable = vtable;
	event->size = size;

	va_list args;
	va_start(args, size);
	(*vtable.ctor)(event, args);
	va_end(args);

	ce_vector_push_back(supply->events, event);
	return event;
}

// Button event

typedef struct {
	ce_input_button button;
} ce_input_event_button;

static void ce_input_event_button_ctor(ce_input_event* event, va_list args)
{
	ce_input_event_button* button_event = (ce_input_event_button*)event->impl;
	button_event->button = va_arg(args, ce_input_button);
}

static void ce_input_event_button_advance(ce_input_event* event, float elapsed)
{
	ce_unused(elapsed);
	ce_input_event_button* button_event = (ce_input_event_button*)event->impl;
	event->triggered = ce_input_buttons[button_event->button];
}

static const ce_input_event_vtable ce_input_event_button_vtable = {
	ce_input_event_button_ctor, NULL, ce_input_event_button_advance
};

ce_input_event* ce_input_event_supply_button_event(ce_input_event_supply* supply,
													ce_input_button button)
{
	return ce_input_create_event(supply, ce_input_event_button_vtable,
									sizeof(ce_input_event_button), button);
}

// Single Front event

typedef struct {
	ce_input_event* event;
	bool activated;
} ce_input_event_single_front;

static void ce_input_event_single_front_ctor(ce_input_event* event, va_list args)
{
	ce_input_event_single_front* single_front_event =
		(ce_input_event_single_front*)event->impl;
	single_front_event->event = va_arg(args, ce_input_event*);
	single_front_event->activated = false;
}

static void ce_input_event_single_front_advance(ce_input_event* event, float elapsed)
{
	ce_unused(elapsed);
	ce_input_event_single_front* single_front_event =
		(ce_input_event_single_front*)event->impl;
	event->triggered = !single_front_event->activated &&
						single_front_event->event->triggered;
	single_front_event->activated = single_front_event->event->triggered;
}

static const ce_input_event_vtable ce_input_event_single_front_vtable = {
	ce_input_event_single_front_ctor, NULL, ce_input_event_single_front_advance
};

ce_input_event*
ce_input_event_supply_single_front_event(ce_input_event_supply* supply,
											ce_input_event* event)
{
	return ce_input_create_event(supply, ce_input_event_single_front_vtable,
									sizeof(ce_input_event_single_front), event);
}

// Single Back event

typedef struct {
	ce_input_event* event;
	bool activated;
} ce_input_event_single_back;

static void ce_input_event_single_back_ctor(ce_input_event* event, va_list args)
{
	ce_input_event_single_back* single_back_event =
		(ce_input_event_single_back*)event->impl;
	single_back_event->event = va_arg(args, ce_input_event*);
	single_back_event->activated = false;
}

static void ce_input_event_single_back_advance(ce_input_event* event, float elapsed)
{
	ce_unused(elapsed);
	ce_input_event_single_back* single_back_event =
		(ce_input_event_single_back*)event->impl;
	event->triggered = single_back_event->activated &&
						!single_back_event->event->triggered;
	single_back_event->activated = single_back_event->event->triggered;
}

static const ce_input_event_vtable ce_input_event_single_back_vtable = {
	ce_input_event_single_back_ctor, NULL, ce_input_event_single_back_advance
};

ce_input_event*
ce_input_event_supply_single_back_event(ce_input_event_supply* supply,
											ce_input_event* event)
{
	return ce_input_create_event(supply, ce_input_event_single_back_vtable,
									sizeof(ce_input_event_single_back), event);
}

// AND event

typedef struct {
	ce_input_event* event1;
	ce_input_event* event2;
} ce_input_event_and;

static void ce_input_event_and_ctor(ce_input_event* event, va_list args)
{
	ce_input_event_and* and_event = (ce_input_event_and*)event->impl;
	and_event->event1 = va_arg(args, ce_input_event*);
	and_event->event2 = va_arg(args, ce_input_event*);
}

static void ce_input_event_and_advance(ce_input_event* event, float elapsed)
{
	ce_unused(elapsed);
	ce_input_event_and* and_event = (ce_input_event_and*)event->impl;
	event->triggered = and_event->event1->triggered &&
						and_event->event2->triggered;
}

static const ce_input_event_vtable ce_input_event_and_vtable = {
	ce_input_event_and_ctor, NULL, ce_input_event_and_advance
};

ce_input_event* ce_input_event_supply_and2_event(ce_input_event_supply* supply,
													ce_input_event* event1,
													ce_input_event* event2)
{
	return ce_input_create_event(supply, ce_input_event_and_vtable,
									sizeof(ce_input_event_and), event1, event2);
}

ce_input_event* ce_input_event_supply_and3_event(ce_input_event_supply* supply,
													ce_input_event* event1,
													ce_input_event* event2,
													ce_input_event* event3)
{
	return ce_input_event_supply_and2_event(supply, event1,
			ce_input_event_supply_and2_event(supply, event2, event3));
}

// OR event

typedef struct {
	ce_input_event* event1;
	ce_input_event* event2;
} ce_input_event_or;

static void ce_input_event_or_ctor(ce_input_event* event, va_list args)
{
	ce_input_event_or* or_event = (ce_input_event_or*)event->impl;
	or_event->event1 = va_arg(args, ce_input_event*);
	or_event->event2 = va_arg(args, ce_input_event*);
}

static void ce_input_event_or_advance(ce_input_event* event, float elapsed)
{
	ce_unused(elapsed);
	ce_input_event_or* or_event = (ce_input_event_or*)event->impl;
	event->triggered = or_event->event1->triggered ||
						or_event->event2->triggered;
}

static const ce_input_event_vtable ce_input_event_or_vtable = {
	ce_input_event_or_ctor, NULL, ce_input_event_or_advance
};

ce_input_event* ce_input_event_supply_or2_event(ce_input_event_supply* supply,
													ce_input_event* event1,
													ce_input_event* event2)
{
	return ce_input_create_event(supply, ce_input_event_or_vtable,
									sizeof(ce_input_event_or), event1, event2);
}

ce_input_event* ce_input_event_supply_or3_event(ce_input_event_supply* supply,
													ce_input_event* event1,
													ce_input_event* event2,
													ce_input_event* event3)
{
	return ce_input_event_supply_or2_event(supply, event1,
			ce_input_event_supply_or2_event(supply, event2, event3));
}

// level 2 input API implementation

static const char* ce_input_button_names[CE_IB_COUNT] = {
	"unknown", "escape",
	"f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12",
	"tilde", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
	"minus", "equals", "backslash", "backspace",
	"tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
	"lbracket", "rbracket",
	"capslock", "a", "s", "d", "f", "g", "h", "j", "k", "l",
	"semicolon", "apostrophe", "enter",
	"lshift", "z", "x", "c", "v", "b", "n", "m",
	"comma", "period", "slash", "rshift",
	"lcontrol", "lmeta", "lalt", "space", "ralt", "rmeta", "menu", "rcontrol",
	"print", "scrolllock", "pause",
	"insert", "delete", "home", "end", "pageup", "pagedown",
	"left", "up", "right", "down",
	"numlock", "divide", "multiply", "subtract", "add", "numpadenter",
	"decimal",
	"numpad7", "numpad8", "numpad9", "numpad4", "numpad5",
	"numpad6", "numpad1", "numpad2", "numpad3", "numpad0",
	"left", "middle", "right",
	"wheelup", "wheeldown"
};

static ce_input_event*
ce_input_button_event_from_button_name(ce_input_event_supply* supply,
										const char* button_name)
{
	for (int i = CE_IB_UNKNOWN; i < CE_IB_COUNT; ++i) {
		if (0 == strcmp(button_name, ce_input_button_names[i])) {
			return ce_input_event_supply_button_event(supply, i);
		}
	}
	return NULL;
}

ce_input_event* ce_input_event_supply_shortcut(ce_input_event_supply* supply,
												const char* key_sequence)
{
	size_t length = strlen(key_sequence);
	char buffer[length + 1], buffer2[length + 1], buffer3[length + 1];
	char *or_seq = buffer, *and_seq, *button_name;
	ce_input_event *or_event = NULL, *and_event, *ev;

	ce_strlwr(buffer, key_sequence);

	do {
		and_seq = ce_strtrim(buffer2, ce_strsep(&or_seq, ","));
		if (0 == strlen(and_seq)) {
			ce_logging_warning("input: parsing key sequence: '%s': "
								"empty parts were skipped", key_sequence);
			continue;
		}

		and_event = NULL;
		do {
			button_name = ce_strtrim(buffer3, ce_strsep(&and_seq, "+"));
			if (0 == strlen(button_name)) {
				ce_logging_warning("input: parsing key sequence: '%s': "
									"empty parts were skipped", key_sequence);
				continue;
			}

			if (NULL == (ev = ce_input_button_event_from_button_name(supply,
														button_name))) {
				ce_logging_error("input: failed to parse "
									"key sequence: '%s'", key_sequence);
				return NULL;
			}

			if (NULL == (and_event = NULL == and_event ? ev :
					ce_input_event_supply_and2_event(supply, and_event, ev))) {
				return NULL;
			}
		} while (0 != strlen(and_seq));

		if (NULL == (or_event = NULL == or_event ? and_event :
				ce_input_event_supply_or2_event(supply, or_event, and_event))) {
			return NULL;
		}
	} while (0 != strlen(or_seq));

	return or_event;
}
