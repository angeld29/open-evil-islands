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

#include <cstdio>
#include <cstring>
#include <cfloat>
#include <vector>

#include "str.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "input.hpp"

namespace cursedearth
{
    // level 0 input API implementation

    ce_input_context* ce_input_context_new(void)
    {
        return (ce_input_context*)ce_alloc_zero(sizeof(ce_input_context));
    }

    void ce_input_context_del(ce_input_context* input_context)
    {
        ce_free(input_context, sizeof(ce_input_context));
    }

    void ce_input_context_clear(ce_input_context* input_context)
    {
        memset(input_context, 0, sizeof(ce_input_context));
    }

    // level 1 input API implementation

    ce_input_supply* ce_input_supply_new(const ce_input_context* input_context)
    {
        ce_input_supply* input_supply = (ce_input_supply*)ce_alloc(sizeof(ce_input_supply));
        input_supply->input_context = input_context;
        input_supply->input_events = ce_vector_new();
        return input_supply;
    }

    void ce_input_supply_del(ce_input_supply* input_supply)
    {
        if (NULL != input_supply) {
            for (size_t i = 0; i < input_supply->input_events->count; ++i) {
                ce_input_event* input_event = (ce_input_event*)input_supply->input_events->items[i];
                if (NULL != input_event->vtable.dtor) {
                    (*input_event->vtable.dtor)(input_event);
                }
                ce_free(input_event, sizeof(ce_input_event) + input_event->vtable.size);
            }
            ce_vector_del(input_supply->input_events);
            ce_free(input_supply, sizeof(ce_input_supply));
        }
    }

    void ce_input_supply_advance(ce_input_supply* input_supply, float elapsed)
    {
        for (size_t i = 0; i < input_supply->input_events->count; ++i) {
            ce_input_event* input_event = (ce_input_event*)input_supply->input_events->items[i];
            (*input_event->vtable.advance)(input_event, elapsed);
        }
    }

    ce_input_event* ce_input_supply_event(ce_input_supply* input_supply,
                                                ce_input_event_vtable vtable, ...)
    {
        ce_input_event* input_event = (ce_input_event*)ce_alloc_zero(sizeof(ce_input_event) + vtable.size);
        input_event->vtable = vtable;

        va_list args;
        va_start(args, vtable);
        (*vtable.ctor)(input_event, args);
        va_end(args);

        ce_vector_push_back(input_supply->input_events, input_event);
        return input_event;
    }

    // Button event

    typedef struct {
        const ce_input_context* input_context;
        ce_input_button input_button;
    } ce_input_event_button;

    void ce_input_event_button_ctor(ce_input_event* input_event, va_list args)
    {
        ce_input_event_button* button_event = (ce_input_event_button*)input_event->impl;
        button_event->input_context = va_arg(args, const ce_input_context*);
        button_event->input_button = static_cast<ce_input_button>(va_arg(args, int));
    }

    void ce_input_event_button_advance(ce_input_event* input_event, float)
    {
        ce_input_event_button* button_event = (ce_input_event_button*)input_event->impl;
        input_event->triggered = button_event->input_context->buttons[button_event->input_button];
    }

    ce_input_event* ce_input_supply_button(ce_input_supply* input_supply, ce_input_button input_button)
    {
        ce_input_event_vtable vt = { sizeof(ce_input_event_button), ce_input_event_button_ctor, NULL, ce_input_event_button_advance };
        return ce_input_supply_event(input_supply, vt, input_supply->input_context, static_cast<int>(input_button));
    }

    // Single Front event

    typedef struct {
        const ce_input_event* event;
        bool activated;
    } ce_input_event_single_front;

    void ce_input_event_single_front_ctor(ce_input_event* input_event, va_list args)
    {
        ce_input_event_single_front* single_front_event = (ce_input_event_single_front*)input_event->impl;
        single_front_event->event = va_arg(args, const ce_input_event*);
    }

    void ce_input_event_single_front_advance(ce_input_event* input_event, float)
    {
        ce_input_event_single_front* single_front_event = (ce_input_event_single_front*)input_event->impl;
        input_event->triggered = !single_front_event->activated && single_front_event->event->triggered;
        single_front_event->activated = single_front_event->event->triggered;
    }

