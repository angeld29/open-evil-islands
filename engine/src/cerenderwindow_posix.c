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

#include <GL/glx.h>

#include "cegl.h"
#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cedisplay.h"
#include "cerenderwindow.h"

typedef struct {
	int error_base, event_base;
	int major_version, minor_version;
	XVisualInfo* visualinfo;
	GLXContext context;
} ce_context;

static ce_context* ce_context_new(Display* display)
{
	int error_base, event_base;
	const int major_version_req = 1, minor_version_req = 2;
	int major_version, minor_version;

	if (!glXQueryExtension(display, &error_base, &event_base) ||
			!glXQueryVersion(display, &major_version, &minor_version)) {
		ce_logging_error("context: no GLX support available");
		return NULL;
	}

	ce_logging_write("context: using GLX %d.%d", major_version, minor_version);

	if (major_version_req != major_version || minor_version < minor_version_req) {
		ce_logging_error("context: GLX %d.>=%d required",
			major_version_req, minor_version_req);
		return NULL;
	}

	XVisualInfo* visualinfo =
		glXChooseVisual(display, XDefaultScreen(display),
			(int[]) { GLX_RGBA, GLX_DOUBLEBUFFER,
						GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
						GLX_BLUE_SIZE, 1, GLX_ALPHA_SIZE, 1,
						GLX_STENCIL_SIZE, 1, GLX_DEPTH_SIZE, 1, None });
	if (NULL == visualinfo) {
		ce_logging_error("context: no appropriate visual found");
	}

	ce_logging_write("context: visual %u chosen", visualinfo->visualid);

	ce_context* context = ce_alloc(sizeof(ce_context));
	context->error_base = error_base;
	context->event_base = event_base;
	context->major_version = major_version;
	context->minor_version = minor_version;
	context->visualinfo = visualinfo;
	context->context = glXCreateContext(display, visualinfo, NULL, True);

	if (glXIsDirect(display, context->context)) {
		ce_logging_write("context: you have direct rendering");
	} else {
		ce_logging_warning("context: no direct rendering possible");
	}

	return context;
}

static void ce_context_del(ce_context* context)
{
	if (NULL != context) {
		assert(glXGetCurrentContext() == context->context);
		if (NULL != context->context) {
			Display* display = glXGetCurrentDisplay();
			glXMakeCurrent(display, None, NULL);
			glXDestroyContext(display, context->context);
		}
		if (NULL != context->visualinfo) {
			XFree(context->visualinfo);
		}
		ce_free(context, sizeof(ce_context));
	}
}

static void ce_context_make_current(ce_context* context,
									Display* display, GLXDrawable drawable)
{
	glXMakeCurrent(display, drawable, context->context);
}

typedef enum {
	CE_RENDERWINDOW_ATOM_PROTOCOLS,
	CE_RENDERWINDOW_ATOM_DELETE_WINDOW,
	CE_RENDERWINDOW_ATOM_STATE_FULLSCREEN,
	CE_RENDERWINDOW_ATOM_STATE,
	CE_RENDERWINDOW_ATOM_COUNT
} ce_renderwindow_atom;

typedef enum {
	CE_RENDERWINDOW_STATE_WINDOW,
	CE_RENDERWINDOW_STATE_FULLSCREEN,
	CE_RENDERWINDOW_STATE_COUNT
} ce_renderwindow_state;

struct ce_renderwindow {
	int width, height;
	bool fullscreen;
	Atom atoms[CE_RENDERWINDOW_ATOM_COUNT];
	unsigned long mask[CE_RENDERWINDOW_STATE_COUNT];
	XSetWindowAttributes attrs[CE_RENDERWINDOW_STATE_COUNT];
	Display* display;
	Window window;
	ce_displaymng* displaymng;
	ce_context* context;
};

ce_renderwindow* ce_renderwindow_new(void)
{
	XInitThreads();

	ce_renderwindow* renderwindow = ce_alloc_zero(sizeof(ce_renderwindow));
	renderwindow->width = 1024;
	renderwindow->height = 768;
	renderwindow->fullscreen = false;
	renderwindow->display = XOpenDisplay(NULL);
	renderwindow->atoms[CE_RENDERWINDOW_ATOM_PROTOCOLS] = XInternAtom(renderwindow->display, "WM_PROTOCOLS", False);
	renderwindow->atoms[CE_RENDERWINDOW_ATOM_DELETE_WINDOW] = XInternAtom(renderwindow->display, "WM_DELETE_WINDOW", False);
	renderwindow->atoms[CE_RENDERWINDOW_ATOM_STATE_FULLSCREEN] = XInternAtom(renderwindow->display, "_NET_WM_STATE_FULLSCREEN", False);
	renderwindow->atoms[CE_RENDERWINDOW_ATOM_STATE] = XInternAtom(renderwindow->display, "_NET_WM_STATE", False);
	int bpp_stub = 32;
	renderwindow->displaymng = ce_displaymng_create(renderwindow->display, bpp_stub);
	renderwindow->context = ce_context_new(renderwindow->display);

	ce_logging_write("renderwindow: resolution %dx%d",
		renderwindow->width, renderwindow->height);

	for (int i = 0; i < CE_RENDERWINDOW_STATE_COUNT; ++i) {
		renderwindow->mask[i] = CWColormap | CWEventMask;
		renderwindow->attrs[i].colormap = XCreateColormap(renderwindow->display,
			XDefaultRootWindow(renderwindow->display),
			renderwindow->context->visualinfo->visual, AllocNone);
		renderwindow->attrs[i].event_mask = ExposureMask | EnterWindowMask |
			KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
			PointerMotionMask | ButtonMotionMask | FocusChangeMask |
			VisibilityChangeMask | StructureNotifyMask | LeaveWindowMask;
		if (CE_RENDERWINDOW_STATE_FULLSCREEN == i) {
			renderwindow->mask[i] |= CWOverrideRedirect;
			renderwindow->attrs[i].override_redirect = True;
		}
	}

	renderwindow->window = XCreateWindow(renderwindow->display,
		XDefaultRootWindow(renderwindow->display),
		100, 100, renderwindow->width, renderwindow->height,
		0, renderwindow->context->visualinfo->depth, InputOutput,
		renderwindow->context->visualinfo->visual,
		renderwindow->mask[CE_RENDERWINDOW_STATE_WINDOW],
		&renderwindow->attrs[CE_RENDERWINDOW_STATE_WINDOW]);

	// set window title
	const char* title = "stub";
	XSetStandardProperties(renderwindow->display, renderwindow->window,
		title, title, None, NULL, 0, NULL);

	// handle wm_delete_events
	XSetWMProtocols(renderwindow->display, renderwindow->window,
		&renderwindow->atoms[CE_RENDERWINDOW_ATOM_DELETE_WINDOW], 1);

	XMapRaised(renderwindow->display, renderwindow->window);

	ce_context_make_current(renderwindow->context, renderwindow->display,
													renderwindow->window);
	ce_gl_init();

	return renderwindow;
}

