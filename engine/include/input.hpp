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

/*
 *  doc/input.txt
 */

#ifndef CE_INPUT_HPP
#define CE_INPUT_HPP

#include <memory>
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>

#include "makeunique.hpp"
#include "vec2.hpp"

namespace cursedearth
{
    enum input_button_t
    {
        CE_IB_UNKNOWN,
        CE_KB_ESCAPE, CE_KB_F1, CE_KB_F2, CE_KB_F3, CE_KB_F4, CE_KB_F5, CE_KB_F6,
        CE_KB_F7, CE_KB_F8, CE_KB_F9, CE_KB_F10, CE_KB_F11, CE_KB_F12, CE_KB_TILDE,
        CE_KB_0, CE_KB_1, CE_KB_2, CE_KB_3, CE_KB_4, CE_KB_5, CE_KB_6, CE_KB_7,
        CE_KB_8, CE_KB_9, CE_KB_MINUS, CE_KB_EQUALS, CE_KB_BACKSLASH, CE_KB_BACKSPACE,
        CE_KB_TAB, CE_KB_Q, CE_KB_W, CE_KB_E, CE_KB_R, CE_KB_T, CE_KB_Y, CE_KB_U,
        CE_KB_I, CE_KB_O, CE_KB_P, CE_KB_LBRACKET, CE_KB_RBRACKET, CE_KB_CAPSLOCK,
        CE_KB_A, CE_KB_S, CE_KB_D, CE_KB_F, CE_KB_G, CE_KB_H, CE_KB_J, CE_KB_K,
        CE_KB_L, CE_KB_SEMICOLON, CE_KB_APOSTROPHE, CE_KB_ENTER, CE_KB_LSHIFT,
        CE_KB_Z, CE_KB_X, CE_KB_C, CE_KB_V, CE_KB_B, CE_KB_N, CE_KB_M, CE_KB_COMMA,
        CE_KB_PERIOD, CE_KB_SLASH, CE_KB_RSHIFT, CE_KB_LCONTROL, CE_KB_LMETA,
        CE_KB_LALT, CE_KB_SPACE, CE_KB_RALT, CE_KB_RMETA, CE_KB_MENU, CE_KB_RCONTROL,
        CE_KB_PRINT, CE_KB_SCROLLLOCK, CE_KB_PAUSE, CE_KB_INSERT, CE_KB_DELETE,
        CE_KB_HOME, CE_KB_END, CE_KB_PAGEUP, CE_KB_PAGEDOWN, CE_KB_LEFT, CE_KB_UP,
        CE_KB_RIGHT, CE_KB_DOWN, CE_KB_NUMLOCK, CE_KB_DIVIDE, CE_KB_MULTIPLY,
        CE_KB_SUBTRACT, CE_KB_ADD, CE_KB_NUMPADENTER, CE_KB_DECIMAL,
        CE_KB_NUMPAD7, CE_KB_NUMPAD8, CE_KB_NUMPAD9, CE_KB_NUMPAD4, CE_KB_NUMPAD5,
        CE_KB_NUMPAD6, CE_KB_NUMPAD1, CE_KB_NUMPAD2, CE_KB_NUMPAD3, CE_KB_NUMPAD0,
        CE_MB_LEFT, CE_MB_MIDDLE, CE_MB_RIGHT, CE_MB_WHEELUP, CE_MB_WHEELDOWN,
        CE_IB_COUNT
    };

    enum
    {
        CE_INPUT_NO_DELAY,
        CE_INPUT_DEFAULT_DELAY = 450,
    };

    enum
    {
        CE_INPUT_NO_RATE,
        CE_INPUT_DEFAULT_RATE = 25,
    };

    // Level 0: raw

    struct input_context_t
    {
        bool buttons[CE_IB_COUNT];
        ce_vec2 pointer_position;
        ce_vec2 pointer_offset;

        void clear();
    };

    typedef std::shared_ptr<input_context_t> input_context_ptr_t;
    typedef std::shared_ptr<const input_context_t> input_context_const_ptr_t;

    // Level 1: events

    class input_event_t: boost::noncopyable
    {
    public:
        virtual ~input_event_t() = 0;

        virtual void advance(float elapsed) = 0;

        bool is_triggered() const { return m_triggered; }

    protected:
        bool m_triggered = false;
    };

    typedef std::shared_ptr<input_event_t> input_event_ptr_t;
    typedef std::shared_ptr<const input_event_t> input_event_const_ptr_t;

    class input_supply_t: boost::noncopyable
    {
    public:
        explicit input_supply_t(const input_context_const_ptr_t&);

        input_event_const_ptr_t push(input_button_t);
        input_event_const_ptr_t single_front(const input_event_const_ptr_t&);
        input_event_const_ptr_t single_back(const input_event_const_ptr_t&);
        input_event_const_ptr_t and2(const input_event_const_ptr_t&, const input_event_const_ptr_t&);
        input_event_const_ptr_t and3(const input_event_const_ptr_t&, const input_event_const_ptr_t&, const input_event_const_ptr_t&);
        input_event_const_ptr_t or2(const input_event_const_ptr_t&, const input_event_const_ptr_t&);
        input_event_const_ptr_t or3(const input_event_const_ptr_t&, const input_event_const_ptr_t&, const input_event_const_ptr_t&);
        input_event_const_ptr_t repeat(const input_event_const_ptr_t&, int /* delay in ms */, int /* rate in triggers per second */);

        void advance(float elapsed);

    private:
        input_context_const_ptr_t m_context;
        std::vector<input_event_ptr_t> m_events;
    };

    typedef std::unique_ptr<input_supply_t> input_supply_ptr_t;

    // Level 2: shortcuts

    input_event_const_ptr_t shortcut(const input_supply_ptr_t&, const std::string& key_sequence);
}

#endif /* CE_INPUT_HPP */
