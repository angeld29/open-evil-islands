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

#include <assert.h>

#include <GL/glut.h>

// Wheel mouse event.
#ifndef GLUT_WHEEL_UP
#define GLUT_WHEEL_UP 3
#endif
#ifndef GLUT_WHEEL_DOWN
#define GLUT_WHEEL_DOWN 4
#endif

#include "celib.h"
#include "ceinput.h"

static bool opened;
static bool buttons[CEIB_COUNT];
static int mouse_prev_x;
static int mouse_prev_y;
static int mouse_offset_x;
static int mouse_offset_y;
static int mouse_offset_delay;
static int wheel_up_delay;
static int wheel_down_delay;

static ceinput_button keyboard_ascii_map(unsigned char key);
static ceinput_button keyboard_special_map(int key);
static ceinput_button mouse_map(int key);

static void keyboard(unsigned char key, int x, int y)
{
	ceunused(x);
	ceunused(y);
	buttons[keyboard_ascii_map(key)] = true;
}

static void keyboard_up(unsigned char key, int x, int y)
{
	ceunused(x);
	ceunused(y);
	buttons[keyboard_ascii_map(key)] = false;
}

static void special(int key, int x, int y)
{
	ceunused(x);
	ceunused(y);
	buttons[keyboard_special_map(key)] = true;
}

static void special_up(int key, int x, int y)
{
	ceunused(x);
	ceunused(y);
	buttons[keyboard_special_map(key)] = false;
}

static void mouse(int button, int state, int x, int y)
{
	mouse_prev_x = x;
	mouse_prev_y = y;
	if (GLUT_UP == state) {
		if (GLUT_WHEEL_UP == button) {
			wheel_up_delay = 2;
			return;
		}
		if (GLUT_WHEEL_DOWN == button) {
			wheel_down_delay = 2;
			return;
		}
	}
	buttons[mouse_map(button)] = GLUT_DOWN == state;
}

static void motion(int x, int y)
{
	mouse_offset_delay = 2;
	mouse_offset_x = x - mouse_prev_x;
	mouse_offset_y = y - mouse_prev_y;
	mouse_prev_x = x;
	mouse_prev_y = y;
}

static void passive_motion(int x, int y)
{
	motion(x, y);
}

bool ceinput_open(void)
{
	assert(!opened);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboard_up);
	glutSpecialFunc(special);
	glutSpecialUpFunc(special_up);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passive_motion);
	return opened = true;
}

void ceinput_close(void)
{
	assert(opened);
	opened = false;
}

bool ceinput_test(ceinput_button button)
{
	assert(opened);
	return buttons[button];
}

int ceinput_mouse_offset_x()
{
	assert(opened);
	return mouse_offset_x;
}

int ceinput_mouse_offset_y()
{
	assert(opened);
	return mouse_offset_y;
}

void ceinput_advance(float elapsed)
{
	assert(opened);
	ceunused(elapsed);

	if (mouse_offset_delay > 0 && 0 == --mouse_offset_delay) {
		mouse_offset_x = 0.0f;
		mouse_offset_y = 0.0f;
	}

	if (wheel_up_delay > 0 && 0 == --wheel_up_delay) {
		buttons[CEMB_WHEELUP] = false;
	}

	if (wheel_down_delay > 0 && 0 == --wheel_down_delay) {
		buttons[CEMB_WHEELDOWN] = false;
	}
}

static ceinput_button keyboard_ascii_map(unsigned char key)
{
	ceinput_button button = CEIB_UNKNOWN;
	switch (key) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		button = CEKB_0 + key - '0';
		break;
    case 27:
		button = CEKB_ESCAPE;
		break;
	}
	return button;
}

static ceinput_button keyboard_special_map(int key)
{
	ceinput_button button = CEIB_UNKNOWN;
	switch (key) {
	case GLUT_KEY_F1:
		button = CEKB_F1;
		break;
	case GLUT_KEY_F2:
		button = CEKB_F2;
		break;
	case GLUT_KEY_F3:
		button = CEKB_F3;
		break;
	case GLUT_KEY_F4:
		button = CEKB_F4;
		break;
	case GLUT_KEY_F5:
		button = CEKB_F5;
		break;
	case GLUT_KEY_F6:
		button = CEKB_F6;
		break;
	case GLUT_KEY_F7:
		button = CEKB_F7;
		break;
	case GLUT_KEY_F8:
		button = CEKB_F8;
		break;
	case GLUT_KEY_F9:
		button = CEKB_F9;
		break;
	case GLUT_KEY_F10:
		button = CEKB_F10;
		break;
	case GLUT_KEY_F11:
		button = CEKB_F11;
		break;
	case GLUT_KEY_F12:
		button = CEKB_F12;
		break;
	case GLUT_KEY_LEFT:
		button = CEKB_LEFT;
		break;
	case GLUT_KEY_UP:
		button = CEKB_UP;
		break;
	case GLUT_KEY_RIGHT:
		button = CEKB_RIGHT;
		break;
	case GLUT_KEY_DOWN:
		button = CEKB_DOWN;
		break;
	case GLUT_KEY_INSERT:
		button = CEKB_INSERT;
		break;
	case GLUT_KEY_HOME:
		button = CEKB_HOME;
		break;
	case GLUT_KEY_END:
		button = CEKB_END;
		break;
	case GLUT_KEY_PAGE_UP:
		button = CEKB_PAGEUP;
		break;
	case GLUT_KEY_PAGE_DOWN:
		button = CEKB_PAGEDOWN;
		break;
	};
	return button;
}

static ceinput_button mouse_map(int key)
{
	ceinput_button button = CEIB_UNKNOWN;
	switch (key) {
	case GLUT_LEFT_BUTTON:
		button = CEMB_LEFT;
		break;
	case GLUT_MIDDLE_BUTTON:
		button = CEMB_MIDDLE;
		break;
	case GLUT_RIGHT_BUTTON:
		button = CEMB_RIGHT;
		break;
	case GLUT_WHEEL_UP:
		button = CEMB_WHEELUP;
		break;
	case GLUT_WHEEL_DOWN:
		button = CEMB_WHEELDOWN;
		break;
	};
	return button;
}
