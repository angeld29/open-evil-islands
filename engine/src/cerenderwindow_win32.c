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

typedef struct ce_renderwindow_win {
	ce_renderwindow* renderwindow;
	HWND window;
	void (*handlers[WM_USER])(struct ce_renderwindow_win*, WPARAM, LPARAM);
} ce_renderwindow_win;

static void ce_renderwindow_handler_skip(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_close(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_destroy(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_size(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_keydown(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_keyup(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_lbuttondown(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_lbuttonup(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_mbuttondown(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_mbuttonup(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_rbuttondown(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_rbuttonup(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_mousewheel(ce_renderwindow_win*, WPARAM, LPARAM);
static void ce_renderwindow_handler_mousemove(ce_renderwindow_win*, WPARAM, LPARAM);

static LRESULT CALLBACK ce_renderwindow_proc(HWND, UINT, WPARAM, LPARAM);

ce_renderwindow* ce_renderwindow_new(const char* title, int width, int height)
{
	ce_renderwindow* renderwindow = ce_alloc_zero(sizeof(ce_renderwindow) +
													sizeof(ce_renderwindow_win));

	ce_renderwindow_win* win_renderwindow = (ce_renderwindow_win*)renderwindow->impl;
	win_renderwindow->renderwindow = renderwindow;

	for (int i = 0; i < WM_USER; ++i) {
		win_renderwindow->handlers[i] = ce_renderwindow_handler_skip;
	}

	win_renderwindow->handlers[WM_CLOSE] = ce_renderwindow_handler_close;
	win_renderwindow->handlers[WM_DESTROY] = ce_renderwindow_handler_destroy;
	win_renderwindow->handlers[WM_SIZE] = ce_renderwindow_handler_size;
	win_renderwindow->handlers[WM_KEYDOWN] = ce_renderwindow_handler_keydown;
	win_renderwindow->handlers[WM_KEYUP] = ce_renderwindow_handler_keyup;
	win_renderwindow->handlers[WM_LBUTTONDOWN] = ce_renderwindow_handler_lbuttondown;
	win_renderwindow->handlers[WM_LBUTTONUP] = ce_renderwindow_handler_lbuttonup;
	win_renderwindow->handlers[WM_MBUTTONDOWN] = ce_renderwindow_handler_mbuttondown;
	win_renderwindow->handlers[WM_MBUTTONUP] = ce_renderwindow_handler_mbuttonup;
	win_renderwindow->handlers[WM_RBUTTONDOWN] = ce_renderwindow_handler_rbuttondown;
	win_renderwindow->handlers[WM_RBUTTONUP] = ce_renderwindow_handler_rbuttonup;
	win_renderwindow->handlers[WM_MOUSEWHEEL] = ce_renderwindow_handler_mousewheel;
	win_renderwindow->handlers[WM_MOUSEMOVE] = ce_renderwindow_handler_mousemove;

	renderwindow->width = width;
	renderwindow->height = height;

	renderwindow->displaymng = ce_displaymng_create();
	renderwindow->input_context = ce_input_context_new();
	renderwindow->listeners = ce_vector_new();

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

	SetWindowLongPtr(win_renderwindow->window, GWLP_USERDATA, (LONG_PTR)win_renderwindow);

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
		ce_vector_for_each(renderwindow->listeners, ce_renderwindow_listener_tuple_del);
		ce_vector_del(renderwindow->listeners);
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
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

static void ce_renderwindow_handler_skip(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_close(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(wparam), ce_unused(lparam);
	ce_renderwindow_emit_closed(renderwindow->renderwindow);
}

static void ce_renderwindow_handler_destroy(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
	PostQuitMessage(0);
}

static void ce_renderwindow_handler_size(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(wparam);
	renderwindow->renderwindow->width = LOWORD(lparam);
	renderwindow->renderwindow->height = HIWORD(lparam);
}

static void ce_renderwindow_handler_keydown(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_keyup(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_lbuttondown(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_lbuttonup(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_mbuttondown(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_mbuttonup(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_rbuttondown(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_rbuttonup(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_mousewheel(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_mousemove(ce_renderwindow_win* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static LRESULT CALLBACK ce_renderwindow_proc(HWND window, UINT message,
											WPARAM wparam, LPARAM lparam)
{
	ce_renderwindow_win* renderwindow =
		(ce_renderwindow_win*)GetWindowLongPtr(window, GWLP_USERDATA);

	if (NULL != renderwindow && message < WM_USER) {
		(*renderwindow->handlers[message])(renderwindow, wparam, lparam);
	}

	return DefWindowProc(window, message, wparam, lparam);
}
