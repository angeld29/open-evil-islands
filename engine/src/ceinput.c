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

#include <stdarg.h>
#include <string.h>

#include "celib.h"
#include "cestr.h"
#include "celogging.h"
#include "cealloc.h"
#include "cevector.h"
#include "ceinput.h"

typedef struct {
	void (*ctor)(ce_input_event* self, va_list args);
	void (*dtor)(ce_input_event* self);
	void (*advance)(ce_input_event* self, float elapsed);
} event_vtable;

/*
 *  The 'triggered' variable is cached. It's ok. The 'advance' function
 *  will be called from low-to-high event (creation order). For example,
 *  button_event_advance ('triggered' updated) -> button_event_advance
 *  ('triggered' updated) -> and_event_advance ('triggered' updated) ->
 *  single_back_event_advance ('triggered' updated).
*/

struct ce_input_event {
	bool triggered;
	event_vtable vtable;
	size_t size;
	char object[];
};

struct ce_input_event_supply {
	ce_vector* events;
};

// Button Event.

typedef struct {
	ce_input_button button;
} button_event;

static void button_event_ctor(ce_input_event* self, va_list args)
{
	button_event* ev = (button_event*)self->object;
	ev->button = va_arg(args, ce_input_button);
}

static void button_event_advance(ce_input_event* self, float elapsed)
{
	ce_unused(elapsed);
	button_event* ev = (button_event*)self->object;
	self->triggered = ce_input_test(ev->button);
}

static event_vtable button_event_vtable = {
	button_event_ctor, NULL, button_event_advance
};

// Single Front Event.

typedef struct {
	ce_input_event* event;
	bool activated;
} single_front_event;

static void single_front_event_ctor(ce_input_event* self, va_list args)
{
	single_front_event* ev = (single_front_event*)self->object;
	ev->event = va_arg(args, ce_input_event*);
	ev->activated = false;
}

static void single_front_event_advance(ce_input_event* self, float elapsed)
{
	ce_unused(elapsed);
	single_front_event* ev = (single_front_event*)self->object;
	bool triggered = ce_input_event_triggered(ev->event);
	self->triggered = !ev->activated && triggered;
	ev->activated = triggered;
}

static event_vtable single_front_event_vtable = {
	single_front_event_ctor, NULL, single_front_event_advance
};

// Single Back Event.

typedef struct {
	ce_input_event* event;
	bool activated;
} single_back_event;

static void single_back_event_ctor(ce_input_event* self, va_list args)
{
	single_back_event* ev = (single_back_event*)self->object;
	ev->event = va_arg(args, ce_input_event*);
	ev->activated = false;
}

static void single_back_event_advance(ce_input_event* self, float elapsed)
{
	ce_unused(elapsed);
	single_back_event* ev = (single_back_event*)self->object;
	bool triggered = ce_input_event_triggered(ev->event);
	self->triggered = ev->activated && !triggered;
	ev->activated = triggered;
}

static event_vtable single_back_event_vtable = {
	single_back_event_ctor, NULL, single_back_event_advance
};

// AND Event.

typedef struct {
	ce_input_event* event1;
	ce_input_event* event2;
} and_event;

static void and_event_ctor(ce_input_event* self, va_list args)
{
	and_event* ev = (and_event*)self->object;
	ev->event1 = va_arg(args, ce_input_event*);
	ev->event2 = va_arg(args, ce_input_event*);
}

static void and_event_advance(ce_input_event* self, float elapsed)
{
	ce_unused(elapsed);
	and_event* ev = (and_event*)self->object;
	self->triggered = ce_input_event_triggered(ev->event1) &&
						ce_input_event_triggered(ev->event2);
}

static event_vtable and_event_vtable = {
	and_event_ctor, NULL, and_event_advance
};

// OR Event.

typedef struct {
	ce_input_event* event1;
	ce_input_event* event2;
} or_event;

static void or_event_ctor(ce_input_event* self, va_list args)
{
	or_event* ev = (or_event*)self->object;
	ev->event1 = va_arg(args, ce_input_event*);
	ev->event2 = va_arg(args, ce_input_event*);
}

static void or_event_advance(ce_input_event* self, float elapsed)
{
	ce_unused(elapsed);
	or_event* ev = (or_event*)self->object;
	self->triggered = ce_input_event_triggered(ev->event1) ||
						ce_input_event_triggered(ev->event2);
}

static event_vtable or_event_vtable = {
	or_event_ctor, NULL, or_event_advance
};

// Level 1 input API implementation.

bool ce_input_event_triggered(ce_input_event* ev)
{
	return ev->triggered;
}

