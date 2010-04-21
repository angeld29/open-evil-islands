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
#include <stdbool.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/xf86vmode.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "cealloc.h"
#include "cerenderwindow.h"

struct ce_renderwindow {
	Display* display;
	int screen;
	Window root_window, window;
	Atom WM_DELETE_WINDOW;
	XF86VidModeModeInfo** modes;
	GLXContext context;
};

static int ce_renderwindow_mode_comp(const void* lhs, const void* rhs) {
	XF86VidModeModeInfo* a = *(XF86VidModeModeInfo**)lhs;
	XF86VidModeModeInfo* b = *(XF86VidModeModeInfo**)rhs;
	return a->hdisplay > b->hdisplay ? -1 : b->vdisplay - a->vdisplay;
}

ce_renderwindow* ce_renderwindow_new(void)
{
	const char* title = "stub";
	int width = 1024;
	int height = 768;
	bool full_screen = false;

	ce_renderwindow* renderwindow = ce_alloc_zero(sizeof(ce_renderwindow));
	renderwindow->display = XOpenDisplay(NULL);
	renderwindow->screen = DefaultScreen(renderwindow->display);
	renderwindow->root_window = RootWindow(renderwindow->display,
											renderwindow->screen);

	XVisualInfo* visual_info =
		glXChooseVisual(renderwindow->display, renderwindow->screen,
			(int[]) { GLX_RGBA,
						GLX_RED_SIZE, 8,
						GLX_GREEN_SIZE, 8,
						GLX_BLUE_SIZE, 8,
						GLX_ALPHA_SIZE, 8,
						GLX_STENCIL_SIZE, 8,
						GLX_DEPTH_SIZE, 24,
						GLX_DOUBLEBUFFER,
						None });
	if (NULL == visual_info) {
	}

	if (full_screen) {
		int mode_count;
		XF86VidModeModeLine mode;
		if (XF86VidModeGetModeLine(renderwindow->display,
									renderwindow->screen,
									&mode_count, &mode) &&
				XF86VidModeGetAllModeLines(renderwindow->display,
											renderwindow->screen,
											&mode_count, &renderwindow->modes)) {
			qsort(renderwindow->modes, mode_count,
				sizeof(XF86VidModeModeInfo*), ce_renderwindow_mode_comp);

			int mode_index;
			for (mode_index = mode_count - 1; mode_index > 0; --mode_index) {
				if (renderwindow->modes[mode_index]->hdisplay >= width &&
						renderwindow->modes[mode_index]->vdisplay >= height) {
					break;
				}
			}

			if (renderwindow->modes[mode_index]->hdisplay != mode.hdisplay ||
					renderwindow->modes[mode_index]->vdisplay != mode.vdisplay) {
				XF86VidModeSwitchToMode(renderwindow->display,
					renderwindow->screen, renderwindow->modes[mode_index]);
			}

			XF86VidModeSetViewPort(renderwindow->display,
									renderwindow->screen, 0, 0);
		} else {
			full_screen = false;
		}
	}

	XSetWindowAttributes attrs;
	unsigned long mask;

	attrs.background_pixel = 0;
	attrs.border_pixel = 0;
	attrs.colormap = XCreateColormap(renderwindow->display,
		renderwindow->root_window, visual_info->visual, AllocNone);
	attrs.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
					PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

	if (full_screen) {
		attrs.override_redirect = True;
		attrs.backing_store = NotUseful;
		attrs.save_under = False;
		mask = CWBackPixel | CWColormap | CWOverrideRedirect |
				CWSaveUnder | CWBackingStore | CWEventMask;
	} else {
		mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
	}

	renderwindow->window = XCreateWindow(renderwindow->display,
		renderwindow->root_window, 0, 0, width, height, 0,
		visual_info->depth, InputOutput, visual_info->visual, mask, &attrs);

	XMapWindow(renderwindow->display, renderwindow->window);

	if (full_screen) {
		XMoveWindow(renderwindow->display, renderwindow->window, 0, 0);
		XRaiseWindow(renderwindow->display, renderwindow->window);
		XWarpPointer(renderwindow->display, None,
			renderwindow->window, 0, 0, 0, 0, width / 2, height / 2);
		XFlush(renderwindow->display);
		XF86VidModeSetViewPort(renderwindow->display, renderwindow->screen, 0, 0);
		XGrabPointer(renderwindow->display, renderwindow->window, True, 0,
			GrabModeAsync, GrabModeAsync, renderwindow->window, None, CurrentTime);
		XGrabKeyboard(renderwindow->display, renderwindow->window,
			True, GrabModeAsync, GrabModeAsync, CurrentTime);
	} else {
		renderwindow->WM_DELETE_WINDOW =
			XInternAtom(renderwindow->display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(renderwindow->display,
			renderwindow->window, &renderwindow->WM_DELETE_WINDOW,1);
		XStoreName(renderwindow->display, renderwindow->window, title);
		XSetIconName(renderwindow->display, renderwindow->window, title);
	}

	XFlush(renderwindow->display);

	renderwindow->context =
		glXCreateContext(renderwindow->display, visual_info, NULL, True);
	if (NULL == renderwindow->context) {
	}

	glXMakeCurrent(renderwindow->display,
		renderwindow->window, renderwindow->context);

	return renderwindow;
}

void ce_renderwindow_del(ce_renderwindow* renderwindow)
{
	if (NULL != renderwindow) {
		if (NULL != renderwindow->context) {
			glXDestroyContext(renderwindow->display, renderwindow->context);
		}
		if (0 != renderwindow->window) {
			XDestroyWindow(renderwindow->display, renderwindow->window);
		}
		if (NULL != renderwindow->modes) {
			XF86VidModeSwitchToMode(renderwindow->display,
				renderwindow->screen, renderwindow->modes[0]);
		}
		if (NULL != renderwindow->display) {
			XCloseDisplay(renderwindow->display);
		}
		ce_free(renderwindow, sizeof(ce_renderwindow));
	}
}

void ce_renderwindow_swap(ce_renderwindow* renderwindow)
{
	glXSwapBuffers(renderwindow->display, renderwindow->window);
}
