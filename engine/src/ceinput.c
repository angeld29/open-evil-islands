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

#include <stdio.h>
#include <string.h>
#include <float.h>
#include <assert.h>

#include "celib.h"
#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceinput.h"

// level 0 input API implementation

ce_inputcontext* ce_inputcontext_new(void)
{
	return ce_alloc_zero(sizeof(ce_inputcontext));
}

void ce_inputcontext_del(ce_inputcontext* inputcontext)
{
	ce_free(inputcontext, sizeof(ce_inputcontext));
}

void ce_inputcontext_clear(ce_inputcontext* inputcontext)
{
	memset(inputcontext, 0, sizeof(ce_inputcontext));
}

// level 1 input API implementation

ce_inputsupply* ce_inputsupply_new(const ce_inputcontext* inputcontext)
{
	ce_inputsupply* inputsupply = ce_alloc(sizeof(ce_inputsupply));
	inputsupply->inputcontext = inputcontext;
	inputsupply->inputevents = ce_vector_new();
	return inputsupply;
}

void ce_inputsupply_del(ce_inputsupply* inputsupply)
{
	if (NULL != inputsupply) {
		for (size_t i = 0; i < inputsupply->inputevents->count; ++i) {
			ce_inputevent* inputevent = inputsupply->inputevents->items[i];
			if (NULL != inputevent->vtable.dtor) {
				(*inputevent->vtable.dtor)(inputevent);
			}
			ce_free(inputevent, sizeof(ce_inputevent) + inputevent->vtable.size);
		}
		ce_vector_del(inputsupply->inputevents);
		ce_free(inputsupply, sizeof(ce_inputsupply));
	}
}

void ce_inputsupply_advance(ce_inputsupply* inputsupply, float elapsed)
{
	for (size_t i = 0; i < inputsupply->inputevents->count; ++i) {
		ce_inputevent* inputevent = inputsupply->inputevents->items[i];
		(*inputevent->vtable.advance)(inputevent, elapsed);
	}
}

static ce_inputevent* ce_inputsupply_event(ce_inputsupply* inputsupply,
											ce_inputevent_vtable vtable, ...)
{
	ce_inputevent* inputevent = ce_alloc_zero(sizeof(ce_inputevent) + vtable.size);
	inputevent->vtable = vtable;

	va_list args;
	va_start(args, vtable);
	(*vtable.ctor)(inputevent, args);
	va_end(args);

	ce_vector_push_back(inputsupply->inputevents, inputevent);
	return inputevent;
}

// Button event

typedef struct {
	const ce_inputcontext* inputcontext;
	ce_inputbutton inputbutton;
} ce_inputevent_button;

static void ce_inputevent_button_ctor(ce_inputevent* inputevent, va_list args)
{
	ce_inputevent_button* buttonevent = (ce_inputevent_button*)inputevent->impl;
	buttonevent->inputcontext = va_arg(args, const ce_inputcontext*);
	buttonevent->inputbutton = va_arg(args, ce_inputbutton);
}

static void ce_inputevent_button_advance(ce_inputevent* inputevent, float elapsed)
{
	ce_unused(elapsed);
	ce_inputevent_button* buttonevent = (ce_inputevent_button*)inputevent->impl;
	inputevent->triggered = buttonevent->inputcontext->buttons[buttonevent->inputbutton];
}

ce_inputevent* ce_inputsupply_button(ce_inputsupply* inputsupply,
										ce_inputbutton inputbutton)
{
	return ce_inputsupply_event(inputsupply, (ce_inputevent_vtable)
		{sizeof(ce_inputevent_button), ce_inputevent_button_ctor, NULL,
		ce_inputevent_button_advance}, inputsupply->inputcontext, inputbutton);
}

// Single Front event

typedef struct {
	const ce_inputevent* event;
	bool activated;
} ce_inputevent_single_front;

static void ce_inputevent_single_front_ctor(ce_inputevent* inputevent, va_list args)
{
	ce_inputevent_single_front* sfevent = (ce_inputevent_single_front*)inputevent->impl;
	sfevent->event = va_arg(args, const ce_inputevent*);
}

static void ce_inputevent_single_front_advance(ce_inputevent* inputevent, float elapsed)
{
	ce_unused(elapsed);
	ce_inputevent_single_front* sfevent = (ce_inputevent_single_front*)inputevent->impl;
	inputevent->triggered = !sfevent->activated && sfevent->event->triggered;
	sfevent->activated = sfevent->event->triggered;
}

