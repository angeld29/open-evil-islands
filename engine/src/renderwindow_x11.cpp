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
#include <cstring>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "alloc.hpp"
#include "logging.hpp"
#include "display_x11.hpp"
#include "graphicscontext_x11.hpp"
#include "renderwindow.hpp"

namespace cursedearth
{
    enum {
        CE_RENDERWINDOW_ATOM_WM_PROTOCOLS,
        CE_RENDERWINDOW_ATOM_WM_DELETE_WINDOW,
        CE_RENDERWINDOW_ATOM_NET_WM_STATE_FULLSCREEN,
        CE_RENDERWINDOW_ATOM_NET_WM_STATE,
        CE_RENDERWINDOW_ATOM_COUNT
    };

    struct ce_renderwindow_x11
    {
        Atom atoms[CE_RENDERWINDOW_ATOM_COUNT];
        unsigned long mask[CE_RENDERWINDOW_STATE_COUNT];
        XSetWindowAttributes attrs[CE_RENDERWINDOW_STATE_COUNT];
        void (*handlers[LASTEvent])(ce_renderwindow*, XEvent*);
        bool autorepeat; // remember old auto repeat settings
        struct {
            int timeout, interval;
            int prefer_blanking;
            int allow_exposures;
        } screensaver; // remember old screen saver settings
        Display* display;
        Window window;
    };

