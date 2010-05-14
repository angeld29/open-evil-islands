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
#include <windowsx.h>

#include <ddk/hidusage.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cerenderwindow.h"

#include "cedisplay_win32.h"
#include "cecontext_win32.h"

enum {
	CE_WINWINDOW_STATE_WINDOW,
	CE_WINWINDOW_STATE_FULLSCREEN,
	CE_WINWINDOW_STATE_COUNT
};

enum {
	CE_RENDERWINDOW_RID_KEYBOARD,
	CE_RENDERWINDOW_RID_MOUSE,
	CE_RENDERWINDOW_RID_COUNT
};

typedef struct {
	DWORD style[CE_WINWINDOW_STATE_COUNT];
	DWORD extended_style[CE_WINWINDOW_STATE_COUNT];
	HWND window;
	void (*handlers[WM_USER])(ce_renderwindow*, WPARAM, LPARAM);
	RAWINPUTDEVICE rid[CE_RENDERWINDOW_RID_COUNT];
	DWORD foreground_lock_timeout;
	int min_animate;
} ce_renderwindow_win;

static void ce_renderwindow_handler_skip(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_close(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_destroy(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_size(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_syscommand(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_input(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_keydown(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_keyup(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_lbuttondown(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_lbuttonup(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_mbuttondown(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_mbuttonup(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_rbuttondown(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_rbuttonup(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_mousewheel(ce_renderwindow*, WPARAM, LPARAM);
static void ce_renderwindow_handler_mousemove(ce_renderwindow*, WPARAM, LPARAM);

static LRESULT CALLBACK ce_renderwindow_proc(HWND, UINT, WPARAM, LPARAM);

ce_renderwindow* ce_renderwindow_new(const char* title, int width, int height)
{
	ce_renderwindow* renderwindow = ce_alloc_zero(sizeof(ce_renderwindow) +
													sizeof(ce_renderwindow_win));
	ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;

	for (int i = 0; i < CE_WINWINDOW_STATE_COUNT; ++i) {
		winwindow->style[i] = WS_VISIBLE | (DWORD[])
			{ WS_OVERLAPPEDWINDOW, WS_POPUP }[CE_WINWINDOW_STATE_FULLSCREEN == i];
		winwindow->extended_style[i] = WS_EX_APPWINDOW;
	}

	for (int i = 0; i < WM_USER; ++i) {
		winwindow->handlers[i] = ce_renderwindow_handler_skip;
	}

	winwindow->handlers[WM_CLOSE] = ce_renderwindow_handler_close;
	winwindow->handlers[WM_DESTROY] = ce_renderwindow_handler_destroy;
	winwindow->handlers[WM_SIZE] = ce_renderwindow_handler_size;
	winwindow->handlers[WM_SYSCOMMAND] = ce_renderwindow_handler_syscommand;
	winwindow->handlers[WM_INPUT] = ce_renderwindow_handler_input;
	winwindow->handlers[WM_KEYDOWN] = ce_renderwindow_handler_keydown;
	winwindow->handlers[WM_SYSKEYDOWN] = ce_renderwindow_handler_keydown;
	winwindow->handlers[WM_KEYUP] = ce_renderwindow_handler_keyup;
	winwindow->handlers[WM_SYSKEYUP] = ce_renderwindow_handler_keyup;
	winwindow->handlers[WM_LBUTTONDOWN] = ce_renderwindow_handler_lbuttondown;
	winwindow->handlers[WM_LBUTTONUP] = ce_renderwindow_handler_lbuttonup;
	winwindow->handlers[WM_MBUTTONDOWN] = ce_renderwindow_handler_mbuttondown;
	winwindow->handlers[WM_MBUTTONUP] = ce_renderwindow_handler_mbuttonup;
	winwindow->handlers[WM_RBUTTONDOWN] = ce_renderwindow_handler_rbuttondown;
	winwindow->handlers[WM_RBUTTONUP] = ce_renderwindow_handler_rbuttonup;
	winwindow->handlers[WM_MOUSEWHEEL] = ce_renderwindow_handler_mousewheel;
	winwindow->handlers[WM_MOUSEMOVE] = ce_renderwindow_handler_mousemove;

	winwindow->rid[CE_RENDERWINDOW_RID_KEYBOARD].usUsagePage = HID_USAGE_PAGE_GENERIC;
	winwindow->rid[CE_RENDERWINDOW_RID_KEYBOARD].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	winwindow->rid[CE_RENDERWINDOW_RID_KEYBOARD].dwFlags = RIDEV_NOLEGACY;
	winwindow->rid[CE_RENDERWINDOW_RID_KEYBOARD].hwndTarget = NULL;

	winwindow->rid[CE_RENDERWINDOW_RID_MOUSE].usUsagePage = HID_USAGE_PAGE_GENERIC;
	winwindow->rid[CE_RENDERWINDOW_RID_MOUSE].usUsage = HID_USAGE_GENERIC_MOUSE;
	winwindow->rid[CE_RENDERWINDOW_RID_MOUSE].dwFlags = RIDEV_NOLEGACY;
	winwindow->rid[CE_RENDERWINDOW_RID_MOUSE].hwndTarget = NULL;

	// TODO: implement it
	//if (RegisterRawInputDevices(winwindow->rid, 2, sizeof(RAWINPUTDEVICE))) {
	//	ce_logging_warning("renderwindow: using raw input");
	//} else {
	//	ce_logging_error("renderwindow: could not register raw input devices");
	//	ce_logging_warning("renderwindow: using event-driven input");
	//}

	SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0,
						&winwindow->foreground_lock_timeout, 0);
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,
						0, (PVOID)0, SPIF_SENDCHANGE);

	ANIMATIONINFO animinfo = { .cbSize = sizeof(ANIMATIONINFO) };
	SystemParametersInfo(SPI_GETANIMATION, animinfo.cbSize, &animinfo, 0);

	winwindow->min_animate = animinfo.iMinAnimate;
	animinfo.iMinAnimate = 0;

	SystemParametersInfo(SPI_SETANIMATION, animinfo.cbSize, &animinfo, 0);

	width = ce_max(400, width);
	height = ce_max(300, height);

	RECT rect = { 0, 0, width, height };
	if (AdjustWindowRectEx(&rect, winwindow->style[CE_WINWINDOW_STATE_WINDOW],
			0, winwindow->extended_style[CE_WINWINDOW_STATE_WINDOW])) {
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}

	renderwindow->width = width;
	renderwindow->height = height;

	renderwindow->displaymng = ce_displaymng_create();
	renderwindow->input_context = ce_input_context_new();
	renderwindow->listeners = ce_vector_new();

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = ce_renderwindow_proc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "cursedearth";

	if (!RegisterClassEx(&wc)) {
		ce_logging_fatal("renderwindow: could not register class");
		ce_renderwindow_del(renderwindow);
		return NULL;
	}

	winwindow->window = CreateWindowEx(winwindow->extended_style[CE_WINWINDOW_STATE_WINDOW],
		wc.lpszClassName, title, winwindow->style[CE_WINWINDOW_STATE_WINDOW],
		CW_USEDEFAULT, CW_USEDEFAULT, renderwindow->width, renderwindow->height,
		HWND_DESKTOP, NULL, wc.hInstance, NULL);

	if (NULL == winwindow->window) {
		ce_logging_fatal("renderwindow: could not create window");
		ce_renderwindow_del(renderwindow);
		return NULL;
	}

	renderwindow->context = ce_context_create(GetDC(winwindow->window));
	if (NULL == renderwindow->context) {
		ce_logging_fatal("renderwindow: could not create context");
		ce_renderwindow_del(renderwindow);
		return NULL;
	}

	SetWindowLongPtr(winwindow->window, GWLP_USERDATA, (LONG_PTR)renderwindow);

	return renderwindow;
}

void ce_renderwindow_del(ce_renderwindow* renderwindow)
{
	if (NULL != renderwindow) {
		ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;

		ce_vector_del(renderwindow->listeners);
		ce_input_context_del(renderwindow->input_context);
		ce_context_del(renderwindow->context);
		ce_displaymng_del(renderwindow->displaymng);

		ANIMATIONINFO animinfo = { .cbSize = sizeof(ANIMATIONINFO),
			.iMinAnimate = winwindow->min_animate };

		SystemParametersInfo(SPI_SETANIMATION, animinfo.cbSize, &animinfo, 0);

		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0,
			(PVOID)winwindow->foreground_lock_timeout, SPIF_SENDCHANGE);

		if (NULL != winwindow->window) {
			ReleaseDC(winwindow->window, GetDC(winwindow->window));
			DestroyWindow(winwindow->window);
		}

		UnregisterClass("cursedearth", GetModuleHandle(NULL));
		ce_free(renderwindow, sizeof(ce_renderwindow) + sizeof(ce_renderwindow_win));
	}
}

void ce_renderwindow_show(ce_renderwindow* renderwindow)
{
	ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;

	//SetWindowPos(winwindow->window, HWND_TOPMOST,
	//	0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	ShowWindow(winwindow->window, SW_SHOW);
	UpdateWindow(winwindow->window);

	SetForegroundWindow(winwindow->window);
	SetFocus(winwindow->window);
}

void ce_renderwindow_toggle_fullscreen(ce_renderwindow* renderwindow)
{
	ce_unused(renderwindow);
	//ce_displaymng_change(renderwindow->displaymng,
	//	renderwindow->width, renderwindow->height, renderwindow->bpp,
	//	renderwindow->rate, renderwindow->rotation, renderwindow->reflection);
}

void ce_renderwindow_minimize(ce_renderwindow* renderwindow)
{
	ce_unused(renderwindow);
	// TODO: implement it
	//WINDOWPLACEMENT wndpl;
	//wndpl.length = sizeof(WINDOWPLACEMENT);
	//GetWindowPlacement(HWnd, &wndpl);
	//wndpl.showCmd = SW_SHOWMINNOACTIVE;
	//SetWindowPlacement(HWnd, &wndpl);
}

void ce_renderwindow_pump(ce_renderwindow* renderwindow)
{
	// reset pointer offset every frame
	renderwindow->input_context->pointer_offset = CE_VEC2_ZERO;

	// special case: reset wheel buttons
	renderwindow->input_context->buttons[CE_MB_WHEELUP] = false;
	renderwindow->input_context->buttons[CE_MB_WHEELDOWN] = false;

	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

static void ce_renderwindow_handler_skip(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_close(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(wparam), ce_unused(lparam);
	ce_renderwindow_emit_closed(renderwindow);
}

static void ce_renderwindow_handler_destroy(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
	PostQuitMessage(0);
}

static void ce_renderwindow_handler_size(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(wparam);
	renderwindow->width = LOWORD(lparam);
	renderwindow->height = HIWORD(lparam);
}

static void ce_renderwindow_handler_syscommand(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
	// TODO: implement it
	// prevent screensaver or monitor powersave mode from starting
	// fullscreen only!
	//if (SC_SCREENSAVE == (wparam & 0xFFF0) || SC_MONITORPOWER == (wparam & 0xFFF0))
	//	return 0;
}

static void ce_renderwindow_handler_input(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
	/*printf("ce_renderwindow_handler_input\n");

	UINT size;
	GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));

	LPBYTE lpb = ce_alloc(size);
	GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &size, sizeof(RAWINPUTHEADER));

	RAWINPUT* raw = (RAWINPUT*)lpb;

	if (raw->header.dwType == RIM_TYPEKEYBOARD) {
		//raw->data.keyboard.MakeCode;
		//raw->data.keyboard.Message;
		//raw->data.keyboard.VKey;
	} else if (raw->header.dwType == RIM_TYPEMOUSE) {
		//raw->data.mouse.usFlags;
		//raw->data.mouse.usButtonFlags;
		//raw->data.mouse.usButtonData;
		//raw->data.mouse.ulRawButtons;
		//raw->data.mouse.lLastX;
		//raw->data.mouse.lLastY;
	}

	ce_free(lpb, size);*/
}

static void ce_renderwindow_handler_keydown(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_keyup(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(renderwindow), ce_unused(wparam), ce_unused(lparam);
}

static void ce_renderwindow_handler_button(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam, ce_input_button button, bool pressed)
{
	ce_unused(wparam);
	renderwindow->input_context->pointer_position.x = GET_X_LPARAM(lparam);
	renderwindow->input_context->pointer_position.y = GET_Y_LPARAM(lparam);
	renderwindow->input_context->buttons[button] = pressed;
}

static void ce_renderwindow_handler_lbuttondown(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_LEFT, true);
}

static void ce_renderwindow_handler_lbuttonup(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_LEFT, false);
}

static void ce_renderwindow_handler_mbuttondown(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_MIDDLE, true);
}

static void ce_renderwindow_handler_mbuttonup(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_MIDDLE, false);
}

static void ce_renderwindow_handler_rbuttondown(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_RIGHT, true);
}

static void ce_renderwindow_handler_rbuttonup(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_RIGHT, false);
}

static void ce_renderwindow_handler_mousewheel(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_renderwindow_handler_button(renderwindow, wparam, lparam, (ce_input_button[])
		{ CE_MB_WHEELUP, CE_MB_WHEELDOWN }[GET_WHEEL_DELTA_WPARAM(wparam) < 0], true);
}

static void ce_renderwindow_handler_mousemove(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
{
	ce_unused(wparam);

	renderwindow->input_context->pointer_offset.x = GET_X_LPARAM(lparam) -
		renderwindow->input_context->pointer_position.x;
	renderwindow->input_context->pointer_offset.y = GET_Y_LPARAM(lparam) -
		renderwindow->input_context->pointer_position.y;

	renderwindow->input_context->pointer_position.x = GET_X_LPARAM(lparam);
	renderwindow->input_context->pointer_position.y = GET_Y_LPARAM(lparam);
}

static LRESULT CALLBACK ce_renderwindow_proc(HWND window, UINT message,
											WPARAM wparam, LPARAM lparam)
{
	ce_renderwindow* renderwindow =
		(ce_renderwindow*)GetWindowLongPtr(window, GWLP_USERDATA);

	if (NULL != renderwindow && message < WM_USER) {
		ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
		(*winwindow->handlers[message])(renderwindow, wparam, lparam);
	}

	return DefWindowProc(window, message, wparam, lparam);
}
