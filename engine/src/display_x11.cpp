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

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/extensions/xf86vmproto.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/extensions/Xrandr.h>

#include "alloc.hpp"
#include "logging.hpp"
#include "display.hpp"
#include "display_posix.hpp"

// XFree86 VidMode, see xf86vidmode(3) for more details
typedef struct {
    int event_base, error_base;
    int major_version, minor_version;
    Display* display;
    XF86VidModeModeInfo** modes;
} ce_xf86vmmng;

static int ce_xf86vmmng_calc_rate(XF86VidModeModeInfo* info)
{
    // dotclock comes in kHz
    return (info->dotclock * 1000.0f) / (info->htotal * info->vtotal) + 0.5f;
}

static void ce_xf86vmmng_ctor(ce_displaymng* displaymng, va_list args)
{
    ce_xf86vmmng* xf86vmmng = (ce_xf86vmmng*)displaymng->impl;
    xf86vmmng->display = va_arg(args, Display*);

    XF86VidModeQueryExtension(xf86vmmng->display, &xf86vmmng->event_base,
                                                &xf86vmmng->error_base);
    XF86VidModeQueryVersion(xf86vmmng->display, &xf86vmmng->major_version,
                                            &xf86vmmng->minor_version);

    ce_logging_write("displaymng: using XFree86 Video Mode Extension %d.%d",
        xf86vmmng->major_version, xf86vmmng->minor_version);

    int bpp = XDefaultDepth(xf86vmmng->display, XDefaultScreen(xf86vmmng->display));

    int mode_count;
    XF86VidModeGetAllModeLines(xf86vmmng->display,
        XDefaultScreen(xf86vmmng->display), &mode_count, &xf86vmmng->modes);

    for (int i = 0; i < mode_count; ++i) {
        XF86VidModeModeInfo* info = xf86vmmng->modes[i];
        ce_vector_push_back(displaymng->supported_modes,
            ce_displaymode_new(info->hdisplay, info->vdisplay,
                bpp, ce_xf86vmmng_calc_rate(info)));
    }
}

static void ce_xf86vmmng_enter(ce_displaymng* displaymng, size_t index,
                                ce_display_rotation CE_UNUSED(rotation),
                                ce_display_reflection CE_UNUSED(reflection))
{
    ce_xf86vmmng* xf86vmmng = (ce_xf86vmmng*)displaymng->impl;

    if (NULL != xf86vmmng->modes) {
        XF86VidModeSwitchToMode(xf86vmmng->display,
            XDefaultScreen(xf86vmmng->display), xf86vmmng->modes[index]);
        XF86VidModeSetViewPort(xf86vmmng->display,
            XDefaultScreen(xf86vmmng->display), 0, 0);
    }
}

static void ce_xf86vmmng_exit(ce_displaymng* displaymng)
{
    ce_xf86vmmng_enter(displaymng, 0, CE_DISPLAY_ROTATION_NONE,
                                        CE_DISPLAY_REFLECTION_NONE);
}

static void ce_xf86vmmng_dtor(ce_displaymng* displaymng)
{
    ce_xf86vmmng* xf86vmmng = (ce_xf86vmmng*)displaymng->impl;

    if (NULL != xf86vmmng->modes) {
        ce_xf86vmmng_exit(displaymng);
        XFree(xf86vmmng->modes);
    }
}

static bool ce_xf86vmmng_query(Display* display)
{
    const int major_version_req = 2, minor_version_req = 0;
    int dummy, major_version = 0, minor_version = 0;

    if (XF86VidModeQueryExtension(display, &dummy, &dummy) &&
            XF86VidModeQueryVersion(display, &major_version, &minor_version) &&
            major_version == major_version_req &&
            minor_version >= minor_version_req) {
        return true;
    }

    ce_logging_warning("displaymng: XFree86 Video Mode Extension %d.%d found, "
        "%d.>=%d required, discarded", major_version, minor_version,
                                        major_version_req, minor_version_req);
    return false;
}

// XRandR, see xrandr(3) for more details
typedef struct {
    int event_base, error_base;
    int major_version, minor_version;
    Display* display;
    XRRScreenConfiguration* conf;
    int size_count;
    XRRScreenSize* sizes;
    SizeID orig_size;
    short orig_rate;
    Rotation orig_rotation;
} ce_xrrmng;

