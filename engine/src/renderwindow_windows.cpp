/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <cstdio>
#include <cassert>

#include <windows.h>
#include <windowsx.h>

#include <ddk/hidusage.h>

#include "alloc.hpp"
#include "logging.hpp"
#include "display_windows.hpp"
#include "graphicscontext_windows.hpp"
#include "renderwindow.hpp"

namespace cursedearth
{
    enum {
        CE_RENDERWINDOW_RID_KEYBOARD,
        CE_RENDERWINDOW_RID_MOUSE,
        CE_RENDERWINDOW_RID_COUNT
    };

    enum {
        CE_RENDERWINDOW_HOTKEY_ALTTAB,
        CE_RENDERWINDOW_HOTKEY_LWIN,
        CE_RENDERWINDOW_HOTKEY_RWIN,
        CE_RENDERWINDOW_HOTKEY_COUNT
    };

    struct {
        UINT mod;
        UINT vk;
    } ce_renderwindow_hotkeys[CE_RENDERWINDOW_HOTKEY_COUNT] = {
        { MOD_ALT, VK_TAB },
        { MOD_WIN, VK_LWIN },
        { MOD_WIN, VK_RWIN },
    };

    typedef struct {
        DWORD style[CE_RENDERWINDOW_STATE_COUNT];
        DWORD extended_style[CE_RENDERWINDOW_STATE_COUNT];
        RAWINPUTDEVICE rid[CE_RENDERWINDOW_RID_COUNT];
        bool (*handlers[WM_USER])(ce_renderwindow*, WPARAM, LPARAM);
        bool in_sizemove;
        bool cursor_inside;
        HWND window;
    } ce_renderwindow_win;

    LRESULT CALLBACK ce_renderwindow_proc(HWND, UINT, WPARAM, LPARAM);