void ce_renderwindow_del(ce_renderwindow* renderwindow)
{
	if (NULL != renderwindow) {
		ce_gl_term();
		ce_context_del(renderwindow->context);
		ce_displaymng_del(renderwindow->displaymng);
		if (0 != renderwindow->window) {
			XDestroyWindow(renderwindow->display, renderwindow->window);
		}
		if (NULL != renderwindow->display) {
			XCloseDisplay(renderwindow->display);
		}
		ce_free(renderwindow, sizeof(ce_renderwindow));
	}
}

static void ce_renderwindow_switch(ce_renderwindow* renderwindow,
									int width, int height, bool fullscreen)
{
	if (renderwindow->fullscreen != fullscreen) {
		XChangeWindowAttributes(renderwindow->display, renderwindow->window,
			renderwindow->mask[fullscreen], &renderwindow->attrs[fullscreen]);
	}

	if (!renderwindow->fullscreen && fullscreen) {
		XGrabPointer(renderwindow->display, renderwindow->window, True, NoEventMask,
			GrabModeAsync, GrabModeAsync, renderwindow->window, None, CurrentTime);
		XGrabKeyboard(renderwindow->display, renderwindow->window,
			True, GrabModeAsync, GrabModeAsync, CurrentTime);
	}

	if (renderwindow->fullscreen && !fullscreen) {
		XUngrabPointer(renderwindow->display, CurrentTime);
		XUngrabKeyboard(renderwindow->display, CurrentTime);
	}

	if (fullscreen) {
		ce_displaymng_change(renderwindow->displaymng, width, height,
			0, 0, CE_DISPLAY_ROTATION_NONE, CE_DISPLAY_REFLECTION_NONE);
	} else {
		ce_displaymng_restore(renderwindow->displaymng);
	}

	XMoveResizeWindow(renderwindow->display, renderwindow->window, 0, 0, width, height);

	XEvent message;
	memset(&message, '\0', sizeof(message));

	message.type = ClientMessage;
	message.xclient.send_event = True;
	message.xclient.window = renderwindow->window;
	message.xclient.message_type = renderwindow->atoms[CE_RENDERWINDOW_ATOM_STATE];
	message.xclient.format = 32;
	message.xclient.data.l[0] = fullscreen;
	message.xclient.data.l[1] = renderwindow->atoms[CE_RENDERWINDOW_ATOM_STATE_FULLSCREEN];

	XSendEvent(renderwindow->display, XDefaultRootWindow(renderwindow->display),
		False, SubstructureRedirectMask | SubstructureNotifyMask, &message);

	renderwindow->fullscreen = fullscreen;
	renderwindow->width = width;
	renderwindow->height = height;
}

void ce_renderwindow_minimize(ce_renderwindow* renderwindow)
{
	// TODO: send fullscreen toggle event
	XIconifyWindow(renderwindow->display,
		renderwindow->window, XDefaultScreen(renderwindow->display));
}

bool ce_renderwindow_pump(ce_renderwindow* renderwindow)
{
	KeySym key;

	while (XPending(renderwindow->display) > 0) {
		XEvent event;
		XNextEvent(renderwindow->display, &event);

		switch (event.type) {
		case ClientMessage:
			if (renderwindow->atoms[CE_RENDERWINDOW_ATOM_PROTOCOLS] ==
					event.xclient.message_type &&
					renderwindow->atoms[CE_RENDERWINDOW_ATOM_DELETE_WINDOW] ==
					(Atom)event.xclient.data.l[0]) {
				ce_logging_write("renderwindow: exiting sanely...");
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

void ce_renderwindow_swap(ce_renderwindow* renderwindow)
{
	ce_unused(renderwindow);
	glXSwapBuffers(glXGetCurrentDisplay(), glXGetCurrentDrawable());
}
