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
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <sys/utsname.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/xf86vmstr.h>
#include <X11/extensions/Xrandr.h>

#include <GL/glx.h>

#include "cegl.h"
#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cerenderwindow.h"

// XFree86, see xf86vidmode(3) for more details
typedef struct {
	int event_base, error_base;
	int version_major, version_minor;
	Display* display;
	XF86VidModeModeInfo** modes;
} ce_renderwindow_xf86vm;

static int ce_renderwindow_xf86vm_calc_rate(XF86VidModeModeInfo* info)
{
	// dotclock comes in kHz
	return (info->dotclock * 1000) / (info->htotal * info->vtotal);
}

static void ce_renderwindow_xf86vm_ctor(ce_renderwindow_modemng* modemng, va_list args)
{
	ce_renderwindow_xf86vm* xf86vm = (ce_renderwindow_xf86vm*)modemng->impl;
	xf86vm->display = va_arg(args, Display*);

	XF86VidModeQueryExtension(xf86vm->display, &xf86vm->event_base,
												&xf86vm->error_base);
	XF86VidModeQueryVersion(xf86vm->display, &xf86vm->version_major,
											&xf86vm->version_minor);

	ce_logging_write("renderwindow: using XFree86 Video Mode Extension %d.%d",
		xf86vm->version_major, xf86vm->version_minor);

	int mode_count;
	XF86VidModeGetAllModeLines(xf86vm->display, DefaultScreen(xf86vm->display),
		&mode_count, &xf86vm->modes);

	for (int i = 0; i < mode_count; ++i) {
		XF86VidModeModeInfo* info = xf86vm->modes[i];
		ce_vector_push_back(modemng->modes,
			ce_renderwindow_mode_new(info->hdisplay, info->vdisplay,
				0, ce_renderwindow_xf86vm_calc_rate(info)));
	}
}

static void ce_renderwindow_xf86vm_change(ce_renderwindow_modemng*, int);

static void ce_renderwindow_xf86vm_dtor(ce_renderwindow_modemng* modemng)
{
	ce_renderwindow_xf86vm* xf86vm = (ce_renderwindow_xf86vm*)modemng->impl;

	if (NULL != xf86vm->modes) {
		ce_renderwindow_xf86vm_change(modemng, 0);
		XFree(xf86vm->modes);
	}
}

static void ce_renderwindow_xf86vm_change(ce_renderwindow_modemng* modemng, int index)
{
	ce_renderwindow_xf86vm* xf86vm = (ce_renderwindow_xf86vm*)modemng->impl;

	if (NULL != xf86vm->modes) {
		XF86VidModeSwitchToMode(xf86vm->display,
			DefaultScreen(xf86vm->display), xf86vm->modes[index]);
		XF86VidModeSetViewPort(xf86vm->display,
			DefaultScreen(xf86vm->display), 0, 0);
	}
}

static bool ce_renderwindow_xf86vm_query(Display* display)
{
	int dummy, version_major, version_minor;
	return XF86VidModeQueryExtension(display, &dummy, &dummy) &&
		XF86VidModeQueryVersion(display, &version_major, &version_minor) &&
		2 == version_major;
}

// XRandR, see xrandr(3) for more details
typedef struct {
	int event_base, error_base;
	int version_major, version_minor;
	Display* display;
	XRRScreenConfiguration* conf;
	SizeID original_size_id;
	short original_rate;
	Rotation original_rotation;
} ce_renderwindow_xrr;

static void ce_renderwindow_xrr_ctor(ce_renderwindow_modemng* modemng, va_list args)
{
	ce_renderwindow_xrr* xrr = (ce_renderwindow_xrr*)modemng->impl;
	xrr->display = va_arg(args, Display*);

	XRRQueryExtension(xrr->display, &xrr->event_base, &xrr->error_base);
	XRRQueryVersion(xrr->display, &xrr->version_major, &xrr->version_minor);

	ce_logging_write("renderwindow: using XRandR Extension %d.%d",
		xrr->version_major, xrr->version_minor);

	// get the possible set of rotations/reflections supported
	//Rotation rotation = XRRRotations(xrr->display,
	//	DefaultScreen(xrr->display), &xrr->original_rotation);

	xrr->conf = XRRGetScreenInfo(xrr->display, XDefaultRootWindow(xrr->display));
	xrr->original_size_id =
		XRRConfigCurrentConfiguration(xrr->conf, &xrr->original_rotation);
	xrr->original_rate = XRRConfigCurrentRate(xrr->conf);

	// get possible screen resolutions
	int size_count;
	XRRScreenSize* sizes = XRRSizes(xrr->display,
		DefaultScreen(xrr->display), &size_count);

	for (int i = 0; i < size_count; ++i) {
		int rate_count;
        short* rates = XRRRates(xrr->display,
			DefaultScreen(xrr->display), i, &rate_count);

		ce_vector_push_back(modemng->modes,
			ce_renderwindow_mode_new(sizes[i].width, sizes[i].height,
									0, rates[0]));
	}
}

