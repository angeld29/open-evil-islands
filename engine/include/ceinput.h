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

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include "cevec2.h"
#include "cevector.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
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
} ce_inputbutton;

enum {
	CE_INPUT_NO_DELAY,
	CE_INPUT_DEFAULT_DELAY = 450,
};

enum {
	CE_INPUT_NO_RATE,
	CE_INPUT_DEFAULT_RATE = 25,
};

// level 0 input API - available through window

typedef struct {
	bool buttons[CE_IB_COUNT];
	ce_vec2 pointer_position;
	ce_vec2 pointer_offset;
} ce_inputcontext;

extern ce_inputcontext* ce_inputcontext_new(void);
extern void ce_inputcontext_del(ce_inputcontext* inputcontext);

extern void ce_inputcontext_clear(ce_inputcontext* inputcontext);

// level 1 input API - events

typedef struct ce_inputevent ce_inputevent;

typedef struct {
	size_t size;
	void (*ctor)(ce_inputevent* inputevent, va_list args);
	void (*dtor)(ce_inputevent* inputevent);
	void (*advance)(ce_inputevent* inputevent, float elapsed);
} ce_inputevent_vtable;

struct ce_inputevent {
	bool triggered;
	ce_inputevent_vtable vtable;
	char impl[];
};

typedef struct {
	const ce_inputcontext* inputcontext;
	ce_vector* inputevents;
} ce_inputsupply;

extern ce_inputsupply* ce_inputsupply_new(const ce_inputcontext* inputcontext);
extern void ce_inputsupply_del(ce_inputsupply* inputsupply);

extern void ce_inputsupply_advance(ce_inputsupply* inputsupply, float elapsed);

extern ce_inputevent* ce_inputsupply_button(ce_inputsupply* inputsupply,
											ce_inputbutton inputbutton);

extern ce_inputevent* ce_inputsupply_single_front(ce_inputsupply* inputsupply,
												const ce_inputevent* inputevent);

extern ce_inputevent* ce_inputsupply_single_back(ce_inputsupply* inputsupply,
												const ce_inputevent* inputevent);

extern ce_inputevent* ce_inputsupply_and2(ce_inputsupply* inputsupply,
											const ce_inputevent* inputevent1,
											const ce_inputevent* inputevent2);

extern ce_inputevent* ce_inputsupply_and3(ce_inputsupply* inputsupply,
											const ce_inputevent* inputevent1,
											const ce_inputevent* inputevent2,
											const ce_inputevent* inputevent3);

extern ce_inputevent* ce_inputsupply_or2(ce_inputsupply* inputsupply,
											const ce_inputevent* inputevent1,
											const ce_inputevent* inputevent2);

extern ce_inputevent* ce_inputsupply_or3(ce_inputsupply* inputsupply,
											const ce_inputevent* inputevent1,
											const ce_inputevent* inputevent2,
											const ce_inputevent* inputevent3);

extern ce_inputevent* ce_inputsupply_repeat(ce_inputsupply* inputsupply,
										const ce_inputevent* inputevent,
										int delay /* ms */,
										int rate /* tps */); // triggers per second

// level 2 input API - shortcuts

extern ce_inputevent* ce_inputsupply_shortcut(ce_inputsupply* inputsupply,
												const char* key_sequence);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_INPUT_H */
