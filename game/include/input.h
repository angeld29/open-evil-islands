#ifndef CE_INPUT_H
#define CE_INPUT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	IB_UNKNOWN,
	KB_ESCAPE,
	KB_F1,
	KB_F2,
	KB_F3,
	KB_F4,
	KB_F5,
	KB_F6,
	KB_F7,
	KB_F8,
	KB_F9,
	KB_F10,
	KB_F11,
	KB_F12,
	KB_TILDE,
	KB_1,
	KB_2,
	KB_3,
	KB_4,
	KB_5,
	KB_6,
	KB_7,
	KB_8,
	KB_9,
	KB_0,
	KB_MINUS,
	KB_EQUALS,
	KB_BACKSLASH,
	KB_BACKSPACE,
	KB_TAB,
	KB_Q,
	KB_W,
	KB_E,
	KB_R,
	KB_T,
	KB_Y,
	KB_U,
	KB_I,
	KB_O,
	KB_P,
	KB_LBRACKET,
	KB_RBRACKET,
	KB_CAPSLOCK,
	KB_A,
	KB_S,
	KB_D,
	KB_F,
	KB_G,
	KB_H,
	KB_J,
	KB_K,
	KB_L,
	KB_SEMICOLON,
	KB_APOSTROPHE,
	KB_ENTER,
	KB_LSHIFT,
	KB_Z,
	KB_X,
	KB_C,
	KB_V,
	KB_B,
	KB_N,
	KB_M,
	KB_COMMA,
	KB_PERIOD,
	KB_SLASH,
	KB_RSHIFT,
	KB_LCONTROL,
	KB_LMETA,
	KB_LALT,
	KB_SPACE,
	KB_RALT,
	KB_RMETA,
	KB_MENU,
	KB_RCONTROL,
	KB_PRINT,
	KB_SCROLLLOCK,
	KB_PAUSE,
	KB_INSERT,
	KB_DELETE,
	KB_HOME,
	KB_END,
	KB_PAGEUP,
	KB_PAGEDOWN,
	KB_LEFT,
	KB_UP,
	KB_RIGHT,
	KB_DOWN,
	KB_NUMLOCK,
	KB_DIVIDE,
	KB_MULTIPLY,
	KB_SUBTRACT,
	KB_ADD,
	KB_NUMPADENTER,
	KB_DECIMAL,
	KB_NUMPAD7,
	KB_NUMPAD8,
	KB_NUMPAD9,
	KB_NUMPAD4,
	KB_NUMPAD5,
	KB_NUMPAD6,
	KB_NUMPAD1,
	KB_NUMPAD2,
	KB_NUMPAD3,
	KB_NUMPAD0,
	MB_LEFT,
	MB_MIDDLE,
	MB_RIGHT,
	MB_WHEELUP,
	MB_WHEELDOWN,
	IB_COUNT
} input_button;

extern void input_open(void);
extern void input_close(void);

extern bool input_test(input_button button);

extern int input_mouse_offset_x();
extern int input_mouse_offset_y();

extern void input_advance(float elapsed);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_INPUT_H */
