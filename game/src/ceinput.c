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

#include "celib.h"
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
	event_vtable vtable;
	bool triggered;
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

// High level input API implementation.

bool ce_input_event_triggered(ce_input_event* ev)
{
	return ev->triggered;
}

ce_input_event_supply* ce_input_event_supply_open(void)
{
	ce_input_event_supply* es = ce_alloc(sizeof(ce_input_event_supply));
	if (NULL == es) {
		ce_logging_error("input: could not allocate memory");
		return NULL;
	}

	if (NULL == (es->events = ce_vector_open())) {
		ce_logging_error("input: could not allocate memory");
		ce_input_event_supply_close(es);
		return NULL;
	}

	return es;
}

void ce_input_event_supply_close(ce_input_event_supply* es)
{
	if (NULL == es) {
		return;
	}

	if (NULL != es->events) {
		for (size_t i = 0, n = ce_vector_count(es->events); i < n; ++i) {
			ce_input_event* ev = ce_vector_get(es->events, i);
			if (NULL != ev->vtable.dtor) {
				(ev->vtable.dtor)(ev);
			}
			ce_free(ev, sizeof(ce_input_event) + ev->size);
		}
		ce_vector_close(es->events);
	}

	ce_free(es, sizeof(ce_input_event_supply));
}

void ce_input_event_supply_advance(ce_input_event_supply* es, float elapsed)
{
	for (size_t i = 0, n = ce_vector_count(es->events); i < n; ++i) {
		ce_input_event* ev = ce_vector_get(es->events, i);
		(ev->vtable.advance)(ev, elapsed);
	}
}

static ce_input_event* create_event(ce_input_event_supply* es,
									event_vtable vtable, size_t size, ...)
{
	ce_input_event* ev = ce_alloc(sizeof(ce_input_event) + size);
	if (NULL == ev) {
		ce_logging_error("input: could not allocate memory");
		return NULL;
	}

	ev->vtable = vtable;
	ev->triggered = false;
	ev->size = size;

	va_list args;
	va_start(args, size);
	(ev->vtable.ctor)(ev, args);
	va_end(args);

	ce_vector_push_back(es->events, ev);
	return ev;
}

// Button Event.

ce_input_event* ce_input_create_button_event(ce_input_event_supply* es,
												ce_input_button button)
{
	return create_event(es, button_event_vtable,
						sizeof(button_event), button);
}

// Single Front Event.

ce_input_event* ce_input_create_single_front_event(ce_input_event_supply* es,
													ce_input_event* event)
{
	return create_event(es, single_front_event_vtable,
						sizeof(single_front_event), event);
}

// Single Back Event.

ce_input_event* ce_input_create_single_back_event(ce_input_event_supply* es,
													ce_input_event* event)
{
	return create_event(es, single_back_event_vtable,
						sizeof(single_back_event), event);
}

// AND Event.

ce_input_event* ce_input_create_and2_event(ce_input_event_supply* es,
											ce_input_event* event1,
											ce_input_event* event2)
{
	return create_event(es, and_event_vtable,
						sizeof(and_event), event1, event2);
}

ce_input_event* ce_input_create_and3_event(ce_input_event_supply* es,
											ce_input_event* event1,
											ce_input_event* event2,
											ce_input_event* event3)
{
	return ce_input_create_and2_event(es, event1,
			ce_input_create_and2_event(es, event2, event3));
}

// OR Event.

ce_input_event* ce_input_create_or2_event(ce_input_event_supply* es,
											ce_input_event* event1,
											ce_input_event* event2)
{
	return create_event(es, or_event_vtable,
						sizeof(or_event), event1, event2);
}

ce_input_event* ce_input_create_or3_event(ce_input_event_supply* es,
											ce_input_event* event1,
											ce_input_event* event2,
											ce_input_event* event3)
{
	return ce_input_create_or2_event(es, event1,
			ce_input_create_or2_event(es, event2, event3));
}
