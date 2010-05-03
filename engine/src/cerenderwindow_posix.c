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
#include <assert.h>

#include <X11/Xlib.h>
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
	Display* display;
	int event_base, error_base;
	int version_major, version_minor;
	XF86VidModeModeInfo** modes;
} ce_renderwindow_xf86vm;

static int ce_renderwindow_xf86vm_refresh(XF86VidModeModeInfo* info)
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
				0, ce_renderwindow_xf86vm_refresh(info)));
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

// XRandR, see xrandr(3) for more details
typedef struct {
	Display* display;
	int event_base, error_base;
	int version_major, version_minor;
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

	xrr->conf = XRRGetScreenInfo(xrr->display, RootWindow(xrr->display,
											DefaultScreen(xrr->display)));
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

	XRRSetScreenConfigAndRate(xrr->display, xrr->conf, RootWindow(xrr->display,
		DefaultScreen(xrr->display)), xrr->original_size_id,
		xrr->original_rotation, xrr->original_rate, CurrentTime);

	XRRFreeScreenConfigInfo(xrr->conf);
}

static void ce_renderwindow_xrr_change(ce_renderwindow_modemng* modemng, int index)
{
	ce_renderwindow_xrr* xrr = (ce_renderwindow_xrr*)modemng->impl;
	ce_renderwindow_mode* mode = modemng->modes->items[index];

	XRRSetScreenConfigAndRate(xrr->display, xrr->conf, RootWindow(xrr->display,
		DefaultScreen(xrr->display)), index, RR_Rotate_0, mode->rate, CurrentTime);
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

struct ce_renderwindow {
	int width, height;
	bool full_screen;
	ce_renderwindow_modemng* modemng;
	Display* display;
	Window window;
	Atom wm_delete_window;
	GLXContext context;
};

ce_renderwindow* ce_renderwindow_new(void)
{
	bool full_screen = false;

	XInitThreads();

	ce_renderwindow* renderwindow = ce_alloc_zero(sizeof(ce_renderwindow));
	renderwindow->display = XOpenDisplay(NULL);
	renderwindow->modemng = ce_renderwindow_modemng_create(renderwindow->display);

	if (NULL == renderwindow->modemng) {
		full_screen = false;
	}

	renderwindow->width = 1024;
	renderwindow->height = 768;
	renderwindow->full_screen = full_screen;

	if (renderwindow->full_screen) {
		ce_renderwindow_mode* mode = renderwindow->modemng->modes->items[0];
		renderwindow->width = mode->width;
		renderwindow->height = mode->height;
	}

	ce_logging_write("renderwindow: resolution %dx%d",
		renderwindow->width, renderwindow->height);

	// setup GLX
	int glx_version_major, glx_version_minor;
	glXQueryVersion(renderwindow->display, &glx_version_major,
											&glx_version_minor);

	ce_logging_write("renderwindow: using GLX %d.%d",
		glx_version_major, glx_version_minor);

	XVisualInfo* visual_info =
		glXChooseVisual(renderwindow->display,
						DefaultScreen(renderwindow->display),
			(int[]) { GLX_RGBA, GLX_DOUBLEBUFFER,
						GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8,
						GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8,
						GLX_STENCIL_SIZE, 8, GLX_DEPTH_SIZE, 24, None });
	if (NULL == visual_info) {
		ce_logging_error("renderwindow: could not get an RGBA, double-buffered visual");
	}

	// create X window
	XSetWindowAttributes attrs;
	unsigned long mask;

	attrs.background_pixmap = None;
	attrs.background_pixel = 0;
	attrs.border_pixel = 0;
	attrs.colormap = XCreateColormap(renderwindow->display,
		RootWindow(renderwindow->display, DefaultScreen(renderwindow->display)),
		visual_info->visual, AllocNone);
	attrs.event_mask = ExposureMask | StructureNotifyMask | PointerMotionMask |
		KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
		VisibilityChangeMask | EnterWindowMask | LeaveWindowMask;
	mask = CWBackPixel | CWColormap | CWEventMask;

	if (renderwindow->full_screen) {
		attrs.override_redirect = True;
		attrs.backing_store = NotUseful;
		attrs.save_under = False;
		mask |= CWOverrideRedirect | CWSaveUnder | CWBackingStore;
	} else {
		mask |= CWBorderPixel;
	}

	renderwindow->window = XCreateWindow(renderwindow->display,
		RootWindow(renderwindow->display, DefaultScreen(renderwindow->display)),
		0, 0, renderwindow->width, renderwindow->height, 0,
		visual_info->depth, InputOutput, visual_info->visual, mask, &attrs);

	XMapRaised(renderwindow->display, renderwindow->window);

	if (renderwindow->full_screen) {
		ce_renderwindow_modemng_change(renderwindow->modemng, 0);

		XMoveWindow(renderwindow->display, renderwindow->window, 0, 0);
		XWarpPointer(renderwindow->display, None, renderwindow->window,
			0, 0, 0, 0, renderwindow->width / 2, renderwindow->height / 2);

		XGrabPointer(renderwindow->display, renderwindow->window, True, 0,
			GrabModeAsync, GrabModeAsync, renderwindow->window, None, CurrentTime);
		XGrabKeyboard(renderwindow->display, renderwindow->window,
			True, GrabModeAsync, GrabModeAsync, CurrentTime);
	} else {
		// handle wm_delete_events
		renderwindow->wm_delete_window = XInternAtom(renderwindow->display,
											"WM_DELETE_WINDOW", False);
		XSetWMProtocols(renderwindow->display,
			renderwindow->window, &renderwindow->wm_delete_window, 1);

		// set window title
		const char* title = "stub";
		XSetStandardProperties(renderwindow->display, renderwindow->window,
			title, title, None, NULL, 0, NULL);
	}

	XFlush(renderwindow->display);

	// create GL context
	renderwindow->context = glXCreateContext(renderwindow->display,
												visual_info, NULL, True);
	if (NULL == renderwindow->context) {
		ce_logging_error("renderwindow: could not create context");
	}

	glXMakeCurrent(renderwindow->display,
		renderwindow->window, renderwindow->context);

	if (glXIsDirect(renderwindow->display, renderwindow->context)) {
		ce_logging_write("renderwindow: you have direct rendering");
	} else {
		ce_logging_warning("renderwindow: no direct rendering possible");
	}

	ce_gl_init();

	XFree(visual_info);

	return renderwindow;
}

void ce_renderwindow_del(ce_renderwindow* renderwindow)
{
	if (NULL != renderwindow) {
		ce_gl_term();
		if (NULL != renderwindow->context) {
			glXDestroyContext(glXGetCurrentDisplay(), glXGetCurrentContext());
			glXMakeCurrent(glXGetCurrentDisplay(), None, NULL);
		}
		if (0 != renderwindow->window) {
			XDestroyWindow(renderwindow->display, renderwindow->window);
		}
		ce_renderwindow_modemng_del(renderwindow->modemng);
		if (NULL != renderwindow->display) {
			XCloseDisplay(renderwindow->display);
		}
		ce_free(renderwindow, sizeof(ce_renderwindow));
	}
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
					(ulong)event.xclient.data.l[0] ==
					renderwindow->wm_delete_window) {
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