    void ce_renderwindow_handler_skip(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_client_message(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_map_notify(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_visibility_notify(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_configure_notify(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_focus_in(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_focus_out(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_enter_notify(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_key_press(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_key_release(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_button_press(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_button_release(ce_renderwindow*, XEvent*);
    void ce_renderwindow_handler_motion_notify(ce_renderwindow*, XEvent*);

    bool ce_renderwindow_x11_ctor(ce_renderwindow* renderwindow, va_list args)
    {
        ce_renderwindow_x11* x11window = (ce_renderwindow_x11*)renderwindow->impl;
        const char* title = va_arg(args, const char*);

        XInitThreads();

        x11window->display = XOpenDisplay(NULL);
        if (NULL == x11window->display) {
            ce_logging_fatal("render window: could not connect to X server");
            return false;
        }

        ce_logging_info("render window: using X server %d.%d", XProtocolVersion(x11window->display), XProtocolRevision(x11window->display));
        ce_logging_info("render window: %s %d", XServerVendor(x11window->display), XVendorRelease(x11window->display));

        renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].x =
            (XDisplayWidth(x11window->display, XDefaultScreen(x11window->display)) -
            renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].width) / 2;

        renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].y =
            (XDisplayHeight(x11window->display, XDefaultScreen(x11window->display)) -
            renderwindow->geometry[CE_RENDERWINDOW_STATE_WINDOW].height) / 2;

        if (0 == renderwindow->visual.bpp) {
            renderwindow->visual.bpp = XDefaultDepth(x11window->display, XDefaultScreen(x11window->display));
            if (1 == renderwindow->visual.bpp) {
                ce_logging_warning("render window: you live in prehistoric times");
            }
        }

        renderwindow->displaymng = ce_displaymng_create(x11window->display);
        renderwindow->graphics_context = ce_graphics_context_new(x11window->display);

        // absolutely don't understand how XChangeKeyboardMapping works...
        renderwindow->m_input_map.insert({
            { XK_VoidSymbol , input_button_t::unknown        }, { XK_Escape      , input_button_t::kb_escape     }, { XK_F1          , input_button_t::kb_f1         },
            { XK_F2         , input_button_t::kb_f2          }, { XK_F3          , input_button_t::kb_f3         }, { XK_F4          , input_button_t::kb_f4         },
            { XK_F5         , input_button_t::kb_f5          }, { XK_F6          , input_button_t::kb_f6         }, { XK_F7          , input_button_t::kb_f7         },
            { XK_F8         , input_button_t::kb_f8          }, { XK_F9          , input_button_t::kb_f9         }, { XK_F10         , input_button_t::kb_f10        },
            { XK_F11        , input_button_t::kb_f11         }, { XK_F12         , input_button_t::kb_f12        }, { XK_grave       , input_button_t::kb_tilde      },
            { XK_0          , input_button_t::kb_0           }, { XK_1           , input_button_t::kb_1          }, { XK_2           , input_button_t::kb_2          },
            { XK_3          , input_button_t::kb_3           }, { XK_4           , input_button_t::kb_4          }, { XK_5           , input_button_t::kb_5          },
            { XK_6          , input_button_t::kb_6           }, { XK_7           , input_button_t::kb_7          }, { XK_8           , input_button_t::kb_8          },
            { XK_9          , input_button_t::kb_9           }, { XK_minus       , input_button_t::kb_minus      }, { XK_equal       , input_button_t::kb_equals     },
            { XK_backslash  , input_button_t::kb_backslash   }, { XK_BackSpace   , input_button_t::kb_backspace  }, { XK_Tab         , input_button_t::kb_tab        },
            { XK_q          , input_button_t::kb_q           }, { XK_w           , input_button_t::kb_w          }, { XK_e           , input_button_t::kb_e          },
            { XK_r          , input_button_t::kb_r           }, { XK_t           , input_button_t::kb_t          }, { XK_y           , input_button_t::kb_y          },
            { XK_u          , input_button_t::kb_u           }, { XK_i           , input_button_t::kb_i          }, { XK_o           , input_button_t::kb_o          },
            { XK_p          , input_button_t::kb_p           }, { XK_bracketleft , input_button_t::kb_lbracket   }, { XK_bracketright, input_button_t::kb_rbracket   },
            { XK_Caps_Lock  , input_button_t::kb_capslock    }, { XK_a           , input_button_t::kb_a          }, { XK_s           , input_button_t::kb_s          },
            { XK_d          , input_button_t::kb_d           }, { XK_f           , input_button_t::kb_f          }, { XK_g           , input_button_t::kb_g          },
            { XK_h          , input_button_t::kb_h           }, { XK_j           , input_button_t::kb_j          }, { XK_k           , input_button_t::kb_k          },
            { XK_l          , input_button_t::kb_l           }, { XK_semicolon   , input_button_t::kb_semicolon  }, { XK_apostrophe  , input_button_t::kb_apostrophe },
            { XK_Return     , input_button_t::kb_enter       }, { XK_Shift_L     , input_button_t::kb_lshift     }, { XK_z           , input_button_t::kb_z          },
            { XK_x          , input_button_t::kb_x           }, { XK_c           , input_button_t::kb_c          }, { XK_v           , input_button_t::kb_v          },
            { XK_b          , input_button_t::kb_b           }, { XK_n           , input_button_t::kb_n          }, { XK_m           , input_button_t::kb_m          },
            { XK_comma      , input_button_t::kb_comma       }, { XK_period      , input_button_t::kb_period     }, { XK_slash       , input_button_t::kb_slash      },
            { XK_Shift_R    , input_button_t::kb_rshift      }, { XK_Control_L   , input_button_t::kb_lcontrol   }, { XK_Super_L     , input_button_t::kb_lmeta      },
            { XK_Alt_L      , input_button_t::kb_lalt        }, { XK_space       , input_button_t::kb_space      }, { XK_Alt_R       , input_button_t::kb_ralt       },
            { XK_Super_R    , input_button_t::kb_rmeta       }, { XK_Menu        , input_button_t::kb_menu       }, { XK_Control_R   , input_button_t::kb_rcontrol   },
            { XK_Print      , input_button_t::kb_print       }, { XK_Scroll_Lock , input_button_t::kb_scrolllock }, { XK_Pause       , input_button_t::kb_pause      },
            { XK_Insert     , input_button_t::kb_insert      }, { XK_Delete      , input_button_t::kb_delete     }, { XK_Home        , input_button_t::kb_home       },
            { XK_End        , input_button_t::kb_end         }, { XK_Page_Up     , input_button_t::kb_pageup     }, { XK_Page_Down   , input_button_t::kb_pagedown   },
            { XK_Left       , input_button_t::kb_left        }, { XK_Up          , input_button_t::kb_up         }, { XK_Right       , input_button_t::kb_right      },
            { XK_Down       , input_button_t::kb_down        }, { XK_Num_Lock    , input_button_t::kb_numlock    }, { XK_KP_Divide   , input_button_t::kb_divide     },
            { XK_KP_Multiply, input_button_t::kb_multiply    }, { XK_KP_Subtract , input_button_t::kb_subtract   }, { XK_KP_Add      , input_button_t::kb_add        },
            { XK_KP_Enter   , input_button_t::kb_numpadenter }, { XK_KP_Delete   , input_button_t::kb_decimal    }, { XK_KP_Home     , input_button_t::kb_numpad7    },
            { XK_KP_Up      , input_button_t::kb_numpad8     }, { XK_KP_Page_Up  , input_button_t::kb_numpad9    }, { XK_KP_Left     , input_button_t::kb_numpad4    },
            { XK_KP_Begin   , input_button_t::kb_numpad5     }, { XK_KP_Right    , input_button_t::kb_numpad6    }, { XK_KP_End      , input_button_t::kb_numpad1    },
            { XK_KP_Down    , input_button_t::kb_numpad2     }, { XK_KP_Page_Down, input_button_t::kb_numpad3    }, { XK_KP_Insert   , input_button_t::kb_numpad0    }
        });

        x11window->atoms[CE_RENDERWINDOW_ATOM_WM_PROTOCOLS] = XInternAtom(x11window->display, "WM_PROTOCOLS", False);
        x11window->atoms[CE_RENDERWINDOW_ATOM_WM_DELETE_WINDOW] = XInternAtom(x11window->display, "WM_DELETE_WINDOW", False);
        x11window->atoms[CE_RENDERWINDOW_ATOM_NET_WM_STATE_FULLSCREEN] = XInternAtom(x11window->display, "_NET_WM_STATE_FULLSCREEN", False);
        x11window->atoms[CE_RENDERWINDOW_ATOM_NET_WM_STATE] = XInternAtom(x11window->display, "_NET_WM_STATE", False);

        for (int i = 0; i < CE_RENDERWINDOW_STATE_COUNT; ++i) {
            x11window->mask[i] = CWColormap | CWEventMask | CWOverrideRedirect;
            x11window->attrs[i].colormap = XCreateColormap(x11window->display, XDefaultRootWindow(x11window->display), renderwindow->graphics_context->visual_info->visual, AllocNone);
            x11window->attrs[i].event_mask = EnterWindowMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
                PointerMotionMask | ButtonMotionMask | FocusChangeMask | VisibilityChangeMask | StructureNotifyMask;
            x11window->attrs[i].override_redirect = (CE_RENDERWINDOW_STATE_FULLSCREEN == i);
        }

        for (int i = 0; i < LASTEvent; ++i) {
            x11window->handlers[i] = ce_renderwindow_handler_skip;
        }

        x11window->handlers[ClientMessage] = ce_renderwindow_handler_client_message;
        x11window->handlers[MapNotify] = ce_renderwindow_handler_map_notify;
        x11window->handlers[VisibilityNotify] = ce_renderwindow_handler_visibility_notify;
        x11window->handlers[ConfigureNotify] = ce_renderwindow_handler_configure_notify;
        x11window->handlers[FocusIn] = ce_renderwindow_handler_focus_in;
        x11window->handlers[FocusOut] = ce_renderwindow_handler_focus_out;
        x11window->handlers[EnterNotify] = ce_renderwindow_handler_enter_notify;
        x11window->handlers[KeyPress] = ce_renderwindow_handler_key_press;
        x11window->handlers[KeyRelease] = ce_renderwindow_handler_key_release;
        x11window->handlers[ButtonPress] = ce_renderwindow_handler_button_press;
        x11window->handlers[ButtonRelease] = ce_renderwindow_handler_button_release;
        x11window->handlers[MotionNotify] = ce_renderwindow_handler_motion_notify;

        XKeyboardState kbdstate;
        XGetKeyboardControl(x11window->display, &kbdstate);

        x11window->autorepeat = (AutoRepeatModeOn == kbdstate.global_auto_repeat);

        XGetScreenSaver(x11window->display, &x11window->screensaver.timeout, &x11window->screensaver.interval, &x11window->screensaver.prefer_blanking, &x11window->screensaver.allow_exposures);

        x11window->window = XCreateWindow(x11window->display,
            XDefaultRootWindow(x11window->display), 0, 0,
            renderwindow->geometry[renderwindow->state].width,
            renderwindow->geometry[renderwindow->state].height,
            0, renderwindow->graphics_context->visual_info->depth,
            InputOutput, renderwindow->graphics_context->visual_info->visual,
            x11window->mask[renderwindow->state],
            &x11window->attrs[renderwindow->state]);

        if (!ce_graphics_context_make_current(renderwindow->graphics_context, x11window->display, x11window->window)) {
            ce_logging_fatal("render window: could not set graphic context");
            return false;
        }

        XSizeHints* size_hints = XAllocSizeHints();
        size_hints->flags = PSize | PMinSize;
        size_hints->min_width = 400;
        size_hints->min_height = 300;
        size_hints->base_width = renderwindow->geometry[renderwindow->state].width;
        size_hints->base_height = renderwindow->geometry[renderwindow->state].height;
        XSetWMNormalHints(x11window->display, x11window->window, size_hints);
        XFree(size_hints);

        XSetStandardProperties(x11window->display, x11window->window, title, title, None, NULL, 0, NULL);

        // handle wm_delete_events
        XSetWMProtocols(x11window->display, x11window->window, &x11window->atoms[CE_RENDERWINDOW_ATOM_WM_DELETE_WINDOW], 1);

        return true;
    }

    void ce_renderwindow_x11_dtor(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_x11* x11window = (ce_renderwindow_x11*)renderwindow->impl;

        ce_graphics_context_del(renderwindow->graphics_context);
        ce_displaymng_del(renderwindow->displaymng);

        if (0 != x11window->window) {
            XDestroyWindow(x11window->display, x11window->window);
        }

        if (NULL != x11window->display) {
            XSetScreenSaver(x11window->display, x11window->screensaver.timeout, x11window->screensaver.interval, x11window->screensaver.prefer_blanking, x11window->screensaver.allow_exposures);
            if (x11window->autorepeat) {
                XAutoRepeatOn(x11window->display);
            }
            XCloseDisplay(x11window->display);
        }
    }

    void ce_renderwindow_x11_show(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_x11* x11window = (ce_renderwindow_x11*)renderwindow->impl;
        XMapRaised(x11window->display, x11window->window);

        // x and y values in XCreateWindow are ignored by most windows managers,
        // which means that top-level windows maybe placed somewhere else on the desktop
        XMoveWindow(x11window->display, x11window->window, renderwindow->geometry[renderwindow->state].x, renderwindow->geometry[renderwindow->state].y);
    }

    void ce_renderwindow_x11_minimize(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_x11* x11window = (ce_renderwindow_x11*)renderwindow->impl;
        XIconifyWindow(x11window->display, x11window->window, XDefaultScreen(x11window->display));
    }

    void ce_renderwindow_x11_fullscreen_before_enter(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_x11* x11window = (ce_renderwindow_x11*)renderwindow->impl;
        XGrabPointer(x11window->display, x11window->window, True, NoEventMask, GrabModeAsync, GrabModeAsync, x11window->window, None, CurrentTime);
        XGrabKeyboard(x11window->display, x11window->window, True, GrabModeAsync, GrabModeAsync, CurrentTime);
        XSetScreenSaver(x11window->display, DisableScreenSaver, DisableScreenInterval, DontPreferBlanking, DefaultExposures);
    }

    void ce_renderwindow_x11_fullscreen_after_exit(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_x11* x11window = (ce_renderwindow_x11*)renderwindow->impl;
        XUngrabPointer(x11window->display, CurrentTime);
        XUngrabKeyboard(x11window->display, CurrentTime);
        XSetScreenSaver(x11window->display, x11window->screensaver.timeout, x11window->screensaver.interval, x11window->screensaver.prefer_blanking, x11window->screensaver.allow_exposures);
    }

    void ce_renderwindow_x11_fullscreen_done(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_x11* x11window = (ce_renderwindow_x11*)renderwindow->impl;
        XChangeWindowAttributes(x11window->display, x11window->window, x11window->mask[renderwindow->state], &x11window->attrs[renderwindow->state]);

        XEvent event;
        memset(&event, 0, sizeof(event));

        event.xclient.type = ClientMessage;
        event.xclient.send_event = True;
        event.xclient.window = x11window->window;
        event.xclient.message_type = x11window->atoms[CE_RENDERWINDOW_ATOM_NET_WM_STATE];
        event.xclient.format = 32;
        event.xclient.data.l[0] = renderwindow->state;
        event.xclient.data.l[1] = x11window->atoms[CE_RENDERWINDOW_ATOM_NET_WM_STATE_FULLSCREEN];

        // absolutely don't understand how event masks work...
        XSendEvent(x11window->display, XDefaultRootWindow(x11window->display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    }

    void ce_renderwindow_x11_pump(ce_renderwindow* renderwindow)
    {
        ce_renderwindow_x11* x11window = (ce_renderwindow_x11*)renderwindow->impl;

        XEvent event;
        while (XPending(x11window->display) > 0) {
            XNextEvent(x11window->display, &event);

            // is it possible? may be new version of the X server...
            assert(0 <= event.type && event.type < LASTEvent);

            // just in case
            if (event.type < LASTEvent) {
                (*x11window->handlers[event.type])(renderwindow, &event);
            }
        }
    }

    ce_renderwindow* ce_renderwindow_create(int width, int height, const char* title)
    {
        ce_renderwindow_vtable vt = {ce_renderwindow_x11_ctor, ce_renderwindow_x11_dtor, ce_renderwindow_x11_show, ce_renderwindow_x11_minimize,
            {NULL,ce_renderwindow_x11_fullscreen_before_enter,NULL,NULL, ce_renderwindow_x11_fullscreen_after_exit, ce_renderwindow_x11_fullscreen_done}, ce_renderwindow_x11_pump};
        return ce_renderwindow_new(vt, sizeof(ce_renderwindow_x11), width, height, title);
    }

    void ce_renderwindow_handler_skip(ce_renderwindow*, XEvent*)
    {
    }

    void ce_renderwindow_handler_client_message(ce_renderwindow* renderwindow, XEvent* event)
    {
        ce_renderwindow_x11* x11window = (ce_renderwindow_x11*)renderwindow->impl;

        if (x11window->atoms[CE_RENDERWINDOW_ATOM_WM_PROTOCOLS] == event->xclient.message_type &&
                x11window->atoms[CE_RENDERWINDOW_ATOM_WM_DELETE_WINDOW] == (Atom)event->xclient.data.l[0]) {
            ce_renderwindow_emit_closed(renderwindow);
        }
    }

    void ce_renderwindow_handler_map_notify(ce_renderwindow* renderwindow, XEvent*)
    {
        assert(CE_RENDERWINDOW_ACTION_NONE == renderwindow->action);
        renderwindow->action = CE_RENDERWINDOW_ACTION_RESTORED;
    }

    void ce_renderwindow_handler_visibility_notify(ce_renderwindow*, XEvent*)
    {
    }

    void ce_renderwindow_handler_configure_notify(ce_renderwindow* renderwindow, XEvent* event)
    {
        renderwindow->geometry[renderwindow->state].x = event->xconfigure.x;
        renderwindow->geometry[renderwindow->state].y = event->xconfigure.y;
        renderwindow->geometry[renderwindow->state].width = event->xconfigure.width;
        renderwindow->geometry[renderwindow->state].height = event->xconfigure.height;

        ce_renderwindow_emit_resized(renderwindow, event->xconfigure.width, event->xconfigure.height);
    }

    void ce_renderwindow_handler_focus_in(ce_renderwindow*, XEvent* event)
    {
        XAutoRepeatOff(event->xfocus.display);
    }

    void ce_renderwindow_handler_focus_out(ce_renderwindow* renderwindow, XEvent* event)
    {
        ce_renderwindow_x11* x11window = (ce_renderwindow_x11*)renderwindow->impl;

        if (x11window->autorepeat) {
            XAutoRepeatOn(event->xfocus.display);
        }

        renderwindow->m_input_context->clear();
    }

    void ce_renderwindow_handler_enter_notify(ce_renderwindow* renderwindow, XEvent* event)
    {
        renderwindow->m_input_context->pointer_position.x = event->xcrossing.x;
        renderwindow->m_input_context->pointer_position.y = event->xcrossing.y;
    }

    void ce_renderwindow_handler_key(ce_renderwindow* renderwindow, XEvent* event, bool pressed)
    {
        // reset modifier keys to disable uppercase keys
        event->xkey.state = 0;

        KeySym key;
        XLookupString(&event->xkey, NULL, 0, &key, NULL);

        renderwindow->m_input_context->buttons[static_cast<size_t>(renderwindow->m_input_map[key])] = pressed;
        renderwindow->m_input_context->pointer_position.x = event->xkey.x;
        renderwindow->m_input_context->pointer_position.y = event->xkey.y;
    }

    void ce_renderwindow_handler_key_press(ce_renderwindow* renderwindow, XEvent* event)
    {
        ce_renderwindow_handler_key(renderwindow, event, true);
    }

    void ce_renderwindow_handler_key_release(ce_renderwindow* renderwindow, XEvent* event)
    {
        ce_renderwindow_handler_key(renderwindow, event, false);
    }

    void ce_renderwindow_handler_button(ce_renderwindow* renderwindow, XEvent* event, bool pressed)
    {
        renderwindow->m_input_context->buttons[event->xbutton.button - 1 + static_cast<size_t>(input_button_t::mb_left)] = pressed;
        renderwindow->m_input_context->pointer_position.x = event->xbutton.x;
        renderwindow->m_input_context->pointer_position.y = event->xbutton.y;
    }

    void ce_renderwindow_handler_button_press(ce_renderwindow* renderwindow, XEvent* event)
    {
        ce_renderwindow_handler_button(renderwindow, event, true);
    }

    void ce_renderwindow_handler_button_release(ce_renderwindow* renderwindow, XEvent* event)
    {
        // special case: ignore wheel buttons, see pump method
        // ButtonPress event is immediately followed by ButtonRelease event
        if (Button4 != event->xbutton.button && Button5 != event->xbutton.button) {
            ce_renderwindow_handler_button(renderwindow, event, false);
        }
    }

    void ce_renderwindow_handler_motion_notify(ce_renderwindow* renderwindow, XEvent* event)
    {
        renderwindow->m_input_context->pointer_offset.x = event->xmotion.x - renderwindow->m_input_context->pointer_position.x;
        renderwindow->m_input_context->pointer_offset.y = event->xmotion.y - renderwindow->m_input_context->pointer_position.y;
        renderwindow->m_input_context->pointer_position.x = event->xmotion.x;
        renderwindow->m_input_context->pointer_position.y = event->xmotion.y;
    }
}
