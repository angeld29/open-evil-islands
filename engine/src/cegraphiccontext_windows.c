/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#include <assert.h>

#include "cealloc.h"
#include "celogging.h"
#include "ceerror_windows.h"
#include "cegraphiccontext.h"
#include "cegraphiccontext_windows.h"

ce_graphic_context* ce_graphic_context_new(HDC dc)
{
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                             // version number
        PFD_DRAW_TO_WINDOW |           // support window
        PFD_SUPPORT_OPENGL |           // support opengl
        PFD_DOUBLEBUFFER,              // double buffered
        PFD_TYPE_RGBA,                 // RGBA type
        GetDeviceCaps(dc, BITSPIXEL),  // color depth
        0, 0, 0, 0, 0, 0,              // color bits ignored
        8,                             // alpha buffer
        0,                             // shift bit ignored
        0,                             // no accumulation buffer
        0, 0, 0, 0,                    // accumulation bits ignored
        24,                            // depth buffer
        8,                             // stencil buffer
        0,                             // no auxiliary buffer
        PFD_MAIN_PLANE,                // main drawing layer
        0,                             // reserved
        0, 0, 0                        // layer masks ignored
    };

    int pixel_format = ChoosePixelFormat(dc, &pfd);
    if (0 == pixel_format) {
        ce_error_report_windows_last("graphic context");
        ce_logging_fatal("graphic context: no appropriate visual found");
        return NULL;
    }

    DescribePixelFormat(dc, pixel_format, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    ce_graphic_context_visual_info(pixel_format,
        0 != (pfd.dwFlags & PFD_DOUBLEBUFFER),
        pfd.cColorBits, pfd.cRedBits, pfd.cGreenBits,
        pfd.cBlueBits, pfd.cAlphaBits, pfd.cDepthBits, pfd.cStencilBits);

    if (!SetPixelFormat(dc, pixel_format, &pfd)) {
        ce_error_report_windows_last("graphic context");
        ce_logging_fatal("graphic context: could not set pixel format");
        return NULL;
    }

    ce_graphic_context* graphic_context = ce_alloc_zero(sizeof(ce_graphic_context));

    graphic_context->context = wglCreateContext(dc);
    if (NULL == graphic_context->context) {
        ce_error_report_windows_last("graphic context");
        ce_logging_fatal("graphic context: could not create context");
        ce_graphic_context_del(graphic_context);
        return NULL;
    }

    assert(NULL == wglGetCurrentContext());
    wglMakeCurrent(dc, graphic_context->context);

    GLenum result;
    if (GLEW_OK != (result = glewInit()) || GLEW_OK != (result = wglewInit())) {
        ce_logging_fatal("graphic context: %s", glewGetErrorString(result));
        ce_graphic_context_del(graphic_context);
        return NULL;
    }

    return graphic_context;
}

void ce_graphic_context_del(ce_graphic_context* graphic_context)
{
    if (NULL != graphic_context) {
        assert(wglGetCurrentContext() == graphic_context->context);
        if (NULL != graphic_context->context) {
            wglMakeCurrent(wglGetCurrentDC(), NULL);
            wglDeleteContext(graphic_context->context);
        }
        ce_free(graphic_context, sizeof(ce_graphic_context));
    }
}

void ce_graphic_context_swap(ce_graphic_context* CE_UNUSED(graphic_context))
{
    assert(NULL != wglGetCurrentContext());
    assert(wglGetCurrentContext() == graphic_context->context);
    SwapBuffers(wglGetCurrentDC());
}
