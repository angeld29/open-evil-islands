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

#include <cassert>
#include <cstdio>

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
    struct ce_renderwindow_win
    {
        DWORD style[CE_RENDERWINDOW_STATE_COUNT];
        DWORD extended_style[CE_RENDERWINDOW_STATE_COUNT];
        bool (*handlers[WM_USER])(render_window_t*, WPARAM, LPARAM);
        bool in_sizemove;
        bool cursor_inside;
        HWND window;
    };

    LRESULT CALLBACK ce_renderwindow_proc(HWND, UINT, WPARAM, LPARAM);

    bool ce_renderwindow_handler_skip(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_close(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_entersizemove(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_exitsizemove(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_windowposchanged(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_size(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_syscommand(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_killfocus(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_keydown(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_keyup(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_lbuttondown(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_lbuttonup(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mbuttondown(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mbuttonup(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_rbuttondown(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_rbuttonup(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mousewheel(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mousehover(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mouseleave(render_window_t*, WPARAM, LPARAM);
    bool ce_renderwindow_handler_mousemove(render_window_t*, WPARAM, LPARAM);

    bool ce_renderwindow_win_ctor(render_window_t* renderwindow, va_list args)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        const char* title = va_arg(args, const char*);

        winwindow->style[CE_RENDERWINDOW_STATE_WINDOW] = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
        winwindow->style[CE_RENDERWINDOW_STATE_FULLSCREEN] = WS_VISIBLE | WS_POPUP;

        winwindow->extended_style[CE_RENDERWINDOW_STATE_WINDOW] = WS_EX_APPWINDOW;
        winwindow->extended_style[CE_RENDERWINDOW_STATE_FULLSCREEN] = WS_EX_TOOLWINDOW;

        for (int i = 0; i < CE_RENDERWINDOW_STATE_COUNT; ++i) {
            RECT rect = { 0, 0, renderwindow->geometry[i].width, renderwindow->geometry[i].height };
            if (AdjustWindowRectEx(&rect, winwindow->style[i], FALSE, winwindow->extended_style[i])) {
                renderwindow->geometry[i].width = rect.right - rect.left;
                renderwindow->geometry[i].height = rect.bottom - rect.top;
            }
        }

        renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].x = (GetSystemMetrics(SM_CXSCREEN) -
            renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].width) / 2;

        renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].y = (GetSystemMetrics(SM_CYSCREEN) -
            renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].height) / 2;

        renderwindow->displaymng = ce_displaymng_create();

        renderwindow->m_input_map.insert({
            { 0           , input_button_t::unknown        }, { VK_ESCAPE    , input_button_t::kb_escape     }, { VK_F1      , input_button_t::kb_f1         },
            { VK_F2       , input_button_t::kb_f2          }, { VK_F3        , input_button_t::kb_f3         }, { VK_F4      , input_button_t::kb_f4         },
            { VK_F5       , input_button_t::kb_f5          }, { VK_F6        , input_button_t::kb_f6         }, { VK_F7      , input_button_t::kb_f7         },
            { VK_F8       , input_button_t::kb_f8          }, { VK_F9        , input_button_t::kb_f9         }, { VK_F10     , input_button_t::kb_f10        },
            { VK_F11      , input_button_t::kb_f11         }, { VK_F12       , input_button_t::kb_f12        }, { VK_OEM_3   , input_button_t::kb_tilde      },
            { '0'         , input_button_t::kb_0           }, { '1'          , input_button_t::kb_1          }, { '2'        , input_button_t::kb_2          },
            { '3'         , input_button_t::kb_3           }, { '4'          , input_button_t::kb_4          }, { '5'        , input_button_t::kb_5          },
            { '6'         , input_button_t::kb_6           }, { '7'          , input_button_t::kb_7          }, { '8'        , input_button_t::kb_8          },
            { '9'         , input_button_t::kb_9           }, { VK_OEM_MINUS , input_button_t::kb_minus      }, { VK_OEM_PLUS, input_button_t::kb_equals     },
            { VK_OEM_5    , input_button_t::kb_backslash   }, { VK_BACK      , input_button_t::kb_backspace  }, { VK_TAB     , input_button_t::kb_tab        },
            { 'Q'         , input_button_t::kb_q           }, { 'W'          , input_button_t::kb_w          }, { 'E'        , input_button_t::kb_e          },
            { 'R'         , input_button_t::kb_r           }, { 'T'          , input_button_t::kb_t          }, { 'Y'        , input_button_t::kb_y          },
            { 'U'         , input_button_t::kb_u           }, { 'I'          , input_button_t::kb_i          }, { 'O'        , input_button_t::kb_o          },
            { 'P'         , input_button_t::kb_p           }, { VK_OEM_4     , input_button_t::kb_lbracket   }, { VK_OEM_6   , input_button_t::kb_rbracket   },
            { VK_CAPITAL  , input_button_t::kb_capslock    }, { 'A'          , input_button_t::kb_a          }, { 'S'        , input_button_t::kb_s          },
            { 'D'         , input_button_t::kb_d           }, { 'F'          , input_button_t::kb_f          }, { 'G'        , input_button_t::kb_g          },
            { 'H'         , input_button_t::kb_h           }, { 'J'          , input_button_t::kb_j          }, { 'K'        , input_button_t::kb_k          },
            { 'L'         , input_button_t::kb_l           }, { VK_OEM_1     , input_button_t::kb_semicolon  }, { VK_OEM_7   , input_button_t::kb_apostrophe },
            { VK_RETURN   , input_button_t::kb_enter       }, { VK_LSHIFT    , input_button_t::kb_lshift     }, { 'Z'        , input_button_t::kb_z          },
            { 'X'         , input_button_t::kb_x           }, { 'C'          , input_button_t::kb_c          }, { 'V'        , input_button_t::kb_v          },
            { 'B'         , input_button_t::kb_b           }, { 'N'          , input_button_t::kb_n          }, { 'M'        , input_button_t::kb_m          },
            { VK_OEM_COMMA, input_button_t::kb_comma       }, { VK_OEM_PERIOD, input_button_t::kb_period     }, { VK_OEM_2   , input_button_t::kb_slash      },
            { VK_RSHIFT   , input_button_t::kb_rshift      }, { VK_LCONTROL  , input_button_t::kb_lcontrol   }, { VK_LWIN    , input_button_t::kb_lmeta      },
            { VK_LMENU    , input_button_t::kb_lalt        }, { VK_SPACE     , input_button_t::kb_space      }, { VK_RMENU   , input_button_t::kb_ralt       },
            { VK_RWIN     , input_button_t::kb_rmeta       }, { VK_APPS      , input_button_t::kb_menu       }, { VK_RCONTROL, input_button_t::kb_rcontrol   },
            { VK_SNAPSHOT , input_button_t::kb_print       }, { VK_SCROLL    , input_button_t::kb_scrolllock }, { VK_PAUSE   , input_button_t::kb_pause      },
            { VK_INSERT   , input_button_t::kb_insert      }, { VK_DELETE    , input_button_t::kb_delete     }, { VK_HOME    , input_button_t::kb_home       },
            { VK_END      , input_button_t::kb_end         }, { VK_PRIOR     , input_button_t::kb_pageup     }, { VK_NEXT    , input_button_t::kb_pagedown   },
            { VK_LEFT     , input_button_t::kb_left        }, { VK_UP        , input_button_t::kb_up         }, { VK_RIGHT   , input_button_t::kb_right      },
            { VK_DOWN     , input_button_t::kb_down        }, { VK_NUMLOCK   , input_button_t::kb_numlock    }, { VK_DIVIDE  , input_button_t::kb_divide     },
            { VK_MULTIPLY , input_button_t::kb_multiply    }, { VK_SUBTRACT  , input_button_t::kb_subtract   }, { VK_ADD     , input_button_t::kb_add        },
            { VK_EXECUTE  , input_button_t::kb_numpadenter }, { VK_DECIMAL   , input_button_t::kb_decimal    }, { VK_NUMPAD7 , input_button_t::kb_numpad7    },
            { VK_NUMPAD8  , input_button_t::kb_numpad8     }, { VK_NUMPAD9   , input_button_t::kb_numpad9    }, { VK_NUMPAD4 , input_button_t::kb_numpad4    },
            { VK_NUMPAD5  , input_button_t::kb_numpad5     }, { VK_NUMPAD6   , input_button_t::kb_numpad6    }, { VK_NUMPAD1 , input_button_t::kb_numpad1    },
            { VK_NUMPAD2  , input_button_t::kb_numpad2     }, { VK_NUMPAD3   , input_button_t::kb_numpad3    }, { VK_NUMPAD0 , input_button_t::kb_numpad0    }
        });

        for (int i = 0; i < WM_USER; ++i) {
            winwindow->handlers[i] = ce_renderwindow_handler_skip;
        }

        winwindow->handlers[WM_CLOSE] = ce_renderwindow_handler_close;
        winwindow->handlers[WM_ENTERSIZEMOVE] = ce_renderwindow_handler_entersizemove;
        winwindow->handlers[WM_EXITSIZEMOVE] = ce_renderwindow_handler_exitsizemove;
        winwindow->handlers[WM_WINDOWPOSCHANGED] = ce_renderwindow_handler_windowposchanged;
        winwindow->handlers[WM_SIZE] = ce_renderwindow_handler_size;
        winwindow->handlers[WM_SYSCOMMAND] = ce_renderwindow_handler_syscommand;
        winwindow->handlers[WM_KILLFOCUS] = ce_renderwindow_handler_killfocus;
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
            ce_logging_fatal("render window: could not register class");
            return false;
        }

        winwindow->window = CreateWindowEx(winwindow->extended_style[renderwindow->state],
            wc.lpszClassName, title, winwindow->style[renderwindow->state],
            renderwindow->geometry[renderwindow->state].x, renderwindow->geometry[renderwindow->state].y,
            renderwindow->geometry[renderwindow->state].width, renderwindow->geometry[renderwindow->state].height,
            HWND_DESKTOP, NULL, wc.hInstance, NULL);

        if (NULL == winwindow->window) {
            ce_logging_fatal("render window: could not create window");
            return false;
        }

        SetWindowLongPtr(winwindow->window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(renderwindow));

        renderwindow->graphics_context = ce_graphics_context_new(GetDC(winwindow->window));
        if (NULL == renderwindow->graphics_context) {
            ce_logging_fatal("render window: could not create graphic context");
            return false;
        }

        return true;
    }

    void ce_renderwindow_win_dtor(render_window_t* renderwindow)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;

        ce_graphics_context_del(renderwindow->graphics_context);
        ce_displaymng_del(renderwindow->displaymng);

        if (NULL != winwindow->window) {
            ReleaseDC(winwindow->window, GetDC(winwindow->window));
            DestroyWindow(winwindow->window);
        }

        UnregisterClass("cursedearth", GetModuleHandle(NULL));
    }

    void ce_renderwindow_win_show(render_window_t* renderwindow)
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

    void ce_renderwindow_win_minimize(render_window_t* renderwindow)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        ShowWindow(winwindow->window, SW_MINIMIZE);
    }

    void ce_renderwindow_win_toggle_fullscreen(render_window_t* renderwindow)
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

    void ce_renderwindow_win_pump(render_window_t*)
    {
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    render_window_t* ce_renderwindow_create(int width, int height, const char* title)
    {
        ce_renderwindow_vtable vt = {ce_renderwindow_win_ctor, ce_renderwindow_win_dtor, ce_renderwindow_win_show,
            ce_renderwindow_win_minimize, ce_renderwindow_win_toggle_fullscreen, ce_renderwindow_win_pump};
        return ce_renderwindow_new(vt, sizeof(ce_renderwindow_win), width, height, title);
    }

    LRESULT CALLBACK ce_renderwindow_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
    {
        render_window_t* renderwindow = reinterpret_cast<render_window_t*>(GetWindowLongPtr(window, GWLP_USERDATA));
        if (NULL != renderwindow && message < WM_USER) {
            ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
            if ((*winwindow->handlers[message])(renderwindow, wparam, lparam)) {
                return 0;
            }
        }
        return DefWindowProc(window, message, wparam, lparam);
    }

    bool ce_renderwindow_handler_skip(render_window_t*, WPARAM, LPARAM)
    {
        return false;
    }

    bool ce_renderwindow_handler_close(render_window_t* renderwindow, WPARAM, LPARAM)
    {
        ce_renderwindow_emit_closed(renderwindow);
        // suppress the WM_CLOSE message to prevent the OS from automatically destroying the window
        return true;
    }

    bool ce_renderwindow_handler_entersizemove(render_window_t* renderwindow, WPARAM, LPARAM)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        winwindow->in_sizemove = true;
        return false;
    }

    bool ce_renderwindow_handler_exitsizemove(render_window_t* renderwindow, WPARAM, LPARAM)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        winwindow->in_sizemove = false;
        return false;
    }

    bool ce_renderwindow_handler_windowposchanged(render_window_t* renderwindow, WPARAM, LPARAM lparam)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        if (winwindow->in_sizemove) {
            WINDOWPOS* wp = reinterpret_cast<WINDOWPOS*>(lparam);

            if (!(SWP_NOMOVE & wp->flags)) {
                renderwindow->geometry[renderwindow->state].x = wp->x;
                renderwindow->geometry[renderwindow->state].y = wp->y;
            }

            if (!(SWP_NOSIZE & wp->flags)) {
                renderwindow->geometry[renderwindow->state].width = wp->cx;
                renderwindow->geometry[renderwindow->state].height = wp->cy;
            }
        }
        return false;
    }

    bool ce_renderwindow_handler_size(render_window_t* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        if (SIZE_MINIMIZED != wparam) {
            ce_renderwindow_emit_resized(renderwindow, LOWORD(lparam), HIWORD(lparam));
        }
        return false;
    }

    bool ce_renderwindow_handler_syscommand(render_window_t* renderwindow, WPARAM wparam, LPARAM)
    {
        wparam &= 0xfff0;

        if (CE_RENDERWINDOW_STATE_FULLSCREEN == renderwindow->state && (SC_MONITORPOWER == wparam || SC_SCREENSAVE == wparam)) {
            // prevent monitor powersave mode or screensaver from starting
            return true;
        }

        if (SC_RESTORE == wparam) {
            // TODO: restore window
        }

        return false;
    }

    bool ce_renderwindow_handler_killfocus(render_window_t* renderwindow, WPARAM, LPARAM)
    {
        renderwindow->m_input_context->clear();
        return false;
    }

    bool ce_renderwindow_handler_key(render_window_t* renderwindow, WPARAM wparam, LPARAM lparam, bool pressed)
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
        renderwindow->m_input_context->buttons[static_cast<size_t>(renderwindow->m_input_map[wparam])] = pressed;
        return false;
    }

    bool ce_renderwindow_handler_keydown(render_window_t* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_key(renderwindow, wparam, lparam, true);
    }

    bool ce_renderwindow_handler_keyup(render_window_t* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_key(renderwindow, wparam, lparam, false);
    }

    bool ce_renderwindow_handler_button(render_window_t* renderwindow, WPARAM, LPARAM lparam, input_button_t button, bool pressed)
    {
        // TODO: undefined behavior in window mode
        if (CE_RENDERWINDOW_STATE_FULLSCREEN == renderwindow->state) {
            renderwindow->m_input_context->pointer_position.x = GET_X_LPARAM(lparam);
            renderwindow->m_input_context->pointer_position.y = GET_Y_LPARAM(lparam);
        }
        renderwindow->m_input_context->buttons[static_cast<size_t>(button)] = pressed;
        return false;
    }

    bool ce_renderwindow_handler_lbuttondown(render_window_t* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, input_button_t::mb_left, true);
    }

    bool ce_renderwindow_handler_lbuttonup(render_window_t* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, input_button_t::mb_left, false);
    }

    bool ce_renderwindow_handler_mbuttondown(render_window_t* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, input_button_t::mb_middle, true);
    }

    bool ce_renderwindow_handler_mbuttonup(render_window_t* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, input_button_t::mb_middle, false);
    }

    bool ce_renderwindow_handler_rbuttondown(render_window_t* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, input_button_t::mb_right, true);
    }

    bool ce_renderwindow_handler_rbuttonup(render_window_t* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        return ce_renderwindow_handler_button(renderwindow, wparam, lparam, input_button_t::mb_right, false);
    }

    bool ce_renderwindow_handler_mousewheel(render_window_t* renderwindow, WPARAM wparam, LPARAM lparam)
    {
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        if (winwindow->cursor_inside) {
            return ce_renderwindow_handler_button(renderwindow, wparam, lparam,
                (GET_WHEEL_DELTA_WPARAM(wparam) < 0 ? input_button_t::mb_wheeldown : input_button_t::mb_wheelup), true);
        }
        return false;
    }

    bool ce_renderwindow_handler_mousehover(render_window_t* renderwindow, WPARAM, LPARAM lparam)
    {
        renderwindow->m_input_context->pointer_position.x = GET_X_LPARAM(lparam);
        renderwindow->m_input_context->pointer_position.y = GET_Y_LPARAM(lparam);
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        winwindow->cursor_inside = true;
        return false;
    }

    bool ce_renderwindow_handler_mouseleave(render_window_t* renderwindow, WPARAM, LPARAM)
    {
        renderwindow->m_input_context->pointer_position = CE_VEC2_ZERO;
        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;
        winwindow->cursor_inside = false;
        return false;
    }

    bool ce_renderwindow_handler_mousemove(render_window_t* renderwindow, WPARAM, LPARAM lparam)
    {
        renderwindow->m_input_context->pointer_offset.x = GET_X_LPARAM(lparam) - renderwindow->m_input_context->pointer_position.x;
        renderwindow->m_input_context->pointer_offset.y = GET_Y_LPARAM(lparam) - renderwindow->m_input_context->pointer_position.y;

        renderwindow->m_input_context->pointer_position.x = GET_X_LPARAM(lparam);
        renderwindow->m_input_context->pointer_position.y = GET_Y_LPARAM(lparam);

        ce_renderwindow_win* winwindow = (ce_renderwindow_win*)renderwindow->impl;

        if (!winwindow->cursor_inside) {
            TRACKMOUSEEVENT event = { sizeof(TRACKMOUSEEVENT), TME_HOVER | TME_LEAVE, winwindow->window, 1 };
            TrackMouseEvent(&event);
        }

        return false;
    }
}
