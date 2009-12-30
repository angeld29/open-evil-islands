#include <assert.h>

#include <GL/glut.h>

// Wheel mouse event.
#ifndef GLUT_WHEEL_UP
#define GLUT_WHEEL_UP 3
#endif
#ifndef GLUT_WHEEL_DOWN
#define GLUT_WHEEL_DOWN 4
#endif

#include "input.h"

static bool opened;
static bool buttons[BUTTON_COUNT];
static int mouse_prev_x;
static int mouse_prev_y;
static int mouse_offset_x;
static int mouse_offset_y;

static input_button keyboard_ascii_map(unsigned char key);
static input_button keyboard_special_map(int key);
static input_button mouse_map(int key);

static void keyboard(unsigned char key, int x, int y)
{
	buttons[keyboard_ascii_map(key)] = true;
	x = x;
	y = y;
}

static void keyboard_up(unsigned char key, int x, int y)
{
	buttons[keyboard_ascii_map(key)] = false;
	x = x;
	y = y;
}

static void special(int key, int x, int y)
{
	buttons[keyboard_special_map(key)] = true;
	x = x;
	y = y;
}

static void special_up(int key, int x, int y)
{
	buttons[keyboard_special_map(key)] = false;
	x = x;
	y = y;
}

static void mouse(int button, int state, int x, int y)
{
	buttons[mouse_map(button)] = GLUT_DOWN == state;
	mouse_prev_x = x;
	mouse_prev_y = y;
}

static void motion(int x, int y)
{
	mouse_offset_x = x - mouse_prev_x;
	mouse_offset_y = y - mouse_prev_y;
	mouse_prev_x = x;
	mouse_prev_y = y;
}

static void passive_motion(int x, int y)
{
	motion(x, y);
}

void input_open(void)
{
	assert(!opened);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboard_up);
	glutSpecialFunc(special);
	glutSpecialUpFunc(special_up);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passive_motion);
	opened = true;
}

void input_close(void)
{
	assert(opened);
	opened = false;
}

bool input_test(input_button button)
{
	assert(opened);
	return buttons[button];
}

int input_mouse_offset_x()
{
	assert(opened);
	return mouse_offset_x;
}

int input_mouse_offset_y()
{
	assert(opened);
	return mouse_offset_y;
}

void input_advance(float elapsed)
{
	assert(opened);
	elapsed = elapsed;
}

static input_button keyboard_ascii_map(unsigned char key)
{
	input_button button = BUTTON_UNKNOWN;
	switch (key) {
    case 27:
		button = KB_ESCAPE;
		break;
	}
	return button;
}

static input_button keyboard_special_map(int key)
{
	input_button button = BUTTON_UNKNOWN;
	switch (key) {
	case GLUT_KEY_F1:
		button = KB_F1;
		break;
	case GLUT_KEY_F2:
		button = KB_F2;
		break;
	case GLUT_KEY_F3:
		button = KB_F3;
		break;
	case GLUT_KEY_F4:
		button = KB_F4;
		break;
	case GLUT_KEY_F5:
		button = KB_F5;
		break;
	case GLUT_KEY_F6:
		button = KB_F6;
		break;
	case GLUT_KEY_F7:
		button = KB_F7;
		break;
	case GLUT_KEY_F8:
		button = KB_F8;
		break;
	case GLUT_KEY_F9:
		button = KB_F9;
		break;
	case GLUT_KEY_F10:
		button = KB_F10;
		break;
	case GLUT_KEY_F11:
		button = KB_F11;
		break;
	case GLUT_KEY_F12:
		button = KB_F12;
		break;
	case GLUT_KEY_LEFT:
		button = KB_LEFT;
		break;
	case GLUT_KEY_UP:
		button = KB_UP;
		break;
	case GLUT_KEY_RIGHT:
		button = KB_RIGHT;
		break;
	case GLUT_KEY_DOWN:
		button = KB_DOWN;
		break;
	case GLUT_KEY_INSERT:
		button = KB_INSERT;
		break;
	case GLUT_KEY_HOME:
		button = KB_HOME;
		break;
	case GLUT_KEY_END:
		button = KB_END;
		break;
	case GLUT_KEY_PAGE_UP:
		button = KB_PAGEUP;
		break;
	case GLUT_KEY_PAGE_DOWN:
		button = KB_PAGEDOWN;
		break;
	};
	return button;
}

static input_button mouse_map(int key)
{
	input_button button = BUTTON_UNKNOWN;
	switch (key) {
	case GLUT_LEFT_BUTTON:
		button = MB_LEFT;
		break;
	case GLUT_MIDDLE_BUTTON:
		button = MB_MIDDLE;
		break;
	case GLUT_RIGHT_BUTTON:
		button = MB_RIGHT;
		break;
	case GLUT_WHEEL_UP:
		button = MB_WHEELUP;
		break;
	case GLUT_WHEEL_DOWN:
		button = MB_WHEELDOWN;
		break;
	};
	return button;
}
