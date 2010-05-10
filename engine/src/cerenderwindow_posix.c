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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cevector.h"
#include "cerenderwindow.h"

#include "cedisplay_posix.h"
#include "cecontext_posix.h"

typedef enum {
	CE_X11WINDOW_ATOM_WM_PROTOCOLS,
	CE_X11WINDOW_ATOM_WM_DELETE_WINDOW,
	CE_X11WINDOW_ATOM_NET_WM_STATE_FULLSCREEN,
	CE_X11WINDOW_ATOM_NET_WM_STATE,
	CE_X11WINDOW_ATOM_COUNT
} ce_x11window_atom;

typedef enum {
	CE_X11WINDOW_STATE_WINDOW,
	CE_X11WINDOW_STATE_FULLSCREEN,
	CE_X11WINDOW_STATE_COUNT
} ce_x11window_state;

typedef struct {
	KeySym keysym;
	ce_input_button button;
} ce_x11keypair;

static ce_x11keypair* ce_x11keypair_new(KeySym keysym, ce_input_button button)
{
	ce_x11keypair* keypair = ce_alloc(sizeof(ce_x11keypair));
	keypair->keysym = keysym;
	keypair->button = button;
	return keypair;
}

static void ce_x11keypair_del(ce_x11keypair* keypair)
{
	ce_free(keypair, sizeof(ce_x11keypair));
}

static int ce_x11keypair_sort_comp(const void* lhs, const void* rhs)
{
	const ce_x11keypair* keypair1 = *(const ce_x11keypair**)lhs;
	const ce_x11keypair* keypair2 = *(const ce_x11keypair**)rhs;
	// be careful: KeySym is something like unsigned...
	return keypair1->keysym < keypair2->keysym ? -1 :
			(int)(keypair1->keysym - keypair2->keysym);
}

static int ce_x11keypair_search_comp(const void* lhs, const void* rhs)
{
	const KeySym* keysym = lhs;
	const ce_x11keypair* keypair = *(const ce_x11keypair**)rhs;
	// be careful: KeySym is something like unsigned...
	return *keysym < keypair->keysym ? -1 : (int)(*keysym - keypair->keysym);
}

static void ce_x11keymap_sort(ce_vector* keymap)
{
	qsort(keymap->items, keymap->count,
		sizeof(ce_x11keypair*), ce_x11keypair_sort_comp);
}

static ce_input_button ce_x11keymap_search(ce_vector* keymap, const KeySym* keysym)
{
	ce_x11keypair** keypair = bsearch(keysym, keymap->items, keymap->count,
		sizeof(ce_x11keypair*), ce_x11keypair_search_comp);
	return NULL != keypair ? (*keypair)->button : CE_IB_UNKNOWN;
}

typedef struct ce_x11window {
	ce_renderwindow* renderwindow;
	Atom atoms[CE_X11WINDOW_ATOM_COUNT];
	unsigned long mask[CE_X11WINDOW_STATE_COUNT];
	XSetWindowAttributes attrs[CE_X11WINDOW_STATE_COUNT];
	Display* display;
	Window window;
	ce_vector* keymap;
	void (*handlers[LASTEvent])(struct ce_x11window*, XEvent*);
	bool fullscreen; // local state, see renderwindow_minimize
} ce_x11window;