ce_inputevent* ce_inputsupply_single_front(ce_inputsupply* inputsupply,
											const ce_inputevent* inputevent)
{
	return ce_inputsupply_event(inputsupply, (ce_inputevent_vtable)
		{sizeof(ce_inputevent_single_front), ce_inputevent_single_front_ctor,
		NULL, ce_inputevent_single_front_advance}, inputevent);
}

// Single Back event

typedef struct {
	const ce_inputevent* event;
	bool activated;
} ce_inputevent_single_back;

static void ce_inputevent_single_back_ctor(ce_inputevent* inputevent, va_list args)
{
	ce_inputevent_single_back* sbevent = (ce_inputevent_single_back*)inputevent->impl;
	sbevent->event = va_arg(args, const ce_inputevent*);
}

static void ce_inputevent_single_back_advance(ce_inputevent* inputevent, float elapsed)
{
	ce_unused(elapsed);
	ce_inputevent_single_back* sbevent = (ce_inputevent_single_back*)inputevent->impl;
	inputevent->triggered = sbevent->activated && !sbevent->event->triggered;
	sbevent->activated = sbevent->event->triggered;
}

ce_inputevent* ce_inputsupply_single_back(ce_inputsupply* inputsupply,
											const ce_inputevent* inputevent)
{
	return ce_inputsupply_event(inputsupply, (ce_inputevent_vtable)
		{sizeof(ce_inputevent_single_back), ce_inputevent_single_back_ctor,
		NULL, ce_inputevent_single_back_advance}, inputevent);
}

// AND event

typedef struct {
	const ce_inputevent* inputevent1;
	const ce_inputevent* inputevent2;
} ce_inputevent_and;

static void ce_inputevent_and_ctor(ce_inputevent* inputevent, va_list args)
{
	ce_inputevent_and* andevent = (ce_inputevent_and*)inputevent->impl;
	andevent->inputevent1 = va_arg(args, const ce_inputevent*);
	andevent->inputevent2 = va_arg(args, const ce_inputevent*);
}

static void ce_inputevent_and_advance(ce_inputevent* inputevent, float elapsed)
{
	ce_unused(elapsed);
	ce_inputevent_and* andevent = (ce_inputevent_and*)inputevent->impl;
	inputevent->triggered = andevent->inputevent1->triggered &&
							andevent->inputevent2->triggered;
}

ce_inputevent* ce_inputsupply_and2(ce_inputsupply* inputsupply,
									const ce_inputevent* inputevent1,
									const ce_inputevent* inputevent2)
{
	return ce_inputsupply_event(inputsupply, (ce_inputevent_vtable)
		{sizeof(ce_inputevent_and), ce_inputevent_and_ctor, NULL,
		ce_inputevent_and_advance}, inputevent1, inputevent2);
}

ce_inputevent* ce_inputsupply_and3(ce_inputsupply* inputsupply,
									const ce_inputevent* inputevent1,
									const ce_inputevent* inputevent2,
									const ce_inputevent* inputevent3)
{
	return ce_inputsupply_and2(inputsupply, inputevent1,
			ce_inputsupply_and2(inputsupply, inputevent2, inputevent3));
}

// OR event

typedef struct {
	const ce_inputevent* inputevent1;
	const ce_inputevent* inputevent2;
} ce_inputevent_or;

static void ce_inputevent_or_ctor(ce_inputevent* inputevent, va_list args)
{
	ce_inputevent_or* orevent = (ce_inputevent_or*)inputevent->impl;
	orevent->inputevent1 = va_arg(args, const ce_inputevent*);
	orevent->inputevent2 = va_arg(args, const ce_inputevent*);
}

static void ce_inputevent_or_advance(ce_inputevent* inputevent, float elapsed)
{
	ce_unused(elapsed);
	ce_inputevent_or* orevent = (ce_inputevent_or*)inputevent->impl;
	inputevent->triggered = orevent->inputevent1->triggered ||
							orevent->inputevent2->triggered;
}

ce_inputevent* ce_inputsupply_or2(ce_inputsupply* inputsupply,
									const ce_inputevent* inputevent1,
									const ce_inputevent* inputevent2)
{
	return ce_inputsupply_event(inputsupply, (ce_inputevent_vtable)
		{sizeof(ce_inputevent_or), ce_inputevent_or_ctor, NULL,
		ce_inputevent_or_advance}, inputevent1, inputevent2);
}

