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

#ifndef CE_INPUT_H
#define CE_INPUT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CEIB_UNKNOWN,
	CEKB_ESCAPE,
	CEKB_F1,
	CEKB_F2,
	CEKB_F3,
	CEKB_F4,
	CEKB_F5,
	CEKB_F6,
	CEKB_F7,
	CEKB_F8,
	CEKB_F9,
	CEKB_F10,
	CEKB_F11,
	CEKB_F12,
	CEKB_TILDE,
	CEKB_0,
	CEKB_1,
	CEKB_2,
	CEKB_3,
	CEKB_4,
	CEKB_5,
	CEKB_6,
	CEKB_7,
	CEKB_8,
	CEKB_9,
	CEKB_MINUS,
	CEKB_EQUALS,
	CEKB_BACKSLASH,
	CEKB_BACKSPACE,
	CEKB_TAB,
	CEKB_Q,
	CEKB_W,
	CEKB_E,
	CEKB_R,
	CEKB_T,
	CEKB_Y,
	CEKB_U,
	CEKB_I,
	CEKB_O,
	CEKB_P,
	CEKB_LBRACKET,
	CEKB_RBRACKET,
	CEKB_CAPSLOCK,
	CEKB_A,
	CEKB_S,
	CEKB_D,
	CEKB_F,
	CEKB_G,
	CEKB_H,
	CEKB_J,
	CEKB_K,
	CEKB_L,
	CEKB_SEMICOLON,
	CEKB_APOSTROPHE,
	CEKB_ENTER,
	CEKB_LSHIFT,
	CEKB_Z,
	CEKB_X,
	CEKB_C,
	CEKB_V,
	CEKB_B,
	CEKB_N,
	CEKB_M,
	CEKB_COMMA,
	CEKB_PERIOD,
	CEKB_SLASH,
	CEKB_RSHIFT,
	CEKB_LCONTROL,
	CEKB_LMETA,
	CEKB_LALT,
	CEKB_SPACE,
	CEKB_RALT,
	CEKB_RMETA,
	CEKB_MENU,
	CEKB_RCONTROL,
	CEKB_PRINT,
	CEKB_SCROLLLOCK,
	CEKB_PAUSE,
	CEKB_INSERT,
	CEKB_DELETE,
	CEKB_HOME,
	CEKB_END,
	CEKB_PAGEUP,
	CEKB_PAGEDOWN,
	CEKB_LEFT,
	CEKB_UP,
	CEKB_RIGHT,
	CEKB_DOWN,
	CEKB_NUMLOCK,
	CEKB_DIVIDE,
	CEKB_MULTIPLY,
	CEKB_SUBTRACT,
	CEKB_ADD,
	CEKB_NUMPADENTER,
	CEKB_DECIMAL,
	CEKB_NUMPAD7,
	CEKB_NUMPAD8,
	CEKB_NUMPAD9,
	CEKB_NUMPAD4,
	CEKB_NUMPAD5,
	CEKB_NUMPAD6,
	CEKB_NUMPAD1,
	CEKB_NUMPAD2,
	CEKB_NUMPAD3,
	CEKB_NUMPAD0,
	CEMB_LEFT,
	CEMB_MIDDLE,
	CEMB_RIGHT,
	CEMB_WHEELUP,
	CEMB_WHEELDOWN,
	CEIB_COUNT
} ceinput_button;

extern bool ceinput_open(void);
extern void ceinput_close(void);

extern bool ceinput_test(ceinput_button button);

extern int ceinput_mouse_offset_x();
extern int ceinput_mouse_offset_y();

extern void ceinput_advance(float elapsed);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_INPUT_H */