static ce_display_rotation ce_xrrmng_xrr2ce_rotation(Rotation xrr_rotation)
{
    ce_display_rotation rotation = CE_DISPLAY_ROTATION_NONE;
    const ce_display_rotation rotations[] = { CE_DISPLAY_ROTATION_0,
                                                CE_DISPLAY_ROTATION_90,
                                                CE_DISPLAY_ROTATION_180,
                                                CE_DISPLAY_ROTATION_270 };
    const Rotation xrr_rotations[] = { RR_Rotate_0, RR_Rotate_90,
                                        RR_Rotate_180, RR_Rotate_270 };
    for (size_t i = 0; i < sizeof(xrr_rotations) /
                            sizeof(xrr_rotations[0]); ++i) {
        if (xrr_rotation & xrr_rotations[i]) {
            rotation |= rotations[i];
        }
    }
    return rotation;
}

static ce_display_reflection ce_xrrmng_xrr2ce_reflection(Rotation xrr_reflection)
{
    ce_display_reflection reflection = CE_DISPLAY_REFLECTION_NONE;
    const ce_display_reflection reflections[] = { CE_DISPLAY_REFLECTION_X,
                                                    CE_DISPLAY_REFLECTION_Y };
    const Rotation xrr_reflections[] = { RR_Reflect_X, RR_Reflect_Y };
    for (size_t i = 0; i < sizeof(xrr_reflections) /
                            sizeof(xrr_reflections[0]); ++i) {
        if (xrr_reflection & xrr_reflections[i]) {
            reflection |= reflections[i];
        }
    }
    return reflection;
}

static Rotation ce_xrrmng_ce2xrr_rotation(ce_display_rotation rotation,
                                        ce_display_reflection reflection)
{
    Rotation xrr_rotation = 0;
    const Rotation xrr_rotations[] = { RR_Rotate_0, RR_Rotate_90,
                                        RR_Rotate_180, RR_Rotate_270 };
    const Rotation xrr_reflections[] = { RR_Reflect_X, RR_Reflect_Y };
    const ce_display_rotation rotations[] = { CE_DISPLAY_ROTATION_0,
                                                CE_DISPLAY_ROTATION_90,
                                                CE_DISPLAY_ROTATION_180,
                                                CE_DISPLAY_ROTATION_270 };
    const ce_display_reflection reflections[] = { CE_DISPLAY_REFLECTION_X,
                                                    CE_DISPLAY_REFLECTION_Y };
    for (size_t i = 0; i < sizeof(rotations) / sizeof(rotations[0]); ++i) {
        if (rotation & rotations[i]) {
            xrr_rotation |= xrr_rotations[i];
        }
    }
    if (0 == xrr_rotation) {
        xrr_rotation = RR_Rotate_0;
    }
    for (size_t i = 0; i < sizeof(reflections) / sizeof(reflections[0]); ++i) {
        if (reflection & reflections[i]) {
            xrr_rotation |= xrr_reflections[i];
        }
    }
    return xrr_rotation;
}

static void ce_xrrmng_ctor(ce_displaymng* displaymng, va_list args)
{
    ce_xrrmng* xrrmng = (ce_xrrmng*)displaymng->impl;
    xrrmng->display = va_arg(args, Display*);

    XRRQueryExtension(xrrmng->display, &xrrmng->event_base, &xrrmng->error_base);
    XRRQueryVersion(xrrmng->display, &xrrmng->major_version, &xrrmng->minor_version);

    ce_logging_write("displaymng: using XRandR Extension %d.%d",
        xrrmng->major_version, xrrmng->minor_version);

    int bpp = XDefaultDepth(xrrmng->display, XDefaultScreen(xrrmng->display));

    // TODO: what about events?
    // request screen change notifications
    //XRRSelectInput(dpy, win, RRScreenChangeNotifyMask);

    // event.type - XRandR.EventBase
    // RRScreenChangeNotify
    // Show XRandR that we really care
    // XRRUpdateConfiguration( &event );

    xrrmng->conf = XRRGetScreenInfo(xrrmng->display, XDefaultRootWindow(xrrmng->display));
    xrrmng->orig_size = XRRConfigCurrentConfiguration(xrrmng->conf, &xrrmng->orig_rotation);
    xrrmng->orig_rate = XRRConfigCurrentRate(xrrmng->conf);

    // get the possible set of rotations/reflections supported
    Rotation rotation = XRRConfigRotations(xrrmng->conf, &xrrmng->orig_rotation);

    displaymng->supported_rotation = ce_xrrmng_xrr2ce_rotation(rotation);
    displaymng->supported_reflection = ce_xrrmng_xrr2ce_reflection(rotation);

    // get possible screen resolutions
    xrrmng->sizes = XRRConfigSizes(xrrmng->conf, &xrrmng->size_count);

    for (int i = 0; i < xrrmng->size_count; ++i) {
        int rate_count;
        short* rates = XRRConfigRates(xrrmng->conf, i, &rate_count);
        for (int j = 0; j < rate_count; ++j) {
            ce_vector_push_back(displaymng->supported_modes,
                ce_displaymode_new(xrrmng->sizes[i].width,
                    xrrmng->sizes[i].height, bpp, rates[j]));
        }
    }
}

