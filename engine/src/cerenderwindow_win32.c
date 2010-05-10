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

#include <windows.h>

// mouse wheel rotation stuff, only define if we are
// on a version of the OS that does not support
// WM_MOUSEWHEEL messages.
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST + 1)
#endif

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cerenderwindow.h"

#include "cedisplay_win32.h"
#include "cecontext_win32.h"

typedef struct {
	ce_renderwindow* renderwindow;
	HWND window;
} ce_renderwindow_win;

static LRESULT CALLBACK ce_renderwindow_proc(HWND, UINT, WPARAM, LPARAM);

ce_renderwindow* ce_renderwindow_new(const char* title, int width, int height)
{
	ce_renderwindow* renderwindow = ce_alloc_zero(sizeof(ce_renderwindow) +
													sizeof(ce_renderwindow_win));

	ce_renderwindow_win* win_renderwindow = (ce_renderwindow_win*)renderwindow->impl;
	win_renderwindow->renderwindow = renderwindow;

	renderwindow->width = width;
	renderwindow->height = height;

	renderwindow->displaymng = ce_displaymng_create();
	renderwindow->input_context = ce_input_context_new();

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
		ce_logging_fatal("renderwindow: could not register class");
		ce_renderwindow_del(renderwindow);
		return NULL;
	}

	DWORD style = WS_VISIBLE;
	DWORD extended_style = 0;

	if (renderwindow->fullscreen) {
		ce_displaymng_change(renderwindow->displaymng,
			renderwindow->width, renderwindow->height, renderwindow->bpp,
			renderwindow->rate, renderwindow->rotation, renderwindow->reflection);

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

	win_renderwindow->window = CreateWindowEx(extended_style,
		wc.lpszClassName, title, style, CW_USEDEFAULT, CW_USEDEFAULT,
		renderwindow->width, renderwindow->height,
		HWND_DESKTOP, NULL, wc.hInstance, NULL);

	if (NULL == win_renderwindow->window) {
		ce_logging_fatal("renderwindow: could not create window");
		ce_renderwindow_del(renderwindow);
		return NULL;
	}

	SetWindowLongPtr(win_renderwindow->window, GWLP_USERDATA, (LONG_PTR)renderwindow);

	renderwindow->context = ce_context_create(GetDC(win_renderwindow->window));
	if (NULL == renderwindow->context) {
		ce_logging_fatal("renderwindow: could not create context");
		ce_renderwindow_del(renderwindow);
		return NULL;
	}

	return renderwindow;
}

void ce_renderwindow_del(ce_renderwindow* renderwindow)
{
	if (NULL != renderwindow) {
		ce_input_context_del(renderwindow->input_context);
		ce_context_del(renderwindow->context);
		ce_displaymng_del(renderwindow->displaymng);

		ce_renderwindow_win* win_renderwindow = (ce_renderwindow_win*)renderwindow->impl;
		if (NULL != win_renderwindow->window) {
			ReleaseDC(win_renderwindow->window, GetDC(win_renderwindow->window));
			DestroyWindow(win_renderwindow->window);
		}

		UnregisterClass("cursedearth", GetModuleHandle(NULL));
		ce_free(renderwindow, sizeof(ce_renderwindow) + sizeof(ce_renderwindow_win));
	}
}

void ce_renderwindow_show(ce_renderwindow* renderwindow)
{
	ce_renderwindow_win* win_renderwindow = (ce_renderwindow_win*)renderwindow->impl;

	ShowWindow(win_renderwindow->window, SW_SHOW);
	UpdateWindow(win_renderwindow->window);

	SetForegroundWindow(win_renderwindow->window);
	SetFocus(win_renderwindow->window);
}

void ce_renderwindow_toggle_fullscreen(ce_renderwindow* renderwindow)
{
	ce_unused(renderwindow);
}

void ce_renderwindow_minimize(ce_renderwindow* renderwindow)
{
	ce_unused(renderwindow);
}

void ce_renderwindow_pump(ce_renderwindow* renderwindow)
{
	ce_unused(renderwindow);
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		//if (WM_QUIT == msg.message) {
		//	return;
		//}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
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