static void ce_renderwindow_xrr_dtor(ce_renderwindow_modemng* modemng)
{
	ce_renderwindow_xrr* xrr = (ce_renderwindow_xrr*)modemng->impl;

	XRRSetScreenConfigAndRate(xrr->display, xrr->conf,
		XDefaultRootWindow(xrr->display), xrr->original_size_id,
		xrr->original_rotation, xrr->original_rate, CurrentTime);

	XRRFreeScreenConfigInfo(xrr->conf);
}

static void ce_renderwindow_xrr_change(ce_renderwindow_modemng* modemng, int index)
{
	ce_renderwindow_xrr* xrr = (ce_renderwindow_xrr*)modemng->impl;
	ce_renderwindow_mode* mode = modemng->modes->items[index];

	XRRSetScreenConfigAndRate(xrr->display, xrr->conf,
		XDefaultRootWindow(xrr->display), index,
		RR_Rotate_0, mode->rate, CurrentTime);
}

static bool ce_renderwindow_xrr_query(Display* display)
{
	int dummy, version_major, version_minor;
	return XRRQueryExtension(display, &dummy, &dummy) &&
		XRRQueryVersion(display, &version_major, &version_minor) &&
		1 == version_major && version_minor >= 1;
}

static ce_renderwindow_modemng* ce_renderwindow_modemng_create(Display* display)
{
	struct {
		const char* name;
		ce_renderwindow_modemng_vtable vtable;
		size_t size;
	} extensions[] = { // prefer XRandR
		{ RANDR_NAME, { ce_renderwindow_xrr_ctor,
			ce_renderwindow_xrr_dtor, ce_renderwindow_xrr_change },
			sizeof(ce_renderwindow_xrr) },
		{ XF86VIDMODENAME, { ce_renderwindow_xf86vm_ctor,
			ce_renderwindow_xf86vm_dtor, ce_renderwindow_xf86vm_change },
			sizeof(ce_renderwindow_xf86vm) },
	};

	for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); ++i) {
		int dummy;
		if (XQueryExtension(display, extensions[i].name, &dummy, &dummy, &dummy)) {
			return ce_renderwindow_modemng_new(extensions[i].vtable,
												extensions[i].size, display);
		}
	}

	return NULL;
}

typedef struct {
	int error_base, event_base;
	int version_major, version_minor;
	Display* display;
	XVisualInfo* visualinfo;
	GLXContext context;
} ce_context;

static ce_context* ce_context_new(Display* display)
{
	int error_base, event_base;
	int version_major, version_minor;

	if (!glXQueryExtension(display, &error_base, &event_base) ||
			!glXQueryVersion(display, &version_major, &version_minor)) {
		ce_logging_error("context: no GLX support available");
		return NULL;
	}

	ce_logging_write("context: using GLX %d.%d", version_major, version_minor);

	XVisualInfo* visualinfo =
		glXChooseVisual(display, DefaultScreen(display),
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
	context->version_major = version_major;
	context->version_minor = version_minor;
	context->display = display;
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
		assert(context->context == glXGetCurrentContext());
		if (NULL != context->context) {
			glXDestroyContext(context->display, context->context);
			glXMakeCurrent(context->display, None, NULL);
		}
		if (NULL != context->visualinfo) {
			XFree(context->visualinfo);
		}
		ce_free(context, sizeof(ce_context));
	}
}

static void ce_context_make_current(ce_context* context, GLXDrawable drawable)
{
	glXMakeCurrent(context->display, drawable, context->context);
}

struct ce_renderwindow {
	int width, height;
	bool fullscreen;
	struct {
		Atom WM_DELETE_WINDOW;
		Atom _NET_WM_STATE_FULLSCREEN;
		Atom _NET_WM_STATE;
	} atom;
	unsigned long mask[2];
	XSetWindowAttributes attrs[2];
	Display* display;
	Window window;
	ce_renderwindow_modemng* modemng;
	ce_context* context;
};

