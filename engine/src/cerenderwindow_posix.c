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

#include "cealloc.h"
#include "celogging.h"
#include "cerenderwindow.h"

#include "cedisplay_posix.h"
#include "cecontext_posix.h"

typedef enum {
	CE_X11WINDOW_ATOM_PROTOCOLS,
	CE_X11WINDOW_ATOM_DELETE_WINDOW,
	CE_X11WINDOW_ATOM_STATE_FULLSCREEN,
	CE_X11WINDOW_ATOM_STATE,
	CE_X11WINDOW_ATOM_COUNT
} ce_x11window_atom;

typedef enum {
	CE_X11WINDOW_STATE_WINDOW,
	CE_X11WINDOW_STATE_FULLSCREEN,
	CE_X11WINDOW_STATE_COUNT
} ce_x11window_state;

typedef struct {
	Atom atoms[CE_X11WINDOW_ATOM_COUNT];
	unsigned long mask[CE_X11WINDOW_STATE_COUNT];
	XSetWindowAttributes attrs[CE_X11WINDOW_STATE_COUNT];
	Display* display;
	Window window;
} ce_x11window;

ce_renderwindow* ce_renderwindow_new(void)
{
	XInitThreads();

	ce_renderwindow* renderwindow = ce_alloc_zero(sizeof(ce_renderwindow) +
													sizeof(ce_x11window));
	ce_x11window* x11window = (ce_x11window*)renderwindow->impl;

	renderwindow->width = 1024;
	renderwindow->height = 768;
	renderwindow->fullscreen = false;

	x11window->display = XOpenDisplay(NULL);
	x11window->atoms[CE_X11WINDOW_ATOM_PROTOCOLS] = XInternAtom(x11window->display, "WM_PROTOCOLS", False);
	x11window->atoms[CE_X11WINDOW_ATOM_DELETE_WINDOW] = XInternAtom(x11window->display, "WM_DELETE_WINDOW", False);
	x11window->atoms[CE_X11WINDOW_ATOM_STATE_FULLSCREEN] = XInternAtom(x11window->display, "_NET_WM_STATE_FULLSCREEN", False);
	x11window->atoms[CE_X11WINDOW_ATOM_STATE] = XInternAtom(x11window->display, "_NET_WM_STATE", False);

	renderwindow->context = ce_context_create(x11window->display);
	renderwindow->displaymng = ce_displaymng_create(x11window->display,
												renderwindow->context->bpp);

	ce_logging_write("renderwindow: resolution %dx%d",
		renderwindow->width, renderwindow->height);

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
		100, 100, renderwindow->width, renderwindow->height,
		0, renderwindow->context->visualinfo->depth, InputOutput,
		renderwindow->context->visualinfo->visual,
		x11window->mask[CE_X11WINDOW_STATE_WINDOW],
		&x11window->attrs[CE_X11WINDOW_STATE_WINDOW]);

	// set window title
	const char* title = "stub";
	XSetStandardProperties(x11window->display, x11window->window,
		title, title, None, NULL, 0, NULL);

	// handle wm_delete_events
	XSetWMProtocols(x11window->display, x11window->window,
		&x11window->atoms[CE_X11WINDOW_ATOM_DELETE_WINDOW], 1);

	XMapRaised(x11window->display, x11window->window);

	ce_context_make_current(renderwindow->context, x11window->display,
													x11window->window);

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

static void ce_renderwindow_switch(ce_renderwindow* renderwindow,
									int width, int height, bool fullscreen)
{
	ce_x11window* x11window = (ce_x11window*)renderwindow->impl;

	if (renderwindow->fullscreen != fullscreen) {
		XChangeWindowAttributes(x11window->display, x11window->window,
			x11window->mask[fullscreen], &x11window->attrs[fullscreen]);
	}

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

	if (fullscreen) {
		ce_displaymng_change(renderwindow->displaymng, width, height,
			0, 0, CE_DISPLAY_ROTATION_NONE, CE_DISPLAY_REFLECTION_NONE);
	} else {
		ce_displaymng_restore(renderwindow->displaymng);
	}

	XMoveResizeWindow(x11window->display, x11window->window, 0, 0, width, height);

	XEvent message;
	memset(&message, '\0', sizeof(message));

	message.type = ClientMessage;
	message.xclient.send_event = True;
	message.xclient.window = x11window->window;
	message.xclient.message_type = x11window->atoms[CE_X11WINDOW_ATOM_STATE];
	message.xclient.format = 32;
	message.xclient.data.l[0] = fullscreen;
	message.xclient.data.l[1] = x11window->atoms[CE_X11WINDOW_ATOM_STATE_FULLSCREEN];

	XSendEvent(x11window->display, XDefaultRootWindow(x11window->display),
		False, SubstructureRedirectMask | SubstructureNotifyMask, &message);

	renderwindow->fullscreen = fullscreen;
	renderwindow->width = width;
	renderwindow->height = height;
}

void ce_renderwindow_minimize(ce_renderwindow* renderwindow)
{
	ce_x11window* x11window = (ce_x11window*)renderwindow->impl;

	// TODO: send fullscreen toggle event
	XIconifyWindow(x11window->display,
		x11window->window, XDefaultScreen(x11window->display));
}

bool ce_renderwindow_pump(ce_renderwindow* renderwindow)
{
	ce_x11window* x11window = (ce_x11window*)renderwindow->impl;
	KeySym key;

	while (XPending(x11window->display) > 0) {
		XEvent event;
		XNextEvent(x11window->display, &event);

		switch (event.type) {
		case ClientMessage:
			if (x11window->atoms[CE_X11WINDOW_ATOM_PROTOCOLS] ==
					event.xclient.message_type &&
					x11window->atoms[CE_X11WINDOW_ATOM_DELETE_WINDOW] ==
					(Atom)event.xclient.data.l[0]) {
				return false;
			}
			break;
		case MapNotify:
			break;
		case Expose:
			if (0 == event.xexpose.count) {
				// the window was exposed, redraw it
				// TODO: render
				//ce_renderwindow_swap(renderwindow);
			}
			break;
		case ConfigureNotify:
			// TODO: reshape
			//event.xconfigure.width;
			//event.xconfigure.height;
			break;
		case KeyPress:
			XLookupString(&event.xkey, NULL, 0, &key, NULL);
			// TODO: key press
			if (XK_Escape == key) {
				return false;
			}
			if (XK_F2 == key) {
				if (renderwindow->fullscreen) {
					ce_renderwindow_switch(renderwindow, 1024, 768, false);
				} else {
					ce_renderwindow_switch(renderwindow, 1024, 768, true);
				}
			}
			break;
		case KeyRelease:
			XLookupString(&event.xkey, NULL, 0, &key, NULL);
			// TODO: key release
			break;
		case MotionNotify:
			//event.xmotion.x;
			//event.xmotion.y;
			break;
		case ButtonPress:
			//event.xbutton.button;
			break;
		case ButtonRelease:
			//event.xbutton.button;
			break;
		}
	}

	return true;
}
