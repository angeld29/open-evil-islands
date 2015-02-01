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

#include <cstring>
#include <limits>
#include <stdexcept>
#include <algorithm>

#include "str.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "input.hpp"

namespace cursedearth
{
    // Level 0

    void input_context_t::clear()
    {
        memset(buttons, 0, sizeof(buttons));
        pointer_position = CE_VEC2_ZERO;
        pointer_offset = CE_VEC2_ZERO;
    }

    // Level 1

    // Push event

    class input_event_push_t final: input_event_t
    {
    public:
        input_event_push_t(const input_context_const_ptr_t& context, const input_button_t& button): m_context(context), m_button(button) {}

    private:
        virtual void advance(float) final
        {
            m_triggered = m_context->buttons[m_button];
        }

    private:
        input_context_const_ptr_t m_context;
        const input_button_t m_button;
    };

    // Single Front event

    class input_event_single_front_t final: input_event_t
    {
    public:
        explicit input_event_single_front_t(const input_event_const_ptr_t& event): m_event(event) {}

    private:
        virtual void advance(float) final
        {
            m_triggered = !m_activated && m_event->is_triggered();
            m_activated = m_event->is_triggered();
        }

    private:
        input_event_const_ptr_t m_event;
        bool m_activated = false;
    };

    // Single Back event

    class input_event_single_back_t final: input_event_t
    {
    public:
        explicit input_event_single_back_t(const input_event_const_ptr_t& event): m_event(event) {}

    private:
        virtual void advance(float) final
        {
            m_triggered = m_activated && !m_event->is_triggered();
            m_activated = m_event->is_triggered();
        }

    private:
        input_event_const_ptr_t m_event;
        bool m_activated = false;
    };

    // And event

    class input_event_and_t final: input_event_t
    {
    public:
        input_event_and_t(const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2): m_event1(event1), m_event2(event2) {}

    private:
        virtual void advance(float) final
        {
            m_triggered = m_event1->is_triggered() && m_event2->is_triggered();
        }

    private:
        input_event_const_ptr_t m_event1;
        input_event_const_ptr_t m_event2;
    };

    // Or event

    class input_event_or_t final: input_event_t
    {
    public:
        input_event_or_t(const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2): m_event1(event1), m_event2(event2) {}

    private:
        virtual void advance(float) final
        {
            m_triggered = m_event1->is_triggered() || m_event2->is_triggered();
        }

    private:
        input_event_const_ptr_t m_event1;
        input_event_const_ptr_t m_event2;
    };

    // Repeat event

    class input_event_repeat_t final: input_event_t
    {
    public:
        input_event_repeat_t(const input_event_const_ptr_t& event, int delay , int rate): m_event(event),
            m_delay(delay * 1e-3f),
            m_rate((rate > 0) ? (1.0f / rate) : std::numeric_limits<float>::max())
        {}

    private:
        virtual void advance(float elapsed) final
        {
            m_triggered = false;
            if (m_event->is_triggered()) {
                if (m_activated) {
                    if (m_delay_elapsed < m_delay) {
                        m_delay_elapsed += elapsed;
                        if (m_delay_elapsed >= m_delay) {
                            elapsed = m_delay_elapsed - m_delay;
                        }
                    }
                    if (m_delay_elapsed >= m_delay) {
                        m_rate_elapsed += elapsed;
                        if (m_rate_elapsed >= m_rate) {
                            m_triggered = true;
                            m_rate_elapsed -= m_rate;
                        }
                    }
                } else {
                    m_triggered = true;
                    m_activated = true;
                }
            } else {
                m_delay_elapsed = 0.0f;
                m_rate_elapsed = 0.0f;
                m_activated = false;
            }
        }

    private:
        input_event_const_ptr_t m_event;
        const float m_delay, m_rate;
        float m_delay_elapsed = 0.0f, m_rate_elapsed = 0.0f;
        bool m_activated = false;
    };