ce_inputevent* ce_inputsupply_or3(ce_inputsupply* inputsupply,
									const ce_inputevent* inputevent1,
									const ce_inputevent* inputevent2,
									const ce_inputevent* inputevent3)
{
	return ce_inputsupply_or2(inputsupply, inputevent1,
			ce_inputsupply_or2(inputsupply, inputevent2, inputevent3));
}

// Repeat event

typedef struct {
	const ce_inputevent* inputevent;
	float delay, delay_elapsed;
	float rate, rate_elapsed;
	bool activated;
} ce_inputevent_repeat;

static void ce_inputevent_repeat_ctor(ce_inputevent* inputevent, va_list args)
{
	ce_inputevent_repeat* repeatevent = (ce_inputevent_repeat*)inputevent->impl;
	repeatevent->inputevent = va_arg(args, const ce_inputevent*);
	repeatevent->delay = va_arg(args, int) * 1e-3f;
	int rate = va_arg(args, int);
	repeatevent->rate = (rate > 0) ? (1.0f / rate) : FLT_MAX;
}

static void ce_inputevent_repeat_advance(ce_inputevent* inputevent, float elapsed)
{
	ce_inputevent_repeat* repeatevent = (ce_inputevent_repeat*)inputevent->impl;
	inputevent->triggered = false;

	if (repeatevent->inputevent->triggered) {
		if (repeatevent->activated) {
			if (repeatevent->delay_elapsed < repeatevent->delay) {
				repeatevent->delay_elapsed += elapsed;
				if (repeatevent->delay_elapsed >= repeatevent->delay) {
					elapsed = repeatevent->delay_elapsed - repeatevent->delay;
				}
			}
			if (repeatevent->delay_elapsed >= repeatevent->delay) {
				repeatevent->rate_elapsed += elapsed;
				if (repeatevent->rate_elapsed >= repeatevent->rate) {
					inputevent->triggered = true;
					repeatevent->rate_elapsed -= repeatevent->rate;
				}
			}
		} else {
			inputevent->triggered = true;
			repeatevent->activated = true;
		}
	} else {
		repeatevent->delay_elapsed = 0.0f;
		repeatevent->rate_elapsed = 0.0f;
		repeatevent->activated = false;
	}
}

ce_inputevent* ce_inputsupply_repeat(ce_inputsupply* inputsupply,
									const ce_inputevent* inputevent,
									int delay, int rate)
{
	return ce_inputsupply_event(inputsupply, (ce_inputevent_vtable)
		{sizeof(ce_inputevent_repeat), ce_inputevent_repeat_ctor,
		NULL, ce_inputevent_repeat_advance}, inputevent, delay, rate);
}

// level 2 input API implementation

static const char* ce_inputbutton_names[CE_IB_COUNT] = {
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
	"arrowleft", "arrowup", "arrowright", "arrowdown",
	"numlock", "divide", "multiply", "subtract", "add", "numpadenter",
	"decimal",
	"numpad7", "numpad8", "numpad9", "numpad4", "numpad5",
	"numpad6", "numpad1", "numpad2", "numpad3", "numpad0",
	"mouseleft", "mousemiddle", "mouseright",
	"wheelup", "wheeldown"
};

static ce_inputevent* ce_inputsupply_event_from_name(ce_inputsupply* inputsupply,
														const char* button_name)
{
	for (size_t i = CE_IB_UNKNOWN; i < CE_IB_COUNT; ++i) {
		if (0 == strcmp(button_name, ce_inputbutton_names[i])) {
			return ce_inputsupply_button(inputsupply, i);
		}
	}
	return NULL;
}

ce_inputevent* ce_inputsupply_shortcut(ce_inputsupply* inputsupply,
										const char* key_sequence)
{
	size_t length = strlen(key_sequence);
	char buffer[length + 1], buffer2[length + 1], buffer3[length + 1];
	char *or_seq = buffer, *and_seq, *button_name;
	ce_inputevent *or_event = NULL, *and_event, *ev;

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
									"empty parts skipped", key_sequence);
				continue;
			}

			if (NULL == (ev = ce_inputsupply_event_from_name(inputsupply, button_name))) {
				ce_logging_error("input: failed to parse "
								"key sequence: '%s'", key_sequence);
				return NULL;
			}

			if (NULL == (and_event = NULL == and_event ? ev :
					ce_inputsupply_and2(inputsupply, and_event, ev))) {
				return NULL;
			}
		} while (NULL != and_seq);

		if (NULL == (or_event = NULL == or_event ? and_event :
				ce_inputsupply_or2(inputsupply, or_event, and_event))) {
			return NULL;
		}
	} while (NULL != or_seq);

	return or_event;
}
