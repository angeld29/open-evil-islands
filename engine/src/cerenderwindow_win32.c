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

#include "cecontext_win32.h"

typedef struct {
	HWND window;
} ce_winwindow;

static LRESULT CALLBACK ce_renderwindow_proc(HWND, UINT, WPARAM, LPARAM);

ce_renderwindow* ce_renderwindow_new(void)
{
	ce_renderwindow* renderwindow = ce_alloc_zero(sizeof(ce_renderwindow) +
													sizeof(ce_winwindow));
	ce_winwindow* winwindow = (ce_winwindow*)renderwindow->impl;

	renderwindow->width = 1024;
	renderwindow->height = 768;
	renderwindow->fullscreen = false;

	renderwindow->displaymng = ce_displaymng_create(NULL);

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
		ce_logging_error("renderwindow: could not register class");
	}

	DWORD style = WS_VISIBLE;
	DWORD extended_style = 0;

	if (renderwindow->fullscreen) {
		ce_displaymng_change(renderwindow->displaymng,
			renderwindow->width, renderwindow->height, 0, 0,
			CE_DISPLAY_ROTATION_NONE, CE_DISPLAY_REFLECTION_NONE);

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

	winwindow->window = CreateWindowEx(extended_style,
		wc.lpszClassName, "stub", style, CW_USEDEFAULT, CW_USEDEFAULT,
		renderwindow->width, renderwindow->height,
		HWND_DESKTOP, NULL, wc.hInstance, NULL);
	if (NULL == winwindow->window) {
		ce_logging_error("renderwindow: could not create window");
	}

	SetWindowLongPtr(winwindow->window, GWLP_USERDATA, (LONG_PTR)renderwindow);

	ShowWindow(winwindow->window, SW_SHOW);
	UpdateWindow(winwindow->window);

	SetForegroundWindow(winwindow->window);
	SetFocus(winwindow->window);

	renderwindow->context = ce_context_new(GetDC(winwindow->window));

	return renderwindow;
}

void ce_renderwindow_del(ce_renderwindow* renderwindow)
{
	if (NULL != renderwindow) {
		ce_context_del(renderwindow->context);
		ce_displaymng_del(renderwindow->displaymng);
		ce_winwindow* winwindow = (ce_winwindow*)renderwindow->impl;
		if (NULL != winwindow->window) {
			ReleaseDC(winwindow->window, GetDC(winwindow->window));
			DestroyWindow(winwindow->window);
		}
		UnregisterClass("cursedearth", GetModuleHandle(NULL));
		ce_free(renderwindow, sizeof(ce_renderwindow) + sizeof(ce_winwindow));
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
