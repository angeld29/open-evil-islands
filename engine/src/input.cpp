/*
 * This file is part of Cursed Earth.
 *
 * Cursed Earth is an open source, cross-platform port of Evil Islands.
 * Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "input.hpp"
#include "exception.hpp"

#include <boost/algorithm/string.hpp>

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

    class input_event_push_t final: public input_event_t
    {
    public:
        input_event_push_t(const input_context_const_ptr_t& context, input_button_t button): m_context(context), m_button(button) {}

    private:
        virtual void advance(float) final
        {
            m_triggered = m_context->buttons[static_cast<size_t>(m_button)];
        }

    private:
        input_context_const_ptr_t m_context;
        const input_button_t m_button;
    };

    // Single Front event

    class input_event_single_front_t final: public input_event_t
    {
    public:
        explicit input_event_single_front_t(const input_event_const_ptr_t& event): m_event(event) {}

    private:
        virtual void advance(float) final
        {
            m_triggered = !m_activated && m_event->triggered();
            m_activated = m_event->triggered();
        }

    private:
        input_event_const_ptr_t m_event;
        bool m_activated = false;
    };

    // Single Back event

    class input_event_single_back_t final: public input_event_t
    {
    public:
        explicit input_event_single_back_t(const input_event_const_ptr_t& event): m_event(event) {}

    private:
        virtual void advance(float) final
        {
            m_triggered = m_activated && !m_event->triggered();
            m_activated = m_event->triggered();
        }

    private:
        input_event_const_ptr_t m_event;
        bool m_activated = false;
    };

    // And event

    class input_event_and_t final: public input_event_t
    {
    public:
        input_event_and_t(const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2): m_event1(event1), m_event2(event2) {}

    private:
        virtual void advance(float) final
        {
            m_triggered = m_event1->triggered() && m_event2->triggered();
        }

    private:
        input_event_const_ptr_t m_event1;
        input_event_const_ptr_t m_event2;
    };

    // Or event

    class input_event_or_t final: public input_event_t
    {
    public:
        input_event_or_t(const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2): m_event1(event1), m_event2(event2) {}

    private:
        virtual void advance(float) final
        {
            m_triggered = m_event1->triggered() || m_event2->triggered();
        }

    private:
        input_event_const_ptr_t m_event1;
        input_event_const_ptr_t m_event2;
    };

    // Repeat event

    class input_event_repeat_t final: public input_event_t
    {
    public:
        input_event_repeat_t(const input_event_const_ptr_t& event, unsigned int delay , unsigned int rate): m_event(event),
            m_delay(delay * 1e-3f),
            m_rate((0 != rate) ? (1.0f / rate) : std::numeric_limits<float>::max())
        {}

    private:
        virtual void advance(float elapsed) final
        {
            m_triggered = false;
            if (m_event->triggered()) {
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

    input_supply_t::input_supply_t(const input_context_const_ptr_t& context): m_context(context) {}

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

    input_event_const_ptr_t input_supply_t::and_(const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2)
    {
        m_events.push_back(std::make_shared<input_event_and_t>(event1, event2));
        return m_events.back();
    }

    input_event_const_ptr_t input_supply_t::or_(const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2)
    {
        m_events.push_back(std::make_shared<input_event_or_t>(event1, event2));
        return m_events.back();
    }

    input_event_const_ptr_t input_supply_t::repeat(const input_event_const_ptr_t& event, unsigned int delay, unsigned int rate)
    {
        m_events.push_back(std::make_shared<input_event_repeat_t>(event, delay, rate));
        return m_events.back();
    }

    void input_supply_t::advance(float elapsed)
    {
        for (const auto& event: m_events)
        {
            event->advance(elapsed);
        }
    }

    // Level 2

    const std::string g_input_button_names[static_cast<size_t>(input_button_t::count)] = {
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

    input_event_const_ptr_t event_from_name(const input_supply_ptr_t& supply, const std::string& button_name)
    {
        for (size_t i = static_cast<size_t>(input_button_t::unknown); i < static_cast<size_t>(input_button_t::count); ++i) {
            if (boost::algorithm::iequals(button_name, g_input_button_names[i])) {
                return supply->push(static_cast<input_button_t>(i));
            }
        }
        return input_event_const_ptr_t();
    }

    input_event_const_ptr_t shortcut(const input_supply_ptr_t& supply, const std::string& key_sequence)
    {
        input_event_const_ptr_t or_event;
        std::vector<std::string> or_parts;
        boost::algorithm::split(or_parts, key_sequence, boost::is_any_of(","), boost::algorithm::token_compress_on);

        for (const auto& or_part: or_parts) {
            input_event_const_ptr_t and_event;
            std::vector<std::string> and_parts;
            boost::algorithm::split(and_parts, or_part, boost::is_any_of("+"), boost::algorithm::token_compress_on);

            for (const auto& and_part: and_parts) {
                if (input_event_const_ptr_t event = event_from_name(supply, boost::algorithm::trim_copy(and_part))) {
                    and_event = !and_event ? event : supply->and_(and_event, event);
                } else {
                    throw game_error("input", "failed to parse key sequence: `%1%'", key_sequence);
                }
            }
            or_event = !or_event ? and_event : supply->or_(or_event, and_event);
        }

        return or_event;
    }
}
