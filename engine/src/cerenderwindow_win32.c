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

#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "celib.h"
#include "cealloc.h"
#include "cerenderwindow.h"

struct ce_renderwindow {
	HWND window;
	HGLRC context;
	bool full_screen;
};

static LRESULT CALLBACK window_proc(HWND window, UINT message,
									WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(window, message, wparam, lparam);
}

ce_renderwindow* ce_renderwindow_new(void)
{
	const char* title = "stub";
	int width = 1024;
	int height = 768;

	ce_renderwindow* renderwindow = ce_alloc(sizeof(ce_renderwindow));
	renderwindow->full_screen = false;

	HDC dc;
	int pixel_format;

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 8, 0,
		PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	WNDCLASS wc;
	wc.style = 0;
	wc.lpfnWndProc = window_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = 0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "cursedearth";

	if (!RegisterClass(&wc)) {
	}

	if (renderwindow->full_screen) {
		DEVMODE settings;
		memset(&settings, '\0', sizeof(DEVMODE));

		settings.dmSize = sizeof(DEVMODE);
		settings.dmPelsWidth = width;
		settings.dmPelsHeight = height;
		settings.dmBitsPerPel = 32;
		settings.dmDisplayFrequency = 85;
		settings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT |
							DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

		if (DISP_CHANGE_SUCCESSFUL !=
				ChangeDisplaySettings(&settings, CDS_FULLSCREEN)) {
		}

		renderwindow->window = CreateWindowEx(0, wc.lpszClassName, title,
			WS_POPUP, 0, 0, width, height, NULL, NULL, wc.hInstance, NULL);
	} else {
		RECT windowRect = { 0, 0, width, height };
		AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, 0, 0);

		width = windowRect.right - windowRect.left;
		height = windowRect.bottom - windowRect.top;

		renderwindow->window = CreateWindowEx(0, wc.lpszClassName,
			title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			width, height, NULL, NULL, wc.hInstance, NULL);
	}

	if (!renderwindow->window) {
	}

	if (!(dc = GetDC(renderwindow->window))) {
	}

	if (!(pixel_format = ChoosePixelFormat(dc, &pfd))) {
	}

	if (!(SetPixelFormat(dc, pixel_format, &pfd))) {
	}

	if (!(renderwindow->context = wglCreateContext(dc))) {
	}

	if (!wglMakeCurrent(dc, renderwindow->context)) {
	}

	ShowWindow(renderwindow->window, SW_SHOW);
	UpdateWindow(renderwindow->window);
	SetForegroundWindow(renderwindow->window);
	SetFocus(renderwindow->window);

	return renderwindow;
}

void ce_renderwindow_del(ce_renderwindow* renderwindow)
{
	if (NULL != renderwindow) {
		if (NULL != renderwindow->window) {
			HDC dc = GetDC(renderwindow->window);
			wglMakeCurrent(dc, NULL);
			if (NULL != renderwindow->context) {
				wglDeleteContext(renderwindow->context);
			}
			ReleaseDC(renderwindow->window, dc);
			DestroyWindow(renderwindow->window);
			if (renderwindow->full_screen) {
				ChangeDisplaySettings(NULL, 0);
				ShowCursor(TRUE);
			}
		}
		ce_free(renderwindow, sizeof(ce_renderwindow));
	}
}

void ce_renderwindow_swap(ce_renderwindow* renderwindow)
{
	ce_unused(renderwindow);
	SwapBuffers(wglGetCurrentDC());
}
