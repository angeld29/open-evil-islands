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
#include <assert.h>

#include "celib.h"
#include "ceinput.h"

static struct {
	bool inited;
	bool buttons[CE_IB_COUNT];
	int mouse_prev_x;
	int mouse_prev_y;
	int mouse_offset_delay;
	int wheel_up_delay;
	int wheel_down_delay;
	ce_vec2 mouse_offset;
} ce_input_inst;

//static ce_input_button keyboard_ascii_map(unsigned char key);
//static ce_input_button keyboard_special_map(int key);
//static ce_input_button mouse_map(int key);

static void keyboard(unsigned char key, int x, int y)
{
	ce_unused(x);
	ce_unused(y);
	//ce_input_inst.buttons[keyboard_ascii_map(key)] = true;
}

static void keyboard_up(unsigned char key, int x, int y)
{
	ce_unused(x);
	ce_unused(y);
	//ce_input_inst.buttons[keyboard_ascii_map(key)] = false;
}

static void special(int key, int x, int y)
{
	ce_unused(x);
	ce_unused(y);
	//ce_input_inst.buttons[keyboard_special_map(key)] = true;
}

static void special_up(int key, int x, int y)
{
	ce_unused(x);
	ce_unused(y);
	//ce_input_inst.buttons[keyboard_special_map(key)] = false;
}

/*static void mouse(int button, int state, int x, int y)
{
	ce_input_inst.mouse_prev_x = x;
	ce_input_inst.mouse_prev_y = y;

	if (GLUT_UP == state) {
		if (GLUT_WHEEL_UP == button) {
			ce_input_inst.wheel_up_delay = 2;
			return;
		}
		if (GLUT_WHEEL_DOWN == button) {
			ce_input_inst.wheel_down_delay = 2;
			return;
		}
	}

	ce_input_inst.buttons[mouse_map(button)] = GLUT_DOWN == state;
}*/

static void motion(int x, int y)
{
	ce_input_inst.mouse_offset_delay = 2;
	ce_vec2_init(&ce_input_inst.mouse_offset, x - ce_input_inst.mouse_prev_x,
												y - ce_input_inst.mouse_prev_y);
	ce_input_inst.mouse_prev_x = x;
	ce_input_inst.mouse_prev_y = y;
}

static void passive_motion(int x, int y)
{
	motion(x, y);
}

bool ce_input_init(void)
{
	assert(!ce_input_inst.inited && "The input subsystem has already been inited");
	ce_input_inst.inited = true;

	return true;
}

void ce_input_term(void)
{
	assert(ce_input_inst.inited && "The input subsystem has not yet been inited");
	ce_input_inst.inited = false;
}

bool ce_input_test(ce_input_button button)
{
	assert(ce_input_inst.inited && "The input subsystem has not yet been inited");
	return ce_input_inst.buttons[button];
}

ce_vec2 ce_input_mouse_offset(void)
{
	assert(ce_input_inst.inited && "The input subsystem has not yet been inited");
	return ce_input_inst.mouse_offset;
}

void ce_input_advance(float elapsed)
{
	assert(ce_input_inst.inited && "The input subsystem has not yet been inited");
	ce_unused(elapsed);

	if (ce_input_inst.mouse_offset_delay > 0 &&
			0 == --ce_input_inst.mouse_offset_delay) {
		ce_input_inst.mouse_offset = CE_VEC2_ZERO;
	}

	if (ce_input_inst.wheel_up_delay > 0 &&
			0 == --ce_input_inst.wheel_up_delay) {
		ce_input_inst.buttons[CE_MB_WHEELUP] = false;
	}

	if (ce_input_inst.wheel_down_delay > 0 &&
			0 == --ce_input_inst.wheel_down_delay) {
		ce_input_inst.buttons[CE_MB_WHEELDOWN] = false;
	}
}

/*static ce_input_button keyboard_ascii_map(unsigned char key)
{
	ce_input_button button = CE_IB_UNKNOWN;
	switch (key) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		button = CE_KB_0 + key - '0';
		break;
	case 'a':
	case 'A':
		button = CE_KB_A;
		break;
	case 'b':
	case 'B':
		button = CE_KB_B;
		break;
	case 'n':
	case 'N':
		button = CE_KB_N;
		break;
	case '-':
		button = CE_KB_SUBTRACT;
		break;
	case '+':
		button = CE_KB_ADD;
		break;
    case 27:
		button = CE_KB_ESCAPE;
		break;
	}
	return button;
}

static ce_input_button keyboard_special_map(int key)
{
	ce_input_button button = CE_IB_UNKNOWN;
	switch (key) {
	case GLUT_KEY_F1:
		button = CE_KB_F1;
		break;
	case GLUT_KEY_F2:
		button = CE_KB_F2;
		break;
	case GLUT_KEY_F3:
		button = CE_KB_F3;
		break;
	case GLUT_KEY_F4:
		button = CE_KB_F4;
		break;
	case GLUT_KEY_F5:
		button = CE_KB_F5;
		break;
	case GLUT_KEY_F6:
		button = CE_KB_F6;
		break;
	case GLUT_KEY_F7:
		button = CE_KB_F7;
		break;
	case GLUT_KEY_F8:
		button = CE_KB_F8;
		break;
	case GLUT_KEY_F9:
		button = CE_KB_F9;
		break;
	case GLUT_KEY_F10:
		button = CE_KB_F10;
		break;
	case GLUT_KEY_F11:
		button = CE_KB_F11;
		break;
	case GLUT_KEY_F12:
		button = CE_KB_F12;
		break;
	case GLUT_KEY_LEFT:
		button = CE_KB_LEFT;
		break;
	case GLUT_KEY_UP:
		button = CE_KB_UP;
		break;
	case GLUT_KEY_RIGHT:
		button = CE_KB_RIGHT;
		break;
	case GLUT_KEY_DOWN:
		button = CE_KB_DOWN;
		break;
	case GLUT_KEY_INSERT:
		button = CE_KB_INSERT;
		break;
	case GLUT_KEY_HOME:
		button = CE_KB_HOME;
		break;
	case GLUT_KEY_END:
		button = CE_KB_END;
		break;
	case GLUT_KEY_PAGE_UP:
		button = CE_KB_PAGEUP;
		break;
	case GLUT_KEY_PAGE_DOWN:
		button = CE_KB_PAGEDOWN;
		break;
	};
	return button;
}

static ce_input_button mouse_map(int key)
{
	ce_input_button button = CE_IB_UNKNOWN;
	switch (key) {
	case GLUT_LEFT_BUTTON:
		button = CE_MB_LEFT;
		break;
	case GLUT_MIDDLE_BUTTON:
		button = CE_MB_MIDDLE;
		break;
	case GLUT_RIGHT_BUTTON:
		button = CE_MB_RIGHT;
		break;
	case GLUT_WHEEL_UP:
		button = CE_MB_WHEELUP;
		break;
	case GLUT_WHEEL_DOWN:
		button = CE_MB_WHEELDOWN;
		break;
	};
	return button;
}*/
