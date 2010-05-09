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
#include "ceinput.h"
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
} ce_x11keymap;

typedef struct ce_x11window {
	ce_renderwindow* renderwindow;
	Atom atoms[CE_X11WINDOW_ATOM_COUNT];
	unsigned long mask[CE_X11WINDOW_STATE_COUNT];
	XSetWindowAttributes attrs[CE_X11WINDOW_STATE_COUNT];
	Display* display;
	Window window;
	void (*handlers[LASTEvent])(struct ce_x11window*, XEvent*);
	bool fullscreen; // local state, see renderwindow_minimize
} ce_x11window;

static void ce_renderwindow_handler_skip(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_client_message(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_expose(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_map_notify(ce_x11window*, XEvent*);
static void ce_renderwindow_handler_configure_notify(ce_x11window*, XEvent*);
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

	for (size_t i = 0; i < sizeof(x11window->handlers) /
							sizeof(x11window->handlers[0]); ++i) {
		x11window->handlers[i] = ce_renderwindow_handler_skip;
	}

	x11window->handlers[ClientMessage] = ce_renderwindow_handler_client_message;
	x11window->handlers[Expose] = ce_renderwindow_handler_expose;
	x11window->handlers[MapNotify] = ce_renderwindow_handler_map_notify;
	x11window->handlers[ConfigureNotify] = ce_renderwindow_handler_configure_notify;
	x11window->handlers[KeyPress] = ce_renderwindow_handler_key_press;
	x11window->handlers[KeyRelease] = ce_renderwindow_handler_key_release;
	x11window->handlers[ButtonPress] = ce_renderwindow_handler_button_press;
	x11window->handlers[ButtonRelease] = ce_renderwindow_handler_button_release;
	x11window->handlers[MotionNotify] = ce_renderwindow_handler_motion_notify;

	renderwindow->x = (XDisplayWidth(x11window->display,
		XDefaultScreen(x11window->display)) - width) / 2;
	renderwindow->y = (XDisplayHeight(x11window->display,
		XDefaultScreen(x11window->display)) - height) / 2;

	renderwindow->width = width;
	renderwindow->height = height;

	renderwindow->context = ce_context_create(x11window->display);
	renderwindow->displaymng = ce_displaymng_create(x11window->display,
												renderwindow->context->bpp);

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
		renderwindow->x, renderwindow->y, width, height,
		0, renderwindow->context->visualinfo->depth, InputOutput,
		renderwindow->context->visualinfo->visual,
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

	XMapRaised(x11window->display, x11window->window);

	// x and y values in XCreateWindow are ignored by most windows managers,
	// which means that top-level windows maybe placed somewhere else on the desktop
	XMoveWindow(x11window->display, x11window->window,
		renderwindow->x, renderwindow->y);

	return renderwindow;
}

void ce_renderwindow_del(ce_renderwindow* renderwindow)
{
	if (NULL != renderwindow) {
		ce_context_del(renderwindow->context);
		ce_displaymng_del(renderwindow->displaymng);
		ce_x11window* x11window = (ce_x11window*)renderwindow->impl;
		if (0 != x11window->window) {
			XDestroyWindow(x11window->display, x11window->window);
		}
		if (NULL != x11window->display) {
			XCloseDisplay(x11window->display);
		}
		ce_free(renderwindow, sizeof(ce_renderwindow) + sizeof(ce_x11window));
	}
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

bool ce_renderwindow_pump(ce_renderwindow* renderwindow)
{
	ce_x11window* x11window = (ce_x11window*)renderwindow->impl;

	while (XPending(x11window->display) > 0) {
		XEvent event;
		XNextEvent(x11window->display, &event);

		assert(0 <= event.type && (size_t)event.type < sizeof(x11window->handlers) /
												sizeof(x11window->handlers[0]));
		(*x11window->handlers[event.type])(x11window, &event);
	}

	return true;
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
		;
	}
}

static void ce_renderwindow_handler_expose(ce_x11window* x11window, XEvent* event)
{
	if (0 == event->xexpose.count) {
		// the window was exposed, redraw it
		// TODO: render
	}
}

static void ce_renderwindow_handler_map_notify(ce_x11window* x11window, XEvent* event)
{
	if (x11window->fullscreen) {
		assert(!x11window->renderwindow->fullscreen);
		ce_renderwindow_toggle_fullscreen(x11window->renderwindow);

		x11window->fullscreen = false;
	}
}

static void ce_renderwindow_handler_configure_notify(ce_x11window* x11window, XEvent* event)
{
	// TODO: reshape
	//event->xconfigure.width;
	//event->xconfigure.height;
}

static void ce_renderwindow_handler_key_press(ce_x11window* x11window, XEvent* event)
{
	KeySym key;
	XLookupString(&event->xkey, NULL, 0, &key, NULL);

	if (XK_Escape == key) {
	}

	if (XK_F2 == key) {
		ce_renderwindow_toggle_fullscreen(x11window->renderwindow);
	}

	if (XK_Tab == key && event->xkey.state & Mod1Mask) {
		ce_renderwindow_minimize(x11window->renderwindow);
	}
}

static void ce_renderwindow_handler_key_release(ce_x11window* x11window, XEvent* event)
{
	KeySym key;
	XLookupString(&event->xkey, NULL, 0, &key, NULL);
}

static void ce_renderwindow_handler_button_press(ce_x11window* x11window, XEvent* event)
{
}

static void ce_renderwindow_handler_button_release(ce_x11window* x11window, XEvent* event)
{
}

static void ce_renderwindow_handler_motion_notify(ce_x11window* x11window, XEvent* event)
{
	//event->xmotion.x;
	//event->xmotion.y;
}
