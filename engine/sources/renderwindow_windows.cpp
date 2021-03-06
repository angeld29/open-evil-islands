/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include "renderwindow.hpp"
#include "display_windows.hpp"
#include "graphicscontext_windows.hpp"
#include "exception.hpp"
#include "makeunique.hpp"

#include <windows.h>
#include <windowsx.h>

namespace cursedearth
{
    class ms_window_t final: public render_window_t
    {
    public:
        ms_window_t(const std::string& title, const input_context_ptr_t& input_context):
            render_window_t(title, input_context)
        {
            m_style[state_window] = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
            m_style[state_fullscreen] = WS_VISIBLE | WS_POPUP;

            m_extended_style[state_window] = WS_EX_APPWINDOW;
            m_extended_style[state_fullscreen] = WS_EX_TOOLWINDOW;

            for (int i = 0; i < state_count; ++i) {
                RECT rect = { 0, 0, m_geometry[i].width, m_geometry[i].height };
                if (AdjustWindowRectEx(&rect, m_style[i], FALSE, m_extended_style[i])) {
                    m_geometry[i].width = rect.right - rect.left;
                    m_geometry[i].height = rect.bottom - rect.top;
                }
            }

            m_geometry[state_window].x = (GetSystemMetrics(SM_CXSCREEN) - m_geometry[state_window].width) / 2;
            m_geometry[state_window].y = (GetSystemMetrics(SM_CYSCREEN) - m_geometry[state_window].height) / 2;

            m_input_map.insert({
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
                m_handlers[i] = &ms_window_t::skip_handler;
            }

            m_handlers[WM_CLOSE] = &ms_window_t::close_handler;
            m_handlers[WM_ENTERSIZEMOVE] = &ms_window_t::entersizemove_handler;
            m_handlers[WM_EXITSIZEMOVE] = &ms_window_t::exitsizemove_handler;
            m_handlers[WM_WINDOWPOSCHANGED] = &ms_window_t::windowposchanged_handler;
            m_handlers[WM_SIZE] = &ms_window_t::size_handler;
            m_handlers[WM_SYSCOMMAND] = &ms_window_t::syscommand_handler;
            m_handlers[WM_KILLFOCUS] = &ms_window_t::killfocus_handler;
            m_handlers[WM_KEYDOWN] = &ms_window_t::keydown_handler;
            m_handlers[WM_SYSKEYDOWN] = &ms_window_t::keydown_handler;
            m_handlers[WM_KEYUP] = &ms_window_t::keyup_handler;
            m_handlers[WM_SYSKEYUP] = &ms_window_t::keyup_handler;
            m_handlers[WM_LBUTTONDOWN] = &ms_window_t::lbuttondown_handler;
            m_handlers[WM_LBUTTONUP] = &ms_window_t::lbuttonup_handler;
            m_handlers[WM_MBUTTONDOWN] = &ms_window_t::mbuttondown_handler;
            m_handlers[WM_MBUTTONUP] = &ms_window_t::mbuttonup_handler;
            m_handlers[WM_RBUTTONDOWN] = &ms_window_t::rbuttondown_handler;
            m_handlers[WM_RBUTTONUP] = &ms_window_t::rbuttonup_handler;
            m_handlers[WM_MOUSEWHEEL] = &ms_window_t::mousewheel_handler;
            m_handlers[WM_MOUSEHOVER] = &ms_window_t::mousehover_handler;
            m_handlers[WM_MOUSELEAVE] = &ms_window_t::mouseleave_handler;
            m_handlers[WM_MOUSEMOVE] = &ms_window_t::mousemove_handler;

            m_display_manager = ce_displaymng_create();

            WNDCLASSEX wc;
            ZeroMemory(&wc, sizeof(WNDCLASSEX));

            wc.cbSize = sizeof(WNDCLASSEX);
            wc.style = CS_OWNDC;
            wc.lpfnWndProc = handler;
            wc.hInstance = GetModuleHandle(NULL);
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.lpszClassName = "cursedearth";

            if (!RegisterClassEx(&wc)) {
                throw game_error("render window", "could not register class");
            }

            m_window = CreateWindowEx(m_extended_style[m_state], wc.lpszClassName, title.c_str(), m_style[m_state],
                m_geometry[m_state].x, m_geometry[m_state].y, m_geometry[m_state].width, m_geometry[m_state].height,
                HWND_DESKTOP, NULL, wc.hInstance, NULL);

            if (NULL == m_window) {
                throw game_error("render window", "could not create window");
            }

            SetWindowLongPtr(m_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

            m_graphics_context = ce_graphics_context_new(GetDC(m_window));
            if (NULL == m_graphics_context) {
                throw game_error("render window", "could not create graphic context");
            }
        }

        virtual ~ms_window_t()
        {
            ce_graphics_context_del(m_graphics_context);
            ce_displaymng_del(m_display_manager);

            ReleaseDC(m_window, GetDC(m_window));
            DestroyWindow(m_window);

            UnregisterClass("cursedearth", GetModuleHandle(NULL));
        }

    private:
        virtual void do_show() final
        {
            ShowWindow(m_window, SW_SHOW);
            UpdateWindow(m_window);

            SetForegroundWindow(m_window);
            SetFocus(m_window);

            // WM_SIZE event is not coming, so force it
            RECT rect;
            if (GetClientRect(m_window, &rect)) {
                resized(rect.right, rect.bottom);
            }
        }

        virtual void do_minimize() final
        {
            ShowWindow(m_window, SW_MINIMIZE);
        }

        virtual void do_toggle_fullscreen() final
        {
            SetWindowLong(m_window, GWL_STYLE, m_style[m_state]);
            SetWindowLong(m_window, GWL_EXSTYLE, m_extended_style[m_state]);
            SetWindowPos(m_window, HWND_TOP, m_geometry[m_state].x, m_geometry[m_state].y, m_geometry[m_state].width, m_geometry[m_state].height, SWP_FRAMECHANGED);
        }

        virtual void do_pump() final
        {
            MSG message;
            while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }

        bool skip_handler(WPARAM, LPARAM)
        {
            return false;
        }

        bool close_handler(WPARAM, LPARAM)
        {
            closed();
            // suppress the WM_CLOSE message to prevent the OS from automatically destroying the window
            return true;
        }

        bool entersizemove_handler(WPARAM, LPARAM)
        {
            m_in_sizemove = true;
            return false;
        }

        bool exitsizemove_handler(WPARAM, LPARAM)
        {
            m_in_sizemove = false;
            return false;
        }

        bool windowposchanged_handler(WPARAM, LPARAM lparam)
        {
            if (m_in_sizemove) {
                WINDOWPOS* wp = reinterpret_cast<WINDOWPOS*>(lparam);
                if (!(SWP_NOMOVE & wp->flags)) {
                    m_geometry[m_state].x = wp->x;
                    m_geometry[m_state].y = wp->y;
                }
                if (!(SWP_NOSIZE & wp->flags)) {
                    m_geometry[m_state].width = wp->cx;
                    m_geometry[m_state].height = wp->cy;
                }
            }
            return false;
        }

        bool size_handler(WPARAM wparam, LPARAM lparam)
        {
            if (SIZE_MINIMIZED != wparam) {
                resized(LOWORD(lparam), HIWORD(lparam));
            }
            return false;
        }

        bool syscommand_handler(WPARAM wparam, LPARAM)
        {
            wparam &= 0xfff0;
            if (state_fullscreen == m_state && (SC_MONITORPOWER == wparam || SC_SCREENSAVE == wparam)) {
                // prevent monitor powersave mode or screensaver from starting
                return true;
            }
            if (SC_RESTORE == wparam) {
                // TODO: restore window
            }
            return false;
        }

        bool killfocus_handler(WPARAM, LPARAM)
        {
            m_input_context->clear();
            return false;
        }

        bool key_handler(WPARAM wparam, LPARAM lparam, bool pressed)
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
            m_input_context->buttons[static_cast<size_t>(m_input_map[wparam])] = pressed;
            return false;
        }

        bool keydown_handler(WPARAM wparam, LPARAM lparam)
        {
            return key_handler(wparam, lparam, true);
        }

        bool keyup_handler(WPARAM wparam, LPARAM lparam)
        {
            return key_handler(wparam, lparam, false);
        }

        bool button_handler(WPARAM, LPARAM lparam, input_button_t button, bool pressed)
        {
            // TODO: undefined behavior in window mode
            if (state_fullscreen == m_state) {
                m_input_context->pointer_position.x = GET_X_LPARAM(lparam);
                m_input_context->pointer_position.y = GET_Y_LPARAM(lparam);
            }
            m_input_context->buttons[static_cast<size_t>(button)] = pressed;
            return false;
        }

        bool lbuttondown_handler(WPARAM wparam, LPARAM lparam)
        {
            return button_handler(wparam, lparam, input_button_t::mb_left, true);
        }

        bool lbuttonup_handler(WPARAM wparam, LPARAM lparam)
        {
            return button_handler(wparam, lparam, input_button_t::mb_left, false);
        }

        bool mbuttondown_handler(WPARAM wparam, LPARAM lparam)
        {
            return button_handler(wparam, lparam, input_button_t::mb_middle, true);
        }

        bool mbuttonup_handler(WPARAM wparam, LPARAM lparam)
        {
            return button_handler(wparam, lparam, input_button_t::mb_middle, false);
        }

        bool rbuttondown_handler(WPARAM wparam, LPARAM lparam)
        {
            return button_handler(wparam, lparam, input_button_t::mb_right, true);
        }

        bool rbuttonup_handler(WPARAM wparam, LPARAM lparam)
        {
            return button_handler(wparam, lparam, input_button_t::mb_right, false);
        }

        bool mousewheel_handler(WPARAM wparam, LPARAM lparam)
        {
            if (m_cursor_inside) {
                return button_handler(wparam, lparam, (GET_WHEEL_DELTA_WPARAM(wparam) < 0 ?
                    input_button_t::mb_wheeldown : input_button_t::mb_wheelup), true);
            }
            return false;
        }

        bool mousehover_handler(WPARAM, LPARAM lparam)
        {
            m_input_context->pointer_position.x = GET_X_LPARAM(lparam);
            m_input_context->pointer_position.y = GET_Y_LPARAM(lparam);
            m_cursor_inside = true;
            return false;
        }

        bool mouseleave_handler(WPARAM, LPARAM)
        {
            m_input_context->pointer_position = CE_VEC2_ZERO;
            m_cursor_inside = false;
            return false;
        }

        bool mousemove_handler(WPARAM, LPARAM lparam)
        {
            m_input_context->pointer_offset.x = GET_X_LPARAM(lparam) - m_input_context->pointer_position.x;
            m_input_context->pointer_offset.y = GET_Y_LPARAM(lparam) - m_input_context->pointer_position.y;
            m_input_context->pointer_position.x = GET_X_LPARAM(lparam);
            m_input_context->pointer_position.y = GET_Y_LPARAM(lparam);
            if (!m_cursor_inside) {
                TRACKMOUSEEVENT event = { sizeof(TRACKMOUSEEVENT), TME_HOVER | TME_LEAVE, m_window, 1 };
                TrackMouseEvent(&event);
            }

            return false;
        }

        static LRESULT CALLBACK handler(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
        {
            if (message < WM_USER) {
                if (ms_window_t* ms_window = reinterpret_cast<ms_window_t*>(GetWindowLongPtr(window, GWLP_USERDATA))) {
                    if ((ms_window->*ms_window->m_handlers[message])(wparam, lparam)) {
                        return 0;
                    }
                }
            }
            return DefWindowProc(window, message, wparam, lparam);
        }

    private:
        DWORD m_style[state_count];
        DWORD m_extended_style[state_count];
        bool (ms_window_t::*m_handlers[WM_USER])(WPARAM, LPARAM);
        bool m_in_sizemove = false;
        bool m_cursor_inside = true;
        HWND m_window;
    };

    render_window_ptr_t make_render_window(const std::string& title, const input_context_ptr_t& input_context)
    {
        return make_unique<ms_window_t>(title, input_context);
    }
}
