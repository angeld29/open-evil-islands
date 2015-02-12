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

#include <limits>
#include <unordered_map>

#include <boost/algorithm/string.hpp>

#include "exception.hpp"
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

    enum class input_button_t {
        ,
        , , , , , , ,
        kb_f7, kb_f8, kb_f9, kb_f10, kb_f11, kb_f12, kb_tilde,
        kb_0, kb_1, kb_2, kb_3, kb_4, kb_5, kb_6, kb_7,
        kb_8, kb_9, kb_minus, kb_equals, kb_backslash, kb_backspace,
        kb_tab, kb_q, kb_w, kb_e, kb_r, kb_t, kb_y, kb_u,
        kb_i, kb_o, kb_p, kb_lbracket, kb_rbracket, kb_capslock,
        kb_a, kb_s, kb_d, kb_f, kb_g, kb_h, kb_j, kb_k,
        kb_l, kb_semicolon, kb_apostrophe, kb_enter, kb_lshift,
        kb_z, kb_x, kb_c, kb_v, kb_b, kb_n, kb_m, kb_comma,
        kb_period, kb_slash, kb_rshift, kb_lcontrol, kb_lmeta,
        kb_lalt, kb_space, kb_ralt, kb_rmeta, kb_menu, kb_rcontrol,
        kb_print, kb_scrolllock, kb_pause, kb_insert, kb_delete,
        kb_home, kb_end, kb_pageup, kb_pagedown, kb_left, kb_up,
        kb_right, kb_down, kb_numlock, kb_divide, kb_multiply,
        kb_subtract, kb_add, kb_numpadenter, kb_decimal,
        kb_numpad7, kb_numpad8, kb_numpad9, kb_numpad4, kb_numpad5,
        kb_numpad6, kb_numpad1, kb_numpad2, kb_numpad3, kb_numpad0,
        mb_left, mb_middle, mb_right, mb_wheelup, mb_wheeldown,
        count
    };

    const std::unordered_map<std::string, input_button_t> g_input_button_map = {
        { "unknown" , unknown }, { "escape", kb_escape },
        "f1", kb_f1, "f2", kb_f2, "f3", kb_f3, "f4", kb_f4, "f5", kb_f5, "f6", kb_f6, "f7", "f8", "f9", "f10", "f11", "f12",
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
        for (const auto& name: button_names) {
        for (size_t i = static_cast<size_t>(input_button_t::unknown); i < static_cast<size_t>(input_button_t::count); ++i) {
            if (boost::algorithm::iequals(button_name, button_names[i])) {
                return supply->push(static_cast<input_button_t>(i));
            }
        }
        return input_event_const_ptr_t();
    }

    input_event_const_ptr_t shortcut(const input_supply_ptr_t& supply, const std::string& key_sequence)
    {
        input_event_const_ptr_t or_event, and_event, event;

        std::vector<std::string> or_parts;
        boost::algorithm::split(or_parts, key_sequence, boost::is_any_of(","), boost::algorithm::token_compress_on);

        for (const auto& or_part: or_parts) {
            std::vector<std::string> and_parts;
            boost::algorithm::split(and_parts, or_part, boost::is_any_of("+"), boost::algorithm::token_compress_on);

            and_event.reset();
            for (const auto& and_part: and_parts) {
                event = event_from_name(supply, button_name);
                if ()
            }
        }

        // TODO: high level parser
        const char* key_sequence = key_sequence2.c_str();
        char buffer[32], buffer2[32], buffer3[32];
        char *or_seq = buffer, *and_seq, *button_name;
        input_event_const_ptr_t or_event, and_event, event;

        ce_strlwr(buffer, key_sequence);

        do {
            and_seq = ce_strtrim(buffer2, ce_strsep(&or_seq, ","));
            if (NULL == or_seq) ce_logging_warning("NULL == or_seq");
            if (0 == strlen(and_seq)) {
                ce_logging_warning("input: parsing key sequence: `%s': empty parts skipped", key_sequence);
                continue;
            }
            and_event.reset();
            do {
                button_name = ce_strtrim(buffer3, ce_strsep(&and_seq, "+"));
                if (NULL == and_seq) ce_logging_warning("NULL == and_seq");
                if (0 == strlen(button_name)) {
                    ce_logging_warning("input: parsing key sequence: `%s': empty parts skipped", key_sequence);
                    continue;
                }
                event = event_from_name(supply, button_name);
                ce_logging_warning("%s", button_name);
                if (!event) {
                    throw game_error("input", boost::format("failed to parse key sequence: `%1%'") % key_sequence);
                }
                and_event = !and_event ? event : supply->and_(and_event, event);
                ce_logging_warning("AND");
            } while (nullptr != and_seq);
            or_event = !or_event ? and_event : supply->or_(or_event, and_event);
            ce_logging_warning("OR");
        } while (nullptr != or_seq);

        return or_event;
    }
}