    input_supply_t::input_supply_t(const input_context_const_ptr_t* context): m_context(context) {}

    input_event_const_ptr_t input_supply_t::push(input_button_t button)
    {
        m_events.push_back(std::make_shared<input_event_push_t>(m_context, button));
        return m_events.back();
    }

    input_event_const_ptr_t input_supply_t::single_front(const input_event_const_ptr_t& event)
    {
        m_events.push_back(std::make_shared<input_event_single_front_t>(event));
        return m_events.back();
    }

    input_event_const_ptr_t input_supply_t::single_back(const input_event_const_ptr_t& event)
    {
        m_events.push_back(std::make_shared<input_event_single_back_t>(event));
        return m_events.back();
    }

    input_event_const_ptr_t input_supply_t::and2(const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2)
    {
        m_events.push_back(std::make_shared<input_event_and2_t>(event1, event2));
        return m_events.back();
    }

    input_event_const_ptr_t input_supply_t::and3(const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2, const input_event_const_ptr_t& event3)
    {
        m_events.push_back(std::make_shared<input_event_and3_t>(event1, event2, event3));
        return m_events.back();
    }

    input_event_const_ptr_t input_supply_t::or2(const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2)
    {
        m_events.push_back(std::make_shared<input_event_or2_t>(event1, event2));
        return m_events.back();
    }

    input_event_const_ptr_t input_supply_t::or3(const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2, const input_event_const_ptr_t& event3)
    {
        m_events.push_back(std::make_shared<input_event_or3_t>(event1, event2, event3));
        return m_events.back();
    }

    input_event_const_ptr_t input_supply_t::repeat(const input_event_const_ptr_t& event, int delay, int rate)
    {
        m_events.push_back(std::make_shared<input_event_repeat_t>(event, delay, rate));
        return m_events.back();
    }

    void input_supply_t::advance(float elapsed)
    {
        std::for_each(m_events.begin(), m_events.end(), [elapsed] (const input_event_ptr_t& event) { event->advance(elapsed) });
    }

    // Level 2

    const char* button_names[CE_IB_COUNT] = {
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

    input_event_const_ptr_t event_from_name(const input_supply_ptr_t& supply, const char* button_name)
    {
        for (size_t i = CE_IB_UNKNOWN; i < CE_IB_COUNT; ++i) {
            if (0 == strcmp(button_name, button_names[i])) {
                return supply->push(i);
            }
        }
        return input_event_const_ptr_t();
    }

    input_event_const_ptr_t shortcut(const input_supply_ptr_t& supply, const std::string& key_sequence2)
    {
        // TODO: high level parser
        const char* key_sequence = key_sequence2.c_str();
        size_t length = strlen(key_sequence);
        char buffer[length + 1], buffer2[length + 1], buffer3[length + 1];
        char *or_seq = buffer, *and_seq, *button_name;
        input_event_const_ptr_t or_event, and_event, event;

        ce_strlwr(buffer, key_sequence);

        do {
            and_seq = ce_strtrim(buffer2, ce_strsep(&or_seq, ","));
            if (0 == strlen(and_seq)) {
                ce_logging_warning("input: parsing key sequence: `%s': empty parts skipped", key_sequence);
                continue;
            }
            and_event.reset();
            do {
                button_name = ce_strtrim(buffer3, ce_strsep(&and_seq, "+"));
                if (0 == strlen(button_name)) {
                    ce_logging_warning("input: parsing key sequence: `%s': empty parts skipped", key_sequence);
                    continue;
                }
                event = event_from_name(supply, button_name);
                if (!event) {
                    throw std::runtime_error(boost::format("input: failed to parse key sequence: `%1%'") % key_sequence);
                }
                and_event = and_event.empty() ? event : supply->and2(and_event, event);
            } while (nullptr != and_seq);
            or_event = or_event.empty() ? and_event : supply->or2(or_event, and_event);
        } while (nullptr != or_seq);

        return or_event;
    }
}