ce_input_event_supply* ce_input_event_supply_new(void)
{
	ce_input_event_supply* es = ce_alloc(sizeof(ce_input_event_supply));
	es->events = ce_vector_new();
	return es;
}

void ce_input_event_supply_del(ce_input_event_supply* es)
{
	if (NULL != es) {
		for (int i = 0; i < es->events->count; ++i) {
			ce_input_event* ev = es->events->items[i];
			if (NULL != ev->vtable.dtor) {
				(ev->vtable.dtor)(ev);
			}
			ce_free(ev, sizeof(ce_input_event) + ev->size);
		}
		ce_vector_del(es->events);
		ce_free(es, sizeof(ce_input_event_supply));
	}
}

void ce_input_event_supply_advance(ce_input_event_supply* es, float elapsed)
{
	for (int i = 0; i < es->events->count; ++i) {
		ce_input_event* ev = es->events->items[i];
		(ev->vtable.advance)(ev, elapsed);
	}
}

static ce_input_event* ce_input_create_event(ce_input_event_supply* es,
										event_vtable vtable, size_t size, ...)
{
	ce_input_event* ev = ce_alloc(sizeof(ce_input_event) + size);

	ev->triggered = false;
	ev->vtable = vtable;
	ev->size = size;

	va_list args;
	va_start(args, size);
	(ev->vtable.ctor)(ev, args);
	va_end(args);

	return ce_vector_push_back(es->events, ev), ev;
}

// Button Event.

ce_input_event* ce_input_event_supply_button_event(ce_input_event_supply* es,
													ce_input_button button)
{
	return ce_input_create_event(es, button_event_vtable,
									sizeof(button_event), button);
}

// Single Front Event.

ce_input_event*
ce_input_event_supply_single_front_event(ce_input_event_supply* es,
											ce_input_event* event)
{
	return ce_input_create_event(es, single_front_event_vtable,
									sizeof(single_front_event), event);
}

// Single Back Event.

ce_input_event*
ce_input_event_supply_single_back_event(ce_input_event_supply* es,
											ce_input_event* event)
{
	return ce_input_create_event(es, single_back_event_vtable,
									sizeof(single_back_event), event);
}

// AND Event.

ce_input_event* ce_input_event_supply_and2_event(ce_input_event_supply* es,
													ce_input_event* event1,
													ce_input_event* event2)
{
	return ce_input_create_event(es, and_event_vtable,
									sizeof(and_event), event1, event2);
}

ce_input_event* ce_input_event_supply_and3_event(ce_input_event_supply* es,
													ce_input_event* event1,
													ce_input_event* event2,
													ce_input_event* event3)
{
	return ce_input_event_supply_and2_event(es, event1,
			ce_input_event_supply_and2_event(es, event2, event3));
}

// OR Event.

ce_input_event* ce_input_event_supply_or2_event(ce_input_event_supply* es,
													ce_input_event* event1,
													ce_input_event* event2)
{
	return ce_input_create_event(es, or_event_vtable,
									sizeof(or_event), event1, event2);
}

ce_input_event* ce_input_event_supply_or3_event(ce_input_event_supply* es,
													ce_input_event* event1,
													ce_input_event* event2,
													ce_input_event* event3)
{
	return ce_input_event_supply_or2_event(es, event1,
			ce_input_event_supply_or2_event(es, event2, event3));
}

// Level 2 input API implementation.

static ce_input_event*
ce_input_create_event_from_button_name(ce_input_event_supply* es,
										const char* button_name);

ce_input_event* ce_input_event_supply_shortcut(ce_input_event_supply* es,
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

			if (NULL == (ev = ce_input_create_event_from_button_name(es,
														button_name))) {
				ce_logging_error("input: failed to parse "
									"key sequence: '%s'", key_sequence);
				return NULL;
			}

			if (NULL == (and_event = NULL == and_event ? ev :
					ce_input_event_supply_and2_event(es, and_event, ev))) {
				return NULL;
			}
		} while (0 != strlen(and_seq));

		if (NULL == (or_event = NULL == or_event ? and_event :
				ce_input_event_supply_or2_event(es, or_event, and_event))) {
			return NULL;
		}
	} while (0 != strlen(or_seq));

	return or_event;
}

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
ce_input_create_event_from_button_name(ce_input_event_supply* es,
										const char* button_name)
{
	for (int i = CE_IB_UNKNOWN; i < CE_IB_COUNT; ++i) {
		if (0 == strcmp(button_name, ce_input_button_names[i])) {
			return ce_input_event_supply_button_event(es, (ce_input_button)i);
		}
	}
	return NULL;
}