static void ce_xrrmng_enter(ce_displaymng* displaymng, size_t index,
    ce_display_rotation rotation, ce_display_reflection reflection)
{
    ce_xrrmng* xrrmng = (ce_xrrmng*)displaymng->impl;
    ce_displaymode* mode = displaymng->supported_modes->items[index];
    SizeID size_index;

    for (size_index = xrrmng->size_count - 1; size_index > 0; --size_index) {
        if (mode->width == xrrmng->sizes[size_index].width &&
                mode->height == xrrmng->sizes[size_index].height) {
            break;
        }
    }

    XRRSetScreenConfigAndRate(xrrmng->display, xrrmng->conf,
        XDefaultRootWindow(xrrmng->display), size_index,
        ce_xrrmng_ce2xrr_rotation(rotation, reflection),
        mode->rate, CurrentTime);
}

static void ce_xrrmng_exit(ce_displaymng* displaymng)
{
    ce_xrrmng* xrrmng = (ce_xrrmng*)displaymng->impl;

    XRRSetScreenConfigAndRate(xrrmng->display, xrrmng->conf,
        XDefaultRootWindow(xrrmng->display), xrrmng->orig_size,
        xrrmng->orig_rotation, xrrmng->orig_rate, CurrentTime);
}

static void ce_xrrmng_dtor(ce_displaymng* displaymng)
{
    ce_xrrmng* xrrmng = (ce_xrrmng*)displaymng->impl;

    ce_xrrmng_exit(displaymng);
    XRRFreeScreenConfigInfo(xrrmng->conf);
}

static bool ce_xrrmng_query(Display* display)
{
    const int major_version_req = 1, minor_version_req = 1;
    int dummy, major_version = 0, minor_version = 0;

    if (XRRQueryExtension(display, &dummy, &dummy) &&
            XRRQueryVersion(display, &major_version, &minor_version) &&
            major_version == major_version_req &&
            minor_version >= minor_version_req) {
        return true;
    }

    ce_logging_warning("displaymng: XRandR Extension %d.%d found, "
        "%d.>=%d required, discarded", major_version, minor_version,
                                        major_version_req, minor_version_req);
    return false;
}

ce_displaymng* ce_displaymng_create(Display* display)
{
    struct {
        const char* name;
        ce_displaymng_vtable vtable;
        size_t size;
        bool (*query)(Display* display);
    } extensions[] = { // prefer XRandR
        { RANDR_NAME, { ce_xrrmng_ctor, ce_xrrmng_dtor,
                        ce_xrrmng_enter, ce_xrrmng_exit },
            sizeof(ce_xrrmng), ce_xrrmng_query },
        { XF86VIDMODENAME, { ce_xf86vmmng_ctor, ce_xf86vmmng_dtor,
                            ce_xf86vmmng_enter, ce_xf86vmmng_exit },
            sizeof(ce_xf86vmmng), ce_xf86vmmng_query },
    };

    for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); ++i) {
        int dummy;
        if (!XQueryExtension(display, extensions[i].name, &dummy, &dummy, &dummy)) {
            ce_logging_warning("displaymng: X11 extension "
                                "'%s' not supported", extensions[i].name);
            continue;
        }
        if ((*extensions[i].query)(display)) {
            return ce_displaymng_new(extensions[i].vtable, extensions[i].size, display);
        }
    }

    ce_logging_warning("displaymng: no appropriate X11 extensions found, "
                        "fullscreen mode is not available");
    return NULL;
}