static void ce_renderwindow_handler_skip(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_client_message(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_expose(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_map_notify(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_visibility_notify(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_configure_notify(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_enter_notify(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_key_press(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_key_release(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_button_press(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_button_release(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_motion_notify(ce_x11window*, XEvent*);

ce_renderwindow* ce_renderwindow_new(const char* title, int width, int height)
{
	XInitThreads();

	ce_renderwindow* renderwindow = ce_alloc_zero(sizeof(ce_renderwindow) +
													sizeof(ce_x11window));

	ce_x11window* x11window = (ce_x11window*)renderwindow->impl;
	x11window->renderwindow = renderwindow;

	x11window->display = XOpenDisplay(NULL);
	if (NULL == x11window->display) {
		ce_logging_fatal("renderwindow: could not connect to X server");
		ce_renderwindow_del(renderwindow);
		return NULL;
	}

	x11window->atoms[CE_X11WINDOW_ATOM_WM_PROTOCOLS] = XInternAtom(x11window->display, "WM_PROTOCOLS", False);
	x11window->atoms[CE_X11WINDOW_ATOM_WM_DELETE_WINDOW] = XInternAtom(x11window->display, "WM_DELETE_WINDOW", False);
	x11window->atoms[CE_X11WINDOW_ATOM_NET_WM_STATE_FULLSCREEN] = XInternAtom(x11window->display, "_NET_WM_STATE_FULLSCREEN", False);
	x11window->atoms[CE_X11WINDOW_ATOM_NET_WM_STATE] = XInternAtom(x11window->display, "_NET_WM_STATE", False);

	KeySym x11keys[2][CE_IB_COUNT] = {
		// lower
		{ XK_VoidSymbol, XK_Escape, XK_F1, XK_F2, XK_F3, XK_F4, XK_F5, XK_F6,
		XK_F7, XK_F8, XK_F9, XK_F10, XK_F11, XK_F12, XK_grave, XK_0, XK_1,
		XK_2, XK_3, XK_4, XK_5, XK_6, XK_7, XK_8, XK_9, XK_minus, XK_equal,
		XK_backslash, XK_BackSpace, XK_Tab, XK_q, XK_w, XK_e, XK_r, XK_t, XK_y,
		XK_u, XK_i, XK_o, XK_p, XK_bracketleft, XK_bracketright, XK_Caps_Lock,
		XK_a, XK_s, XK_d, XK_f, XK_g, XK_h, XK_j, XK_k, XK_l, XK_semicolon,
		XK_apostrophe, XK_Return, XK_Shift_L, XK_z, XK_x, XK_c, XK_v, XK_b,
		XK_n, XK_m, XK_comma, XK_period, XK_slash, XK_Shift_R, XK_Control_L,
		XK_Super_L, XK_Alt_L, XK_space, XK_Alt_R, XK_Super_R, XK_Menu, XK_Control_R,
		XK_Sys_Req, XK_Scroll_Lock, XK_Pause, XK_Insert, XK_Delete, XK_Home, XK_End,
		XK_Page_Up, XK_Page_Down, XK_Left, XK_Up, XK_Right, XK_Down, XK_Num_Lock,
		XK_KP_Divide, XK_KP_Multiply, XK_KP_Subtract, XK_KP_Add, XK_KP_Enter,
		XK_KP_Delete, XK_KP_Home, XK_KP_Up, XK_KP_Page_Up, XK_KP_Left, XK_KP_Begin,
		XK_KP_Right, XK_KP_End, XK_KP_Down, XK_KP_Page_Down, XK_KP_Insert,
		XK_VoidSymbol, XK_VoidSymbol, XK_VoidSymbol, XK_VoidSymbol, XK_VoidSymbol },
		// upper
		{ XK_VoidSymbol, XK_Escape, XK_F1, XK_F2, XK_F3, XK_F4, XK_F5, XK_F6,
		XK_F7, XK_F8, XK_F9, XK_F10, XK_F11, XK_F12, XK_asciitilde, XK_parenright,
		XK_exclam, XK_at, XK_numbersign, XK_dollar, XK_percent, XK_asciicircum,
		XK_ampersand, XK_asterisk, XK_parenleft, XK_underscore, XK_plus, XK_bar,
		XK_BackSpace, XK_Tab, XK_Q, XK_W, XK_E, XK_R, XK_T, XK_Y, XK_U,
		XK_I, XK_O, XK_P, XK_braceleft, XK_braceright, XK_Caps_Lock, XK_A, XK_S,
		XK_D, XK_F, XK_G, XK_H, XK_J, XK_K, XK_L, XK_colon, XK_quotedbl, XK_Return,
		XK_Shift_L, XK_Z, XK_X, XK_C, XK_V, XK_B, XK_N, XK_M, XK_less, XK_greater,
		XK_question, XK_Shift_R, XK_Control_L, XK_Super_L, XK_Alt_L, XK_space,
		XK_Alt_R, XK_Super_R, XK_Menu, XK_Control_R, XK_Print, XK_Scroll_Lock,
		XK_Pause, XK_Insert, XK_Delete, XK_Home, XK_End, XK_Page_Up, XK_Page_Down,
		XK_Left, XK_Up, XK_Right, XK_Down, XK_Num_Lock, XK_KP_Divide, XK_KP_Multiply,
		XK_KP_Subtract, XK_KP_Add, XK_KP_Enter, XK_KP_Decimal, XK_KP_7, XK_KP_8,
		XK_KP_9, XK_KP_4, XK_KP_5, XK_KP_6, XK_KP_1, XK_KP_2, XK_KP_3, XK_KP_0,
		XK_VoidSymbol, XK_VoidSymbol, XK_VoidSymbol, XK_VoidSymbol, XK_VoidSymbol },
	};

	// absolutely don't understand how XChangeKeyboardMapping work...
	x11window->keymap = ce_vector_new_reserved(2 * CE_IB_COUNT);

	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < CE_IB_COUNT; ++j) {
			ce_vector_push_back(x11window->keymap,
				ce_x11keypair_new(x11keys[i][j], j));
		}
	}

	ce_x11keymap_sort(x11window->keymap);

	for (int i = 0; i < LASTEvent; ++i) {
		x11window->handlers[i] = ce_renderwindow_handler_skip;
	}

	x11window->handlers[ClientMessage] = ce_renderwindow_handler_client_message;
	x11window->handlers[Expose] = ce_renderwindow_handler_expose;
	x11window->handlers[MapNotify] = ce_renderwindow_handler_map_notify;
	x11window->handlers[VisibilityNotify] = ce_renderwindow_handler_visibility_notify;
	x11window->handlers[ConfigureNotify] = ce_renderwindow_handler_configure_notify;
	x11window->handlers[EnterNotify] = ce_renderwindow_handler_enter_notify;
	x11window->handlers[KeyPress] = ce_renderwindow_handler_key_press;
	x11window->handlers[KeyRelease] = ce_renderwindow_handler_key_release;
	x11window->handlers[ButtonPress] = ce_renderwindow_handler_button_press;
	x11window->handlers[ButtonRelease] = ce_renderwindow_handler_button_release;
	x11window->handlers[MotionNotify] = ce_renderwindow_handler_motion_notify;

	renderwindow->width = width;
	renderwindow->height = height;

	renderwindow->context = ce_context_create(x11window->display);
	renderwindow->displaymng = ce_displaymng_create(x11window->display,
												renderwindow->context->bpp);
	renderwindow->input_context = ce_input_context_new();
	renderwindow->listeners = ce_vector_new();

	renderwindow->bpp = renderwindow->context->bpp;
	renderwindow->rate = 0;
	renderwindow->rotation = CE_DISPLAY_ROTATION_NONE;
	renderwindow->reflection = CE_DISPLAY_REFLECTION_NONE;

	for (int i = 0; i < CE_X11WINDOW_STATE_COUNT; ++i) {
		x11window->mask[i] = CWColormap | CWEventMask;
		x11window->attrs[i].colormap = XCreateColormap(x11window->display,
			XDefaultRootWindow(x11window->display),
			renderwindow->context->visualinfo->visual, AllocNone);
		x11window->attrs[i].event_mask = ExposureMask | EnterWindowMask |
			KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
			PointerMotionMask | ButtonMotionMask | FocusChangeMask |
			VisibilityChangeMask | StructureNotifyMask | LeaveWindowMask;
		if (CE_X11WINDOW_STATE_FULLSCREEN == i) {
			x11window->mask[i] |= CWOverrideRedirect;
			x11window->attrs[i].override_redirect = True;
		}
	}

	x11window->window = XCreateWindow(x11window->display,
		XDefaultRootWindow(x11window->display),
		0, 0, width, height, 0, renderwindow->context->visualinfo->depth,
		InputOutput, renderwindow->context->visualinfo->visual,
		x11window->mask[CE_X11WINDOW_STATE_WINDOW],
		&x11window->attrs[CE_X11WINDOW_STATE_WINDOW]);

	if (!ce_context_make_current(renderwindow->context, x11window->display,
														x11window->window)) {
		ce_logging_fatal("renderwindow: could not set context");
		ce_renderwindow_del(renderwindow);
		return NULL;
	}

	XSetStandardProperties(x11window->display, x11window->window,
		title, title, None, NULL, 0, NULL);

	// handle wm_delete_events
	XSetWMProtocols(x11window->display, x11window->window,
		&x11window->atoms[CE_X11WINDOW_ATOM_WM_DELETE_WINDOW], 1);

	return renderwindow;
}

void ce_renderwindow_del(ce_renderwindow* renderwindow)
{
	if (NULL != renderwindow) {
		ce_vector_for_each(renderwindow->listeners, ce_renderwindow_listener_tuple_del);
		ce_vector_del(renderwindow->listeners);
		ce_input_context_del(renderwindow->input_context);
		ce_context_del(renderwindow->context);
		ce_displaymng_del(renderwindow->displaymng);

		ce_x11window* x11window = (ce_x11window*)renderwindow->impl;
		ce_vector_for_each(x11window->keymap, ce_x11keypair_del);
		ce_vector_del(x11window->keymap);

		if (0 != x11window->window) {
			XDestroyWindow(x11window->display, x11window->window);
		}
		if (NULL != x11window->display) {
			XCloseDisplay(x11window->display);
		}
		ce_free(renderwindow, sizeof(ce_renderwindow) + sizeof(ce_x11window));
	}
}

void ce_renderwindow_show(ce_renderwindow* renderwindow)
{
	ce_x11window* x11window = (ce_x11window*)renderwindow->impl;

	XMapRaised(x11window->display, x11window->window);

	// x and y values in XCreateWindow are ignored by most windows managers,
	// which means that top-level windows maybe placed somewhere else on the desktop
	XMoveWindow(x11window->display, x11window->window,
		(XDisplayWidth(x11window->display,
		XDefaultScreen(x11window->display)) - renderwindow->width) / 2,
		(XDisplayHeight(x11window->display,
		XDefaultScreen(x11window->display)) - renderwindow->height) / 2);
}

void ce_renderwindow_toggle_fullscreen(ce_renderwindow* renderwindow)
{
	ce_x11window* x11window = (ce_x11window*)renderwindow->impl;

	bool fullscreen = !renderwindow->fullscreen;

	XChangeWindowAttributes(x11window->display, x11window->window,
		x11window->mask[fullscreen], &x11window->attrs[fullscreen]);

	if (!renderwindow->fullscreen && fullscreen) {
		XGrabPointer(x11window->display, x11window->window, True, NoEventMask,
			GrabModeAsync, GrabModeAsync, x11window->window, None, CurrentTime);
		XGrabKeyboard(x11window->display, x11window->window,
			True, GrabModeAsync, GrabModeAsync, CurrentTime);
	}

	if (renderwindow->fullscreen && !fullscreen) {
		XUngrabPointer(x11window->display, CurrentTime);
		XUngrabKeyboard(x11window->display, CurrentTime);
	}

	renderwindow->fullscreen = fullscreen;

	if (fullscreen) {
		ce_displaymng_change(renderwindow->displaymng,
			renderwindow->width, renderwindow->height,
			renderwindow->bpp, renderwindow->rate,
			renderwindow->rotation, renderwindow->reflection);
	} else {
		ce_displaymng_restore(renderwindow->displaymng);
	}

	XEvent event;
	memset(&event, 0, sizeof(event));

	event.type = ClientMessage;
	event.xclient.send_event = True;
	event.xclient.window = x11window->window;
	event.xclient.message_type = x11window->atoms[CE_X11WINDOW_ATOM_NET_WM_STATE];
	event.xclient.format = 32;
	event.xclient.data.l[0] = fullscreen;
	event.xclient.data.l[1] = x11window->atoms[CE_X11WINDOW_ATOM_NET_WM_STATE_FULLSCREEN];

	// absolutely don't understand how event masks work...
	XSendEvent(x11window->display, XDefaultRootWindow(x11window->display),
		False, SubstructureNotifyMask, &event);
}

void ce_renderwindow_minimize(ce_renderwindow* renderwindow)
{
	ce_x11window* x11window = (ce_x11window*)renderwindow->impl;

	if (renderwindow->fullscreen) {
		// always exit from fullscreen before minimizing!
		ce_renderwindow_toggle_fullscreen(renderwindow);

		// save fullscreen state to restore later (see Map Event)
		assert(!x11window->fullscreen);
		x11window->fullscreen = true;
	}

	XIconifyWindow(x11window->display,
		x11window->window, XDefaultScreen(x11window->display));
}

void ce_renderwindow_pump(ce_renderwindow* renderwindow)
{
	ce_x11window* x11window = (ce_x11window*)renderwindow->impl;
	ce_input_context* input_context = x11window->renderwindow->input_context;

	// reset pointer offset every frame
	input_context->pointer_offset = CE_VEC2_ZERO;

	// special case: reset wheel buttons, see ButtonRelease event
	input_context->buttons[CE_MB_WHEELUP] = false;
	input_context->buttons[CE_MB_WHEELDOWN] = false;

	while (XPending(x11window->display) > 0) {
		XEvent event;
		XNextEvent(x11window->display, &event);

		// is it possible? may be new version of the X server...
		assert(0 <= event.type && event.type < LASTEvent);

		// just in case
		if (event.type < LASTEvent) {
			(*x11window->handlers[event.type])(x11window, &event);
		}
	}
}

static void ce_renderwindow_handler_skip(ce_x11window* x11window, XEvent* event)
{
	ce_unused(x11window);
	ce_unused(event);
}

static void ce_renderwindow_handler_client_message(ce_x11window* x11window, XEvent* event)
{
	if (x11window->atoms[CE_X11WINDOW_ATOM_WM_PROTOCOLS] ==
			event->xclient.message_type &&
			x11window->atoms[CE_X11WINDOW_ATOM_WM_DELETE_WINDOW] ==
			(Atom)event->xclient.data.l[0]) {
		ce_renderwindow_emit_closed(x11window->renderwindow);
	}
}

static void ce_renderwindow_handler_expose(ce_x11window* x11window, XEvent* event)
{
	if (0 == event->xexpose.count) {
		// the window was exposed, redraw it
		ce_renderwindow_emit_exposed(x11window->renderwindow);
	}
}

static void ce_renderwindow_handler_map_notify(ce_x11window* x11window, XEvent* event)
{
	ce_unused(event);

	if (x11window->fullscreen) {
		assert(!x11window->renderwindow->fullscreen);
		ce_renderwindow_toggle_fullscreen(x11window->renderwindow);
		x11window->fullscreen = false;
	}
}

static void ce_renderwindow_handler_visibility_notify(ce_x11window* x11window, XEvent* event)
{
	ce_input_context* input_context = x11window->renderwindow->input_context;

	if (VisibilityUnobscured == event->xvisibility.state) {
		Window window;
		unsigned int mask;
		int dummy, x, y;

		XQueryPointer(event->xvisibility.display, event->xvisibility.window,
			&window, &window, &dummy, &dummy, &x, &y, &mask);

		input_context->pointer_position.x = x;
		input_context->pointer_position.y = y;
	}

	if (VisibilityFullyObscured != event->xvisibility.state) {
		memset(input_context->buttons, 0, sizeof(input_context->buttons));
	}
}

static void ce_renderwindow_handler_configure_notify(ce_x11window* x11window, XEvent* event)
{
	x11window->renderwindow->width = event->xconfigure.width;
	x11window->renderwindow->height = event->xconfigure.height;
}

static void ce_renderwindow_handler_enter_notify(ce_x11window* x11window, XEvent* event)
{
	ce_input_context* input_context = x11window->renderwindow->input_context;
	input_context->pointer_position.x = event->xcrossing.x;
	input_context->pointer_position.y = event->xcrossing.y;
}

static void ce_renderwindow_handler_key_press(ce_x11window* x11window, XEvent* event)
{
	ce_input_context* input_context = x11window->renderwindow->input_context;
	input_context->pointer_position.x = event->xkey.x;
	input_context->pointer_position.y = event->xkey.y;

	KeySym key;
	XLookupString(&event->xkey, NULL, 0, &key, NULL);

	input_context->buttons[ce_x11keymap_search(x11window->keymap, &key)] = true;
}

static void ce_renderwindow_handler_key_release(ce_x11window* x11window, XEvent* event)
{
	ce_input_context* input_context = x11window->renderwindow->input_context;
	input_context->pointer_position.x = event->xkey.x;
	input_context->pointer_position.y = event->xkey.y;

	KeySym key;
	XLookupString(&event->xkey, NULL, 0, &key, NULL);

	input_context->buttons[ce_x11keymap_search(x11window->keymap, &key)] = false;
}

static void ce_renderwindow_handler_button_press(ce_x11window* x11window, XEvent* event)
{
	ce_input_context* input_context = x11window->renderwindow->input_context;
	input_context->pointer_position.x = event->xbutton.x;
	input_context->pointer_position.y = event->xbutton.y;

	input_context->buttons[event->xbutton.button - 1 + CE_MB_LEFT] = true;
}

static void ce_renderwindow_handler_button_release(ce_x11window* x11window, XEvent* event)
{
	ce_input_context* input_context = x11window->renderwindow->input_context;
	input_context->pointer_position.x = event->xbutton.x;
	input_context->pointer_position.y = event->xbutton.y;

	// special case: ignore wheel buttons, see renderwindow_pump
	// ButtonRelease event arrives immediately after ButtonPress event
	if (Button4 == event->xbutton.button || Button5 == event->xbutton.button) {
		return;
	}

	input_context->buttons[event->xbutton.button - 1 + CE_MB_LEFT] = false;
}

static void ce_renderwindow_handler_motion_notify(ce_x11window* x11window, XEvent* event)
{
	ce_input_context* input_context = x11window->renderwindow->input_context;
	input_context->pointer_offset.x = event->xmotion.x - input_context->pointer_position.x;
	input_context->pointer_offset.y = event->xmotion.y - input_context->pointer_position.y;
	input_context->pointer_position.x = event->xmotion.x;
	input_context->pointer_position.y = event->xmotion.y;
}
