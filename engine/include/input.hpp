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

/*
 * doc/input.txt
 */

#ifndef CE_INPUT_HPP
#define CE_INPUT_HPP

#include <memory>
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>

#include "vector2.hpp"

namespace cursedearth
{
    enum class input_button_t {
        unknown,
        kb_escape, kb_f1, kb_f2, kb_f3, kb_f4, kb_f5, kb_f6,
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

    // Level 0: raw

    struct input_context_t
    {
        bool buttons[static_cast<size_t>(input_button_t::count)];
        vector2_t pointer_position;
        vector2_t pointer_offset;

        input_context_t() { clear(); }

        void clear();
    };

    typedef std::shared_ptr<input_context_t> input_context_ptr_t;
    typedef std::shared_ptr<const input_context_t> input_context_const_ptr_t;

    // Level 1: events

    class input_event_t: boost::noncopyable
    {
    public:
        virtual ~input_event_t() = default;

        virtual void advance(float elapsed) = 0;

        bool triggered() const { return m_triggered; }

    protected:
        bool m_triggered = false;
    };

    typedef std::shared_ptr<input_event_t> input_event_ptr_t;
    typedef std::shared_ptr<const input_event_t> input_event_const_ptr_t;

    class input_supply_t final: boost::noncopyable
    {
        enum {
            default_delay = 450, // delay in milliseconds
            default_rate = 10    // rate in triggers per second
        };

    public:
        explicit input_supply_t(const input_context_const_ptr_t&);

        input_event_const_ptr_t push(input_button_t);
        input_event_const_ptr_t single_front(const input_event_const_ptr_t&);
        input_event_const_ptr_t single_back(const input_event_const_ptr_t&);
        input_event_const_ptr_t and_(const input_event_const_ptr_t&, const input_event_const_ptr_t&);
        input_event_const_ptr_t or_(const input_event_const_ptr_t&, const input_event_const_ptr_t&);
        input_event_const_ptr_t repeat(const input_event_const_ptr_t&, unsigned int = default_delay, unsigned int = default_rate);

        void advance(float elapsed);

        const vector2_t& pointer_position() const { return m_context->pointer_position; }
        const vector2_t& pointer_offset() const { return m_context->pointer_offset; }

    private:
        input_context_const_ptr_t m_context;
        std::vector<input_event_ptr_t> m_events;
    };

    typedef std::shared_ptr<input_supply_t> input_supply_ptr_t;

    inline input_event_const_ptr_t and_(const input_supply_ptr_t& supply, const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2, const input_event_const_ptr_t& event3)
    {
        return supply->and_(event1, supply->and_(event2, event3));
    }

    inline input_event_const_ptr_t or_(const input_supply_ptr_t& supply, const input_event_const_ptr_t& event1, const input_event_const_ptr_t& event2, const input_event_const_ptr_t& event3)
    {
        return supply->or_(event1, supply->or_(event2, event3));
    }

    // Level 2: shortcuts

    input_event_const_ptr_t shortcut(const input_supply_ptr_t&, const std::string& key_sequence);
}

#endif