    bool ce_renderwindow_handler_skip(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_close(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_entersizemove(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_exitsizemove(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_windowposchanged(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_size(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_activate(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_syscommand(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_killfocus(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_hotkey(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_input(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_keydown(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_keyup(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_lbuttondown(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_lbuttonup(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mbuttondown(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mbuttonup(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_rbuttondown(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_rbuttonup(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mousewheel(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mousehover(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mouseleave(ce_renderwindow*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mousemove(ce_renderwindow*, WPARAM, LPARAM);

    bool ce_renderwindow_win_ctor(ce_renderwindow* renderwindow, va_list args)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        const char* title = va_arg(args, const char*);

        winwindow->style[CE_RENDERWINDOW_STATE_WINDOW] = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
        winwindow->style[CE_RENDERWINDOW_STATE_FULLSCREEN] = WS_VISIBLE | WS_POPUP;

        winwindow->extended_style[CE_RENDERWINDOW_STATE_WINDOW] = WS_EX_APPWINDOW;
        winwindow->extended_style[CE_RENDERWINDOW_STATE_FULLSCREEN] = WS_EX_TOOLWINDOW;

        for (int i = 0; i < CE_RENDERWINDOW_STATE_COUNT; ++i) {
            RECT rect = { 0, 0, renderwindow->geometry[i].width,
                                renderwindow->geometry[i].height };
            if (AdjustWindowRectEx(&rect, winwindow->style[i],
                                    FALSE, winwindow->extended_style[i])) {
                renderwindow->geometry[i].width = rect.right - rect.left;
                renderwindow->geometry[i].height = rect.bottom - rect.top;
            }
        }

        renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].x =
            (GetSystemMetrics(SM_CXSCREEN) -
            renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].width) / 2;

        renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].y =
            (GetSystemMetrics(SM_CYSCREEN) -
            renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].height) / 2;

        renderwindow->displaymng = ce_displaymng_create();

        const unsigned long keys[CE_IB_COUNT] = {
            0, VK_ESCAPE, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8,
            VK_F9, VK_F10, VK_F11, VK_F12, VK_OEM_3, '0', '1', '2', '3', '4', '5',
            '6', '7', '8', '9', VK_OEM_MINUS, VK_OEM_PLUS, VK_OEM_5, VK_BACK, VK_TAB,
            'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', VK_OEM_4, VK_OEM_6,
            VK_CAPITAL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', VK_OEM_1,
            VK_OEM_7, VK_RETURN, VK_LSHIFT, 'Z', 'X', 'C', 'V', 'B', 'N', 'M',
            VK_OEM_COMMA, VK_OEM_PERIOD, VK_OEM_2, VK_RSHIFT, VK_LCONTROL, VK_LWIN,
            VK_LMENU, VK_SPACE, VK_RMENU, VK_RWIN, VK_APPS, VK_RCONTROL, VK_SNAPSHOT,
            VK_SCROLL, VK_PAUSE, VK_INSERT, VK_DELETE, VK_HOME, VK_END, VK_PRIOR,
            VK_NEXT, VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN, VK_NUMLOCK, VK_DIVIDE,
            VK_MULTIPLY, VK_SUBTRACT, VK_ADD, VK_EXECUTE, VK_DECIMAL, VK_NUMPAD7,
            VK_NUMPAD8, VK_NUMPAD9, VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD1,
            VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD0, 0, 0, 0, 0, 0
        };

        ce_renderwindow_keymap_add_array(renderwindow->keymap, keys);
        ce_renderwindow_keymap_sort(renderwindow->keymap);

        winwindow->rid[CE_RENDERWINDOW_RID_KEYBOARD].usUsagePage = HID_USAGE_PAGE_GENERIC;
        winwindow->rid[CE_RENDERWINDOW_RID_KEYBOARD].usUsage = HID_USAGE_GENERIC_KEYBOARD;
        winwindow->rid[CE_RENDERWINDOW_RID_KEYBOARD].dwFlags = RIDEV_NOLEGACY;
        winwindow->rid[CE_RENDERWINDOW_RID_KEYBOARD].hwndTarget = NULL;

        winwindow->rid[CE_RENDERWINDOW_RID_MOUSE].usUsagePage = HID_USAGE_PAGE_GENERIC;
        winwindow->rid[CE_RENDERWINDOW_RID_MOUSE].usUsage = HID_USAGE_GENERIC_MOUSE;
        winwindow->rid[CE_RENDERWINDOW_RID_MOUSE].dwFlags = RIDEV_NOLEGACY;
        winwindow->rid[CE_RENDERWINDOW_RID_MOUSE].hwndTarget = NULL;

        // TODO: implement RID
        //if (RegisterRawInputDevices(winwindow->rid, 2, sizeof(RAWINPUTDEVICE))) {
        //	ce_logging_warning("renderwindow: using raw input");
        //} else {
        //	ce_logging_error("renderwindow: could not register raw input devices");
        //	ce_logging_warning("renderwindow: using event-driven input");
        //}

        for (int i = 0; i < WM_USER; ++i) {
            winwindow->handlers[i] = ce_renderwindow_handler_skip;
        }

        winwindow->handlers[WM_CLOSE] = ce_renderwindow_handler_close;
        winwindow->handlers[WM_ENTERSIZEMOVE] = ce_renderwindow_handler_entersizemove;
        winwindow->handlers[WM_EXITSIZEMOVE] = ce_renderwindow_handler_exitsizemove;
        winwindow->handlers[WM_WINDOWPOSCHANGED] = ce_renderwindow_handler_windowposchanged;
        winwindow->handlers[WM_SIZE] = ce_renderwindow_handler_size;
        winwindow->handlers[WM_ACTIVATE] = ce_renderwindow_handler_activate;
        winwindow->handlers[WM_SYSCOMMAND] = ce_renderwindow_handler_syscommand;
        winwindow->handlers[WM_KILLFOCUS] = ce_renderwindow_handler_killfocus;
        winwindow->handlers[WM_HOTKEY] = ce_renderwindow_handler_hotkey;
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
        winwindow->handlers[WM_MOUSEHOVER] = ce_renderwindow_handler_mousehover;
        winwindow->handlers[WM_MOUSELEAVE] = ce_renderwindow_handler_mouseleave;
        winwindow->handlers[WM_MOUSEMOVE] = ce_renderwindow_handler_mousemove;

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
            return false;
        }

        winwindow->window = CreateWindowEx(winwindow->extended_style[renderwindow->state],
            wc.lpszClassName, title, winwindow->style[renderwindow->state],
            renderwindow->geometry[renderwindow->state].x, renderwindow->geometry[renderwindow->state].y,
            renderwindow->geometry[renderwindow->state].width, renderwindow->geometry[renderwindow->state].height,
            HWND_DESKTOP, NULL, wc.hInstance, NULL);

        if (NULL == winwindow->window) {
            ce_logging_fatal("renderwindow: could not create window");
            return false;
        }

        SetWindowLongPtr(winwindow->window, GWLP_USERDATA, (LONG_PTR)renderwindow);

        renderwindow->graphics_context = ce_graphics_context_new(GetDC(winwindow->window));
        if (NULL == renderwindow->graphics_context) {
            ce_logging_fatal("renderwindow: could not create graphic context");
            return false;
        }

        return true;
    }

    void ce_renderwindow_win_dtor(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;

        ce_graphics_context_del(renderwindow->graphics_context);
        ce_displaymng_del(renderwindow->displaymng);

        if (NULL != winwindow->window) {
            for (int i = 0; i < CE_RENDERWINDOW_HOTKEY_COUNT; ++i) {
                UnregisterHotKey(winwindow->window, i);
            }

            ReleaseDC(winwindow->window, GetDC(winwindow->window));
            DestroyWindow(winwindow->window);
        }

        UnregisterClass("cursedearth", GetModuleHandle(NULL));
    }

    void ce_renderwindow_win_show(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;

        ShowWindow(winwindow->window, SW_SHOW);
        UpdateWindow(winwindow->window);

        SetForegroundWindow(winwindow->window);
        SetFocus(winwindow->window);

        // WM_SIZE event is not coming, so force it
        RECT rect;
        if (GetClientRect(winwindow->window, &rect)) {
            ce_renderwindow_emit_resized(renderwindow, rect.right, rect.bottom);
        }
    }

    void ce_renderwindow_win_minimize(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        ShowWindow(winwindow->window, SW_MINIMIZE);
    }

    void ce_renderwindow_win_fullscreen_before_enter(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        for (int i = 0; i < CE_RENDERWINDOW_HOTKEY_COUNT; ++i) {
            RegisterHotKey(winwindow->window, i, ce_renderwindow_hotkeys[i].mod, ce_renderwindow_hotkeys[i].vk);
        }
    }

    void ce_renderwindow_win_fullscreen_after_exit(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;

        for (int i = 0; i < CE_RENDERWINDOW_HOTKEY_COUNT; ++i) {
            UnregisterHotKey(winwindow->window, i);
        }
    }

    void ce_renderwindow_win_fullscreen_done(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;

        SetWindowLong(winwindow->window, GWL_STYLE, winwindow->style[renderwindow->state]);
        SetWindowLong(winwindow->window, GWL_EXSTYLE, winwindow->extended_style[renderwindow->state]);

        SetWindowPos(winwindow->window, HWND_TOP,
            renderwindow->geometry[renderwindow->state].x,
            renderwindow->geometry[renderwindow->state].y,
            renderwindow->geometry[renderwindow->state].width,
            renderwindow->geometry[renderwindow->state].height,
            SWP_FRAMECHANGED);
    }

    void ce_renderwindow_win_pump(ce_renderwindow*)
    {
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    ce_renderwindow* ce_renderwindow_create(int width, int height, const char* title)
    {
        ce_renderwindow_vtable vt = {ce_renderwindow_win_ctor, ce_renderwindow_win_dtor, ce_renderwindow_win_show, ce_renderwindow_win_minimize,
            {NULL, ce_renderwindow_win_fullscreen_before_enter,NULL, NULL,ce_renderwindow_win_fullscreen_after_exit, ce_renderwindow_win_fullscreen_done}, ce_renderwindow_win_pump};
        return ce_renderwindow_new(vt, sizeof(ce_renderwindow_win), width, height, title);
    }

    LRESULT CALLBACK ce_renderwindow_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
    {
        ce_renderwindow* renderwindow = (ce_renderwindow*)GetWindowLongPtr(window, GWLP_USERDATA);
        if (NULL != renderwindow && message < WM_USER) {
            ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
            if ((*winwindow->handlers[message])(renderwindow, wparam, lparam)) {
                return 0;
            }
        }
        return DefWindowProc(window, message, wparam, lparam);
    }

    bool ce_renderwindow_handler_skip(ce_renderwindow*, WPARAM, LPARAM)
    {
        return false;
    }

    bool ce_renderwindow_handler_close(ce_renderwindow* renderwindow, WPARAM, LPARAM)
    {
        ce_renderwindow_emit_closed(renderwindow);
        // suppress the WM_CLOSE message to prevent the OS
        // from automatically destroying the window
        return true;
    }

    bool ce_renderwindow_handler_entersizemove(ce_renderwindow* renderwindow, WPARAM, LPARAM)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        winwindow->in_sizemove = true;
        return false;
    }

    bool ce_renderwindow_handler_exitsizemove(ce_renderwindow* renderwindow, WPARAM, LPARAM)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        winwindow->in_sizemove = false;
        return false;
    }

    bool ce_renderwindow_handler_windowposchanged(ce_renderwindow* renderwindow, WPARAM, LPARAM lparam)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;

        if (winwindow->in_sizemove) {
            WINDOWPOS* wp = (WINDOWPOS*)lparam;

            if (!(SWP_NOMOVE & wp->flags)) {
                renderwindow->geometry[renderwindow->state].x = wp->x;
                renderwindow->geometry[renderwindow->state].y = wp->y;
            }

            if (!(SWP_NOSIZE & wp->flags)) {
                // new window width and height
                renderwindow->geometry[renderwindow->state].width = wp->cx;
                renderwindow->geometry[renderwindow->state].height = wp->cy;
            }
        }

        return false;
    }

    bool ce_renderwindow_handler_size(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        if (SIZE_MINIMIZED != wparam) {
            // new width and height of the client area
            ce_renderwindow_emit_resized(renderwindow, LOWORD(lparam), HIWORD(lparam));
        }

        return false;
    }

    bool ce_renderwindow_handler_activate(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM)
    {
        if (WA_INACTIVE == LOWORD(wparam)) {
            if (CE_RENDERWINDOW_STATE_FULLSCREEN == renderwindow->state) {
                assert(CE_RENDERWINDOW_ACTION_NONE == renderwindow->action);
                renderwindow->action = CE_RENDERWINDOW_ACTION_MINIMIZE;
            }
        }

        return false;
    }

    bool ce_renderwindow_handler_syscommand(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM)
    {
        wparam &= 0xfff0;

        if (CE_RENDERWINDOW_STATE_FULLSCREEN == renderwindow->state && (SC_MONITORPOWER == wparam || SC_SCREENSAVE == wparam)) {
            // prevent monitor powersave mode or screensaver from starting
            return true;
        }

        if (SC_RESTORE == wparam) {
            assert(CE_RENDERWINDOW_ACTION_NONE == renderwindow->action);
            renderwindow->action = CE_RENDERWINDOW_ACTION_RESTORED;
        }

        return false;
    }

    bool ce_renderwindow_handler_killfocus(ce_renderwindow* renderwindow, WPARAM, LPARAM)
    {
        ce_input_context_clear(renderwindow->input_context);
        return false;
    }

    bool ce_renderwindow_handler_hotkey(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM)
    {
        // hotkeys are active only in fullscreen mode
        if (CE_RENDERWINDOW_HOTKEY_ALTTAB == wparam || CE_RENDERWINDOW_HOTKEY_LWIN == wparam || CE_RENDERWINDOW_HOTKEY_RWIN == wparam) {
            assert(CE_RENDERWINDOW_ACTION_NONE == renderwindow->action);
            renderwindow->action = CE_RENDERWINDOW_ACTION_MINIMIZE;
        }

        return false;
    }

    bool ce_renderwindow_handler_input(ce_renderwindow*, WPARAM, LPARAM)
    {
        // TODO: implement RID
        //printf("ce_renderwindow_handler_input\n");

        /*UINT size;
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

        return false;
    }

    bool ce_renderwindow_handler_key(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam, bool pressed)
    {
        switch (wparam) {
        case VK_RETURN:
            // from MSDN: "Original equipment manufacturers should not use
            // the unassigned portions of the VK mapping tables.
            // Microsoft will assign these values in the future. If manufacturers
            // require additional VK mappings, they should reuse some of the
            // current manufacturer-specific and vendor-specific assignments."
            // no problem: use VK_EXECUTE to simulate missing VK_NUMPADRETURN
            wparam = HIWORD(lparam) & KF_EXTENDED ? VK_EXECUTE : VK_RETURN;
            break;
        // see MSDN WM_KEYDOWN/WM_KEYUP Message for next VKs
        case VK_SHIFT:
            wparam = MapVirtualKey(HIWORD(lparam) & 0xff, MAPVK_VSC_TO_VK_EX);
            break;
        case VK_CONTROL:
            wparam = HIWORD(lparam) & KF_EXTENDED ? VK_RCONTROL : VK_LCONTROL;
            break;
        case VK_MENU:
            wparam = HIWORD(lparam) & KF_EXTENDED ? VK_RMENU : VK_LMENU;
            break;
        }

        renderwindow->input_context->buttons[ce_renderwindow_keymap_search(renderwindow->keymap, wparam)] = pressed;

        return false;
    }

    bool ce_renderwindow_handler_keydown(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_key(renderwindow, wparam, lparam, true);
    }

    bool ce_renderwindow_handler_keyup(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_key(renderwindow, wparam, lparam, false);
    }

    bool ce_renderwindow_handler_button(ce_renderwindow* renderwindow, WPARAM, LPARAM lparam, ce_input_button button, bool pressed)
    {
        // TODO: undefined behavior in windows mode
        if (CE_RENDERWINDOW_STATE_FULLSCREEN == renderwindow->state) {
            renderwindow->input_context->pointer_position.x = GET_X_LPARAM(lparam);
            renderwindow->input_context->pointer_position.y = GET_Y_LPARAM(lparam);
        }

        renderwindow->input_context->buttons[button] = pressed;

        return false;
    }

    bool ce_renderwindow_handler_lbuttondown(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_LEFT, true);
    }

    bool ce_renderwindow_handler_lbuttonup(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_LEFT, false);
    }

    bool ce_renderwindow_handler_mbuttondown(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_MIDDLE, true);
    }

    bool ce_renderwindow_handler_mbuttonup(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_MIDDLE, false);
    }

    bool ce_renderwindow_handler_rbuttondown(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_RIGHT, true);
    }

    bool ce_renderwindow_handler_rbuttonup(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, CE_MB_RIGHT, false);
    }

    bool ce_renderwindow_handler_mousewheel(ce_renderwindow* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        if (winwindow->cursor_inside) {
            return ce_renderwindow_handler_button(renderwindow, wparam, lparam,
                (GET_WHEEL_DELTA_WPARAM(wparam) < 0 ? CE_MB_WHEELDOWN : CE_MB_WHEELUP), true);
        }
        return false;
    }

    bool ce_renderwindow_handler_mousehover(ce_renderwindow* renderwindow, WPARAM, LPARAM lparam)
    {
        renderwindow->input_context->pointer_position.x = GET_X_LPARAM(lparam);
        renderwindow->input_context->pointer_position.y = GET_Y_LPARAM(lparam);

        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        winwindow->cursor_inside = true;

        return false;
    }

    bool ce_renderwindow_handler_mouseleave(ce_renderwindow* renderwindow, WPARAM, LPARAM)
    {
        renderwindow->input_context->pointer_position = CE_VEC2_ZERO;

        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        winwindow->cursor_inside = false;

        return false;
    }

    bool ce_renderwindow_handler_mousemove(ce_renderwindow* renderwindow, WPARAM, LPARAM lparam)
    {
        renderwindow->input_context->pointer_offset.x = GET_X_LPARAM(lparam) - renderwindow->input_context->pointer_position.x;
        renderwindow->input_context->pointer_offset.y = GET_Y_LPARAM(lparam) - renderwindow->input_context->pointer_position.y;

        renderwindow->input_context->pointer_position.x = GET_X_LPARAM(lparam);
        renderwindow->input_context->pointer_position.y = GET_Y_LPARAM(lparam);

        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;

        if (!winwindow->cursor_inside) {
            TRACKMOUSEEVENT event = { sizeof(TRACKMOUSEEVENT), TME_HOVER | TME_LEAVE, winwindow->window, 1 };
            TrackMouseEvent(&event);
        }

        return false;
    }
}
