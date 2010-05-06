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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// mouse wheel rotation stuff, only define if we are
// on a version of the OS that does not support
// WM_MOUSEWHEEL messages.
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST + 1)
#endif

#include "cegl.h"
#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cerenderwindow.h"

typedef struct {
	ce_vector* modes;
	DEVMODE original_mode;
} ce_renderwindow_dm;

static void ce_renderwindow_dm_ctor(ce_renderwindow_modemng* modemng, va_list args)
{
	ce_unused(args);

	ce_renderwindow_dm* dm = (ce_renderwindow_dm*)modemng->impl;
	dm->modes = ce_vector_new();

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm->original_mode);

	// assume that EnumDisplaySettings returns sorted data
	// if not, fix it!

	DEVMODE mode;
	DWORD mode_index = 0;
	DWORD prev_width = 0, prev_height = 0;
	DWORD bpp = GetDeviceCaps(GetDC(NULL), BITSPIXEL);

	while (EnumDisplaySettings(NULL, mode_index++, &mode)) {
		if (mode.dmBitsPerPel != bpp || (mode.dmPelsWidth == prev_width &&
				mode.dmPelsHeight == prev_height)) {
			continue;
		}

		ce_vector_push_back(dm->modes, ce_alloc(sizeof(DEVMODE)));
		*(DEVMODE*)ce_vector_back(dm->modes) = mode;

		ce_vector_push_back(modemng->modes,
			ce_renderwindow_mode_new(mode.dmPelsWidth, mode.dmPelsHeight,
				mode.dmBitsPerPel, mode.dmDisplayFrequency));

		prev_width = mode.dmPelsWidth;
		prev_height = mode.dmPelsHeight;
	}
}

static void ce_renderwindow_dm_dtor(ce_renderwindow_modemng* modemng)
{
	ce_renderwindow_dm* dm = (ce_renderwindow_dm*)modemng->impl;

	for (int i = 0; i < dm->modes->count; ++i) {
		ce_free(dm->modes->items[i], sizeof(DEVMODE));
	}
	ce_vector_del(dm->modes);

	ChangeDisplaySettingsEx(NULL, &dm->original_mode, NULL, 0, NULL);
}

static void ce_renderwindow_dm_change(ce_renderwindow_modemng* modemng, int index)
{
	ce_renderwindow_dm* dm = (ce_renderwindow_dm*)modemng->impl;
	DEVMODE* mode = dm->modes->items[index];

	LONG code = ChangeDisplaySettingsEx(NULL, mode, NULL, CDS_FULLSCREEN, NULL);
	if (DISP_CHANGE_SUCCESSFUL != code) {
		ce_logging_error("renderwindow: could not change display settings");
	}
}

static ce_renderwindow_modemng* ce_renderwindow_modemng_create(void)
{
	ce_renderwindow_modemng_vtable vtable = {
		ce_renderwindow_dm_ctor, ce_renderwindow_dm_dtor, ce_renderwindow_dm_change
	};
	return ce_renderwindow_modemng_new(vtable, sizeof(ce_renderwindow_dm));
}

struct ce_renderwindow {
	int width, height;
	bool full_screen;
	ce_renderwindow_modemng* modemng;
	HWND window;
	HGLRC context;
};

static LRESULT CALLBACK ce_renderwindow_proc(HWND, UINT, WPARAM, LPARAM);