ce_renderwindow* ce_renderwindow_new(void)
{
	XInitThreads();

	struct utsname osinfo;
	uname(&osinfo);

	ce_logging_write("renderwindow: %s %s %s %s %s", osinfo.sysname,
		osinfo.release, osinfo.version, osinfo.machine, osinfo.nodename);

	ce_renderwindow* renderwindow = ce_alloc_zero(sizeof(ce_renderwindow));
	renderwindow->width = 1024;
	renderwindow->height = 768;
	renderwindow->fullscreen = false;

	renderwindow->display = XOpenDisplay(NULL);
	renderwindow->modemng = ce_renderwindow_modemng_create(renderwindow->display);
	renderwindow->context = ce_context_new(renderwindow->display);

	renderwindow->atom.WM_DELETE_WINDOW = XInternAtom(renderwindow->display, "WM_DELETE_WINDOW", False);
	renderwindow->atom._NET_WM_STATE_FULLSCREEN = XInternAtom(renderwindow->display, "_NET_WM_STATE_FULLSCREEN", False);
	renderwindow->atom._NET_WM_STATE = XInternAtom(renderwindow->display, "_NET_WM_STATE", False);

	ce_logging_write("renderwindow: resolution %dx%d",
		renderwindow->width, renderwindow->height);

	for (int i = 0; i < 2; ++i) {
		renderwindow->mask[i] = CWColormap | CWEventMask;
		renderwindow->attrs[i].colormap = XCreateColormap(renderwindow->display,
			XDefaultRootWindow(renderwindow->display),
			renderwindow->context->visualinfo->visual, AllocNone);
		renderwindow->attrs[i].event_mask = ExposureMask | EnterWindowMask |
			KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
			PointerMotionMask | ButtonMotionMask | FocusChangeMask |
			VisibilityChangeMask | StructureNotifyMask | LeaveWindowMask;
		if (1 == i) {
			renderwindow->mask[i] |= CWOverrideRedirect;
			renderwindow->attrs[i].override_redirect = True;
		}
	}

	renderwindow->window = XCreateWindow(renderwindow->display,
		XDefaultRootWindow(renderwindow->display),
		100, 100, renderwindow->width, renderwindow->height,
		0, renderwindow->context->visualinfo->depth, InputOutput,
		renderwindow->context->visualinfo->visual,
		renderwindow->mask[0], &renderwindow->attrs[0]);

	// set window title
	const char* title = "stub";
	XSetStandardProperties(renderwindow->display, renderwindow->window,
		title, title, None, NULL, 0, NULL);

	// handle wm_delete_events
	XSetWMProtocols(renderwindow->display,
		renderwindow->window, &renderwindow->atom.WM_DELETE_WINDOW, 1);

	XMapRaised(renderwindow->display, renderwindow->window);

	ce_context_make_current(renderwindow->context, renderwindow->window);
	ce_gl_init();

	return renderwindow;
}

void ce_renderwindow_del(ce_renderwindow* renderwindow)
{
	if (NULL != renderwindow) {
		ce_gl_term();
		ce_context_del(renderwindow->context);
		ce_renderwindow_modemng_del(renderwindow->modemng);
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
		XGrabPointer(renderwindow->display, renderwindow->window, True, 0,
			GrabModeAsync, GrabModeAsync, renderwindow->window, None, CurrentTime);
		XGrabKeyboard(renderwindow->display, renderwindow->window,
			True, GrabModeAsync, GrabModeAsync, CurrentTime);
	}

	if (renderwindow->fullscreen && !fullscreen) {
		XUngrabPointer(renderwindow->display, CurrentTime);
		XUngrabKeyboard(renderwindow->display, CurrentTime);
	}

	ce_renderwindow_modemng_change(renderwindow->modemng, 0);

	if (fullscreen) {
		ce_renderwindow_mode* mode = renderwindow->modemng->modes->items[0];
		width = mode->width;
		height = mode->height;

		//XWarpPointer(renderwindow->display, None, renderwindow->window,
		//	0, 0, 0, 0, width / 2, height / 2);
	}

	XMoveResizeWindow(renderwindow->display, renderwindow->window, 0, 0, width, height);

	XEvent message;
	memset(&message, '\0', sizeof(message));

	message.type = ClientMessage;
	message.xclient.send_event = True;
	message.xclient.window = renderwindow->window;
	message.xclient.message_type = renderwindow->atom._NET_WM_STATE;
	message.xclient.format = 32;
	message.xclient.data.l[0] = fullscreen;
	message.xclient.data.l[1] = renderwindow->atom._NET_WM_STATE_FULLSCREEN;

	XSendEvent(renderwindow->display, XDefaultRootWindow(renderwindow->display),
		False, SubstructureRedirectMask | SubstructureNotifyMask, &message);

	renderwindow->fullscreen = fullscreen;
	renderwindow->width = width;
	renderwindow->height = height;
}

void ce_renderwindow_minimize(ce_renderwindow* renderwindow)
{
	XIconifyWindow(renderwindow->display,
		renderwindow->window, DefaultScreen(renderwindow->display));
}

bool ce_renderwindow_pump(ce_renderwindow* renderwindow)
{
	KeySym key;

	while (XPending(renderwindow->display) > 0) {
		XEvent event;
		XNextEvent(renderwindow->display, &event);

		switch (event.type) {
		case ClientMessage:
			if (32 == event.xclient.format &&
					(Atom)event.xclient.data.l[0] ==
					renderwindow->atom.WM_DELETE_WINDOW) {
				ce_logging_write("renderwindow: exiting sanely...");
				return false;
			}
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