    ce_input_event* ce_input_supply_single_front(ce_input_supply* input_supply,
                                                const ce_input_event* input_event)
    {
        ce_input_event_vtable vt = { sizeof(ce_input_event_single_front), ce_input_event_single_front_ctor, NULL, ce_input_event_single_front_advance };
        return ce_input_supply_event(input_supply, vt, input_event);
    }

    // Single Back event

    typedef struct {
        const ce_input_event* event;
        bool activated;
    } ce_input_event_single_back;

    void ce_input_event_single_back_ctor(ce_input_event* input_event, va_list args)
    {
        ce_input_event_single_back* single_back_event = (ce_input_event_single_back*)input_event->impl;
        single_back_event->event = va_arg(args, const ce_input_event*);
    }

    void ce_input_event_single_back_advance(ce_input_event* input_event, float)
    {
        ce_input_event_single_back* single_back_event = (ce_input_event_single_back*)input_event->impl;
        input_event->triggered = single_back_event->activated && !single_back_event->event->triggered;
        single_back_event->activated = single_back_event->event->triggered;
    }

    ce_input_event* ce_input_supply_single_back(ce_input_supply* input_supply,
                                                const ce_input_event* input_event)
    {
        ce_input_event_vtable vt = { sizeof(ce_input_event_single_back), ce_input_event_single_back_ctor, NULL, ce_input_event_single_back_advance };
        return ce_input_supply_event(input_supply, vt, input_event);
    }

    // AND event

    typedef struct {
        const ce_input_event* input_event1;
        const ce_input_event* input_event2;
    } ce_input_event_and;

    void ce_input_event_and_ctor(ce_input_event* input_event, va_list args)
    {
        ce_input_event_and* and_event = (ce_input_event_and*)input_event->impl;
        and_event->input_event1 = va_arg(args, const ce_input_event*);
        and_event->input_event2 = va_arg(args, const ce_input_event*);
    }

    void ce_input_event_and_advance(ce_input_event* input_event, float)
    {
        ce_input_event_and* and_event = (ce_input_event_and*)input_event->impl;
        input_event->triggered = and_event->input_event1->triggered &&
                                and_event->input_event2->triggered;
    }

    ce_input_event* ce_input_supply_and2(ce_input_supply* input_supply,
                                        const ce_input_event* input_event1,
                                        const ce_input_event* input_event2)
    {
        ce_input_event_vtable vt = { sizeof(ce_input_event_and), ce_input_event_and_ctor, NULL, ce_input_event_and_advance };
        return ce_input_supply_event(input_supply, vt, input_event1, input_event2);
    }

    ce_input_event* ce_input_supply_and3(ce_input_supply* input_supply,
                                        const ce_input_event* input_event1,
                                        const ce_input_event* input_event2,
                                        const ce_input_event* input_event3)
    {
        return ce_input_supply_and2(input_supply, input_event1,
                ce_input_supply_and2(input_supply, input_event2, input_event3));
    }

    // OR event

    typedef struct {
        const ce_input_event* input_event1;
        const ce_input_event* input_event2;
    } ce_input_event_or;

    void ce_input_event_or_ctor(ce_input_event* input_event, va_list args)
    {
        ce_input_event_or* or_event = (ce_input_event_or*)input_event->impl;
        or_event->input_event1 = va_arg(args, const ce_input_event*);
        or_event->input_event2 = va_arg(args, const ce_input_event*);
    }

    void ce_input_event_or_advance(ce_input_event* input_event, float)
    {
        ce_input_event_or* or_event = (ce_input_event_or*)input_event->impl;
        input_event->triggered = or_event->input_event1->triggered ||
                                or_event->input_event2->triggered;
    }

    ce_input_event* ce_input_supply_or2(ce_input_supply* input_supply,
                                        const ce_input_event* input_event1,
                                        const ce_input_event* input_event2)
    {
        ce_input_event_vtable vt = { sizeof(ce_input_event_or), ce_input_event_or_ctor, NULL, ce_input_event_or_advance };
        return ce_input_supply_event(input_supply, vt, input_event1, input_event2);
    }

    ce_input_event* ce_input_supply_or3(ce_input_supply* input_supply,
                                        const ce_input_event* input_event1,
                                        const ce_input_event* input_event2,
                                        const ce_input_event* input_event3)
    {
        return ce_input_supply_or2(input_supply, input_event1,
                ce_input_supply_or2(input_supply, input_event2, input_event3));
    }

    // Repeat event

