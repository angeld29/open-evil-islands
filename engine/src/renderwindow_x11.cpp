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
#include <cstring>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "exception.hpp"
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

    class x11_window_t final: public render_window_t
    {
    public:
        explicit x11_window_t(const std::string& title):
            render_window_t(title)
        {
            XInitThreads();

            display = XOpenDisplay(NULL);
            if (NULL == display) {
                throw game_error("render window", "could not connect to X server");
            }

            ce_logging_info("render window: using X server %d.%d", XProtocolVersion(display), XProtocolRevision(display));
            ce_logging_info("render window: %s %d", XServerVendor(display), XVendorRelease(display));

            geometry[CE_RENDERWINDOW_STATE_WINDOW].x = (XDisplayWidth(display, XDefaultScreen(display)) - geometry[CE_RENDERWINDOW_STATE_WINDOW].width) / 2;
            geometry[CE_RENDERWINDOW_STATE_WINDOW].y = (XDisplayHeight(display, XDefaultScreen(display)) - geometry[CE_RENDERWINDOW_STATE_WINDOW].height) / 2;

            if (0 == visual.bpp) {
                visual.bpp = XDefaultDepth(display, XDefaultScreen(display));
                if (1 == visual.bpp) {
                    ce_logging_warning("render window: you live in prehistoric times");
                }
            }

            displaymng = ce_displaymng_create(display);
            graphics_context = ce_graphics_context_new(display);

            // absolutely don't understand how XChangeKeyboardMapping works...
            m_input_map.insert({
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

            atoms[CE_RENDERWINDOW_ATOM_WM_PROTOCOLS] = XInternAtom(display, "WM_PROTOCOLS", False);
            atoms[CE_RENDERWINDOW_ATOM_WM_DELETE_WINDOW] = XInternAtom(display, "WM_DELETE_WINDOW", False);
            atoms[CE_RENDERWINDOW_ATOM_NET_WM_STATE_FULLSCREEN] = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
            atoms[CE_RENDERWINDOW_ATOM_NET_WM_STATE] = XInternAtom(display, "_NET_WM_STATE", False);

            for (int i = 0; i < CE_RENDERWINDOW_STATE_COUNT; ++i) {
                mask[i] = CWColormap | CWEventMask | CWOverrideRedirect;
                attrs[i].colormap = XCreateColormap(display, XDefaultRootWindow(display), graphics_context->visual_info->visual, AllocNone);
                attrs[i].event_mask = EnterWindowMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
                    PointerMotionMask | ButtonMotionMask | FocusChangeMask | VisibilityChangeMask | StructureNotifyMask;
                attrs[i].override_redirect = (CE_RENDERWINDOW_STATE_FULLSCREEN == i);
            }

            for (int i = 0; i < LASTEvent; ++i) {
                handlers[i] = &x11_window_t::skip_handler;
            }

            handlers[ClientMessage] = &x11_window_t::client_message_handler;
            handlers[MapNotify] = &x11_window_t::map_notify_handler;
            handlers[VisibilityNotify] = &x11_window_t::visibility_notify_handler;
            handlers[ConfigureNotify] = &x11_window_t::configure_notify_handler;
            handlers[FocusIn] = &x11_window_t::focus_in_handler;
            handlers[FocusOut] = &x11_window_t::focus_out_handler;
            handlers[EnterNotify] = &x11_window_t::enter_notify_handler;
            handlers[KeyPress] = &x11_window_t::key_press_handler;
            handlers[KeyRelease] = &x11_window_t::key_release_handler;
            handlers[ButtonPress] = &x11_window_t::button_press_handler;
            handlers[ButtonRelease] = &x11_window_t::button_release_handler;
            handlers[MotionNotify] = &x11_window_t::motion_notify_handler;

            XKeyboardState kbdstate;
            XGetKeyboardControl(display, &kbdstate);

            autorepeat = (AutoRepeatModeOn == kbdstate.global_auto_repeat);

            XGetScreenSaver(display, &screensaver.timeout, &screensaver.interval, &screensaver.prefer_blanking, &screensaver.allow_exposures);

            window = XCreateWindow(display,
                XDefaultRootWindow(display), 0, 0,
                geometry[state].width,
                geometry[state].height,
                0, graphics_context->visual_info->depth,
                InputOutput, graphics_context->visual_info->visual,
                mask[state],
                &attrs[state]);

            if (!ce_graphics_context_make_current(graphics_context, display, window)) {
                throw game_error("render window", "could not set graphic context");
            }

            XSizeHints* size_hints = XAllocSizeHints();
            size_hints->flags = PSize | PMinSize;
            size_hints->min_width = 400;
            size_hints->min_height = 300;
            size_hints->base_width = geometry[state].width;
            size_hints->base_height = geometry[state].height;
            XSetWMNormalHints(display, window, size_hints);
            XFree(size_hints);

            XSetStandardProperties(display, window, title.c_str(), title.c_str(), None, NULL, 0, NULL);

            // handle wm_delete_events
            XSetWMProtocols(display, window, &atoms[CE_RENDERWINDOW_ATOM_WM_DELETE_WINDOW], 1);
        }

        virtual ~x11_window_t()
        {
            ce_graphics_context_del(graphics_context);
            ce_displaymng_del(displaymng);

            if (0 != window) {
                XDestroyWindow(display, window);
            }

            if (NULL != display) {
                XSetScreenSaver(display, screensaver.timeout, screensaver.interval, screensaver.prefer_blanking, screensaver.allow_exposures);
                if (autorepeat) {
                    XAutoRepeatOn(display);
                }
                XCloseDisplay(display);
            }
        }

    private:
        virtual void do_show() final
        {
            XMapRaised(display, window);
            // x and y values in XCreateWindow are ignored by most windows managers,
            // which means that top-level windows maybe placed somewhere else on the desktop
            XMoveWindow(display, window, geometry[state].x, geometry[state].y);
        }

        virtual void do_minimize() final
        {
            XIconifyWindow(display, window, XDefaultScreen(display));
        }

        virtual void do_toggle_fullscreen() final
        {
            if (CE_RENDERWINDOW_STATE_FULLSCREEN == state) {
                XGrabPointer(display, window, True, NoEventMask, GrabModeAsync, GrabModeAsync, window, None, CurrentTime);
                XGrabKeyboard(display, window, True, GrabModeAsync, GrabModeAsync, CurrentTime);
                XSetScreenSaver(display, DisableScreenSaver, DisableScreenInterval, DontPreferBlanking, DefaultExposures);
            } else {
                XUngrabPointer(display, CurrentTime);
                XUngrabKeyboard(display, CurrentTime);
                XSetScreenSaver(display, screensaver.timeout, screensaver.interval, screensaver.prefer_blanking, screensaver.allow_exposures);
            }

            XChangeWindowAttributes(display, window, mask[state], &attrs[state]);

            XEvent event;
            memset(&event, 0, sizeof(event));

            event.xclient.type = ClientMessage;
            event.xclient.send_event = True;
            event.xclient.window = window;
            event.xclient.message_type = atoms[CE_RENDERWINDOW_ATOM_NET_WM_STATE];
            event.xclient.format = 32;
            event.xclient.data.l[0] = state;
            event.xclient.data.l[1] = atoms[CE_RENDERWINDOW_ATOM_NET_WM_STATE_FULLSCREEN];

            // absolutely don't understand how event masks work...
            XSendEvent(display, XDefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
        }

        virtual void do_pump() final
        {
            XEvent event;
            while (XPending(display) > 0) {
                XNextEvent(display, &event);

                // is it possible? may be new version of the X server...
                assert(0 <= event.type && event.type < LASTEvent);

                // just in case
                if (event.type < LASTEvent) {
                    (this->*handlers[event.type])(&event);
                }
            }
        }

        void skip_handler(XEvent*)
        {
        }

        void client_message_handler(XEvent* event)
        {
            if (atoms[CE_RENDERWINDOW_ATOM_WM_PROTOCOLS] == event->xclient.message_type &&
                    atoms[CE_RENDERWINDOW_ATOM_WM_DELETE_WINDOW] == static_cast<Atom>(event->xclient.data.l[0])) {
                emit_closed();
            }
        }

        void map_notify_handler(XEvent*)
        {
            // TODO: restore window
        }

        void visibility_notify_handler(XEvent*)
        {
        }

        void configure_notify_handler(XEvent* event)
        {
            geometry[state].x = event->xconfigure.x;
            geometry[state].y = event->xconfigure.y;
            geometry[state].width = event->xconfigure.width;
            geometry[state].height = event->xconfigure.height;
            emit_resized(event->xconfigure.width, event->xconfigure.height);
        }

        void focus_in_handler(XEvent* event)
        {
            XAutoRepeatOff(event->xfocus.display);
        }

        void focus_out_handler(XEvent* event)
        {
            if (autorepeat) {
                XAutoRepeatOn(event->xfocus.display);
            }
            m_input_context->clear();
        }

        void enter_notify_handler(XEvent* event)
        {
            m_input_context->pointer_position.x = event->xcrossing.x;
            m_input_context->pointer_position.y = event->xcrossing.y;
        }

        void key_handler(XEvent* event, bool pressed)
        {
            // reset modifier keys to disable uppercase keys
            event->xkey.state = 0;

            KeySym key;
            XLookupString(&event->xkey, NULL, 0, &key, NULL);

            m_input_context->buttons[static_cast<size_t>(m_input_map[key])] = pressed;
            m_input_context->pointer_position.x = event->xkey.x;
            m_input_context->pointer_position.y = event->xkey.y;
        }

        void key_press_handler(XEvent* event)
        {
            key_handler(event, true);
        }

        void key_release_handler(XEvent* event)
        {
            key_handler(event, false);
        }

        void button_handler(XEvent* event, bool pressed)
        {
            m_input_context->buttons[event->xbutton.button - 1 + static_cast<size_t>(input_button_t::mb_left)] = pressed;
            m_input_context->pointer_position.x = event->xbutton.x;
            m_input_context->pointer_position.y = event->xbutton.y;
        }

        void button_press_handler(XEvent* event)
        {
            button_handler(event, true);
        }

        void button_release_handler(XEvent* event)
        {
            // special case: ignore wheel buttons
            // ButtonPress event is immediately followed by ButtonRelease event
            if (Button4 != event->xbutton.button && Button5 != event->xbutton.button) {
                button_handler(event, false);
            }
        }

        void motion_notify_handler(XEvent* event)
        {
            m_input_context->pointer_offset.x = event->xmotion.x - m_input_context->pointer_position.x;
            m_input_context->pointer_offset.y = event->xmotion.y - m_input_context->pointer_position.y;
            m_input_context->pointer_position.x = event->xmotion.x;
            m_input_context->pointer_position.y = event->xmotion.y;
        }

    private:
        Atom atoms[CE_RENDERWINDOW_ATOM_COUNT];
        unsigned long mask[CE_RENDERWINDOW_STATE_COUNT];
        XSetWindowAttributes attrs[CE_RENDERWINDOW_STATE_COUNT];
        void (x11_window_t::*handlers[LASTEvent])(XEvent*);
        bool autorepeat; // remember old auto repeat settings
        struct {
            int timeout, interval;
            int prefer_blanking;
            int allow_exposures;
        } screensaver; // remember old screen saver settings
        Display* display;
        Window window;
    };

    render_window_ptr_t make_render_window(const std::string& title)
    {
        return std::make_shared<x11_window_t>(title);
    }
}
