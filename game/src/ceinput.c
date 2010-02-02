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
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "ceinput.h"

typedef struct {
	void (*ctor)(void* self, va_list args);
	void (*dtor)(void* self);
	void (*advance)(void* self, float elapsed);
	bool (*triggered)(void* self);
} event_vtable;

struct ce_input_event {
	event_vtable vtable;
	size_t size;
	char object[];
};

struct ce_input_event_supply {
	size_t capacity;
	size_t count;
	ce_input_event** events;
};

// Button Event.

typedef struct {
	ce_input_button button;
	bool triggered;
} button_event;

static void button_event_ctor(void* self, va_list args)
{
	button_event* ev = self;
	ev->button = va_arg(args, ce_input_button);
	ev->triggered = false;
}

static void button_event_advance(void* self, float elapsed)
{
	ce_unused(elapsed);
	button_event* ev = self;
	ev->triggered = ce_input_test(ev->button);
}

static bool button_event_triggered(void* self)
{
	button_event* ev = self;
	return ev->triggered;
}

static event_vtable button_event_vtable = {
	button_event_ctor, NULL,
	button_event_advance, button_event_triggered
};

// Single Front Event.

typedef struct {
	ce_input_event* event;
	bool activated;
	bool triggered;
} single_front_event;

static void single_front_event_ctor(void* self, va_list args)
{
	single_front_event* ev = self;
	ev->event = va_arg(args, ce_input_event*);
	ev->activated = false;
	ev->triggered = false;
}

static void single_front_event_advance(void* self, float elapsed)
{
	ce_unused(elapsed);
	single_front_event* ev = self;
	bool triggered = ce_input_event_triggered(ev->event);
	ev->triggered = !ev->activated && triggered;
	ev->activated = triggered;
}

static bool single_front_event_triggered(void* self)
{
	single_front_event* ev = self;
	return ev->triggered;
}

static event_vtable single_front_event_vtable = {
	single_front_event_ctor, NULL,
	single_front_event_advance, single_front_event_triggered
};

// Single Back Event.

typedef struct {
	ce_input_event* event;
	bool activated;
	bool triggered;
} single_back_event;

static void single_back_event_ctor(void* self, va_list args)
{
	single_back_event* ev = self;
	ev->event = va_arg(args, ce_input_event*);
	ev->activated = false;
	ev->triggered = false;
}

static void single_back_event_advance(void* self, float elapsed)
{
	ce_unused(elapsed);
	single_back_event* ev = self;
	bool triggered = ce_input_event_triggered(ev->event);
	ev->triggered = ev->activated && !triggered;
	ev->activated = triggered;
}

static bool single_back_event_triggered(void* self)
{
	single_back_event* ev = self;
	return ev->triggered;
}

static event_vtable single_back_event_vtable = {
	single_back_event_ctor, NULL,
	single_back_event_advance, single_back_event_triggered
};

// High level input API implementation.

bool ce_input_event_triggered(ce_input_event* ev)
{
	return (ev->vtable.triggered)(ev->object);
}

ce_input_event_supply* ce_input_event_supply_open(void)
{
	ce_input_event_supply* es = ce_alloc(sizeof(ce_input_event_supply));
	if (NULL == es) {
		return NULL;
	}

	es->capacity = 16;
	es->count = 0;

	if (NULL == (es->events = ce_alloc(sizeof(ce_input_event*) *
												es->capacity))) {
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

	for (size_t i = 0; i < es->count; ++i) {
		ce_input_event* ev = es->events[i];
		if (NULL != ev->vtable.dtor) {
			(ev->vtable.dtor)(ev->object);
		}
		ce_free(ev, sizeof(ce_input_event) + ev->size);
	}

	ce_free(es->events, sizeof(ce_input_event*) * es->capacity);
	ce_free(es, sizeof(ce_input_event_supply));
}

void ce_input_event_supply_advance(ce_input_event_supply* es, float elapsed)
{
	for (size_t i = 0; i < es->count; ++i) {
		ce_input_event* ev = es->events[i];
		(ev->vtable.advance)(ev->object, elapsed);
	}
}

static ce_input_event* create_event(ce_input_event_supply* es,
									event_vtable vtable, size_t size, ...)
{
	ce_input_event* ev = ce_alloc(sizeof(ce_input_event) + size);
	if (NULL == ev) {
		return NULL;
	}

	ev->vtable = vtable;
	ev->size = size;

	va_list args;
	va_start(args, size);
	(ev->vtable.ctor)(ev->object, args);
	va_end(args);

	assert(es->count < es->capacity && "to be implemented");

	es->events[es->count++] = ev;

	return ev;
}

ce_input_event* ce_input_create_button_event(ce_input_event_supply* es,
												ce_input_button button)
{
	return create_event(es, button_event_vtable, sizeof(button_event), button);
}

ce_input_event* ce_input_create_single_front_event(ce_input_event_supply* es,
													ce_input_event* event)
{
	return create_event(es, single_front_event_vtable,
						sizeof(single_front_event), event);
}

ce_input_event* ce_input_create_single_back_event(ce_input_event_supply* es,
													ce_input_event* event)
{
	return create_event(es, single_back_event_vtable,
						sizeof(single_back_event), event);
}