    typedef struct {
        const ce_input_event* input_event;
        float delay, delay_elapsed;
        float rate, rate_elapsed;
        bool activated;
    } ce_input_event_repeat;

    void ce_input_event_repeat_ctor(ce_input_event* input_event, va_list args)
    {
        ce_input_event_repeat* repeat_event = (ce_input_event_repeat*)input_event->impl;
        repeat_event->input_event = va_arg(args, const ce_input_event*);
        repeat_event->delay = va_arg(args, int) * 1e-3f;
        int rate = va_arg(args, int);
        repeat_event->rate = (rate > 0) ? (1.0f / rate) : FLT_MAX;
    }

    void ce_input_event_repeat_advance(ce_input_event* input_event, float elapsed)
    {
        ce_input_event_repeat* repeat_event = (ce_input_event_repeat*)input_event->impl;
        input_event->triggered = false;

        if (repeat_event->input_event->triggered) {
            if (repeat_event->activated) {
                if (repeat_event->delay_elapsed < repeat_event->delay) {
                    repeat_event->delay_elapsed += elapsed;
                    if (repeat_event->delay_elapsed >= repeat_event->delay) {
                        elapsed = repeat_event->delay_elapsed - repeat_event->delay;
                    }
                }
                if (repeat_event->delay_elapsed >= repeat_event->delay) {
                    repeat_event->rate_elapsed += elapsed;
                    if (repeat_event->rate_elapsed >= repeat_event->rate) {
                        input_event->triggered = true;
                        repeat_event->rate_elapsed -= repeat_event->rate;
                    }
                }
            } else {
                input_event->triggered = true;
                repeat_event->activated = true;
            }
        } else {
            repeat_event->delay_elapsed = 0.0f;
            repeat_event->rate_elapsed = 0.0f;
            repeat_event->activated = false;
        }
    }

    ce_input_event* ce_input_supply_repeat(ce_input_supply* input_supply,
                                        const ce_input_event* input_event,
                                        int delay, int rate)
    {
        ce_input_event_vtable vt = { sizeof(ce_input_event_repeat), ce_input_event_repeat_ctor, NULL, ce_input_event_repeat_advance };
        return ce_input_supply_event(input_supply, vt, input_event, delay, rate);
    }

    // level 2 input API implementation

    const char* ce_input_button_names[CE_IB_COUNT] = {
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

    ce_input_event* ce_input_supply_event_from_name(ce_input_supply* input_supply, const char* button_name)
    {
        for (size_t i = CE_IB_UNKNOWN; i < CE_IB_COUNT; ++i) {
            if (0 == strcmp(button_name, ce_input_button_names[i])) {
                return ce_input_supply_button(input_supply, static_cast<ce_input_button>(i));
            }
        }
        return NULL;
    }

    ce_input_event* ce_input_supply_shortcut(ce_input_supply* input_supply, const char* key_sequence)
    {
        size_t length = strlen(key_sequence);
        std::vector<char> buffer(length + 1), buffer2(length + 1), buffer3(length + 1);
        char *or_seq = buffer.data(), *and_seq, *button_name;
        ce_input_event *or_event = NULL, *and_event, *ev;

        ce_strlwr(buffer.data(), key_sequence);

        do {
            and_seq = ce_strtrim(buffer2.data(), ce_strsep(&or_seq, ","));
            if (0 == strlen(and_seq)) {
                ce_logging_warning("input: parsing key sequence: `%s': empty parts were skipped", key_sequence);
                continue;
            }

            and_event = NULL;
            do {
                button_name = ce_strtrim(buffer3.data(), ce_strsep(&and_seq, "+"));
                if (0 == strlen(button_name)) {
                    ce_logging_warning("input: parsing key sequence: `%s': empty parts skipped", key_sequence);
                    continue;
                }

                if (NULL == (ev = ce_input_supply_event_from_name(input_supply, button_name))) {
                    ce_logging_error("input: failed to parse key sequence: `%s'", key_sequence);
                    return NULL;
                }

                if (NULL == (and_event = NULL == and_event ? ev : ce_input_supply_and2(input_supply, and_event, ev))) {
                    return NULL;
                }
            } while (NULL != and_seq);

            if (NULL == (or_event = NULL == or_event ? and_event : ce_input_supply_or2(input_supply, or_event, and_event))) {
                return NULL;
            }
        } while (NULL != or_seq);

        return or_event;
    }
}