ce_renderwindow* ce_renderwindow_new(void)
{
	ce_renderwindow* renderwindow = ce_alloc_zero(sizeof(ce_renderwindow));
	renderwindow->width = 1024;
	renderwindow->height = 768;
	renderwindow->full_screen = false;
	renderwindow->modemng = ce_renderwindow_modemng_create();

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // redraws the window for
	                                               // any movement/resizing
	wc.lpfnWndProc = ce_renderwindow_proc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE; // class background brush color
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "cursedearth";

	if (!RegisterClassEx(&wc)) {
		ce_logging_error("renderwindow: RegisterClassEx failed");
	}

	DWORD style = WS_VISIBLE;
	DWORD extended_style = 0;

	if (renderwindow->full_screen) {
		ce_renderwindow_mode* mode = renderwindow->modemng->modes->items[0];
		renderwindow->width = mode->width;
		renderwindow->height = mode->height;

		ce_renderwindow_modemng_change(renderwindow->modemng, 0);

		style |= WS_POPUP;
		extended_style |= WS_EX_TOOLWINDOW;
	} else {
		style |= WS_OVERLAPPEDWINDOW;
		extended_style |= WS_EX_APPWINDOW;

		RECT rect = { 0, 0, renderwindow->width, renderwindow->height };
		AdjustWindowRectEx(&rect, style, 0, extended_style);

		renderwindow->width = rect.right - rect.left;
		renderwindow->height = rect.bottom - rect.top;
	}

	ce_logging_write("renderwindow: resolution %dx%d",
		renderwindow->width, renderwindow->height);

	renderwindow->window = CreateWindowEx(extended_style,
		wc.lpszClassName, "stub", style, CW_USEDEFAULT, CW_USEDEFAULT,
		renderwindow->width, renderwindow->height,
		HWND_DESKTOP, NULL, wc.hInstance, NULL);
	if (NULL == renderwindow->window) {
		ce_logging_error("renderwindow: CreateWindowEx failed");
	}

	SetWindowLongPtr(renderwindow->window, GWLP_USERDATA, (LONG_PTR)renderwindow);

	ShowWindow(renderwindow->window, SW_SHOW);
	UpdateWindow(renderwindow->window);

	SetForegroundWindow(renderwindow->window);
	SetFocus(renderwindow->window);

	HDC dc = GetDC(renderwindow->window);
	if (NULL == dc) {
		ce_logging_error("renderwindow: GetDC failed");
	}

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,                             // version number
		PFD_DRAW_TO_WINDOW |           // format must support window
		PFD_SUPPORT_OPENGL |           // format must support opengl
		PFD_DOUBLEBUFFER,              // must support double buffering
		PFD_TYPE_RGBA,                 // request an RGBA format
		GetDeviceCaps(dc, BITSPIXEL),  // select our color depth
		8, 0, 8, 0, 8, 0,              // color shifts ignored
		8,                             // alpha buffer
		0,                             // shift bit ignored
		0,                             // no accumulation buffer
		0, 0, 0, 0,                    // accumulation bits ignored
		24,                            // depth buffer
		8,                             // stencil buffer
		0,                             // no auxiliary buffer
		PFD_MAIN_PLANE,                // main drawing layer
		0,                             // reserved
		0, 0, 0                        // layer masks ignored
	};

	// find a compatible pixel format
	int pixel_format = ChoosePixelFormat(dc, &pfd);
	if (0 == pixel_format) {
		ce_logging_error("renderwindow: could not choose pixel format");
	}

	// try to set the pixel format
	if (!(SetPixelFormat(dc, pixel_format, &pfd))) {
		ce_logging_error("renderwindow: could not set pixel format");
	}

	// try to get a rendering context
	renderwindow->context = wglCreateContext(dc);
	if (NULL == renderwindow->context) {
		ce_logging_error("renderwindow: could not create context");
	}

	// make the rendering context our current rendering context
	if (!wglMakeCurrent(dc, renderwindow->context)) {
		ce_logging_error("renderwindow: could not set context");
	}

	ce_gl_init();

	return renderwindow;
}

void ce_renderwindow_del(ce_renderwindow* renderwindow)
{
	if (NULL != renderwindow) {
		ce_gl_term();
		if (NULL != renderwindow->window) {
			if (NULL != renderwindow->context) {
				wglDeleteContext(wglGetCurrentContext());
				wglMakeCurrent(wglGetCurrentDC(), NULL);
			}
			ReleaseDC(renderwindow->window, GetDC(renderwindow->window));
			DestroyWindow(renderwindow->window);
		}
		//UnregisterClass(class, instance); // ???
		ce_renderwindow_modemng_del(renderwindow->modemng);
		ce_free(renderwindow, sizeof(ce_renderwindow));
	}
}

bool ce_renderwindow_pump(ce_renderwindow* renderwindow)
{
	ce_unused(renderwindow);
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (WM_QUIT == msg.message) {
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

void ce_renderwindow_swap(ce_renderwindow* renderwindow)
{
	ce_unused(renderwindow);
	SwapBuffers(wglGetCurrentDC());
}

static void ce_renderwindow_process(ce_renderwindow* renderwindow,
									UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {
	case WM_SIZE:
		//width = LOWORD(lparam);
		//height = HIWORD(lparam);
		break;
	case WM_KEYDOWN:
		if (VK_ESCAPE == wparam) {
			PostQuitMessage(0);
		}
		break;
	case WM_KEYUP:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_MOUSEMOVE:
		break;
	};
}

static LRESULT CALLBACK ce_renderwindow_proc(HWND window, UINT message,
											WPARAM wparam, LPARAM lparam)
{
	switch (message) {
	case WM_DESTROY:
		ce_logging_write("renderwindow: exiting sanely...");
		if (NULL == GetParent(window)) {
			PostQuitMessage(0);
		}
		break;
	};

	ce_renderwindow* renderwindow =
		(ce_renderwindow*)GetWindowLongPtr(window, GWLP_USERDATA);
	if (NULL != renderwindow) {
		ce_renderwindow_process(renderwindow, message, wparam, lparam);
	}

	return DefWindowProc(window, message, wparam, lparam);
}
