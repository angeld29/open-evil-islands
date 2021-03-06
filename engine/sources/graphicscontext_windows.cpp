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

#include <cassert>

#include "alloc.hpp"
#include "logging.hpp"
#include "graphicscontext_windows.hpp"

namespace cursedearth
{
    ce_graphics_context* ce_graphics_context_new(HDC dc)
    {
        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,                                  // version number
            PFD_DRAW_TO_WINDOW |                // support window
            PFD_SUPPORT_OPENGL |                // support opengl
            PFD_DOUBLEBUFFER,                   // double buffered
            PFD_TYPE_RGBA,                      // RGBA type
            (BYTE)GetDeviceCaps(dc, BITSPIXEL), // color depth
            0, 0, 0, 0, 0, 0,                   // color bits ignored
            8,                                  // alpha buffer
            0,                                  // shift bit ignored
            0,                                  // no accumulation buffer
            0, 0, 0, 0,                         // accumulation bits ignored
            24,                                 // depth buffer
            8,                                  // stencil buffer
            0,                                  // no auxiliary buffer
            PFD_MAIN_PLANE,                     // main drawing layer
            0,                                  // reserved
            0, 0, 0                             // layer masks ignored
        };

        int pixel_format = ChoosePixelFormat(dc, &pfd);
        if (0 == pixel_format) {
            ce_logging_fatal("graphic context: no appropriate visual found");
            return NULL;
        }

        DescribePixelFormat(dc, pixel_format, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        ce_graphics_context_visual_info(pixel_format, 0 != (pfd.dwFlags & PFD_DOUBLEBUFFER), pfd.cColorBits,
            pfd.cRedBits, pfd.cGreenBits, pfd.cBlueBits, pfd.cAlphaBits, pfd.cDepthBits, pfd.cStencilBits);

        if (!SetPixelFormat(dc, pixel_format, &pfd)) {
            ce_logging_fatal("graphic context: could not set pixel format");
            return NULL;
        }

        ce_graphics_context* graphics_context = (ce_graphics_context*)ce_alloc_zero(sizeof(ce_graphics_context));

        graphics_context->context = wglCreateContext(dc);
        if (NULL == graphics_context->context) {
            ce_logging_fatal("graphic context: could not create context");
            ce_graphics_context_del(graphics_context);
            return NULL;
        }

        assert(NULL == wglGetCurrentContext());
        wglMakeCurrent(dc, graphics_context->context);

        GLenum result;
        if (GLEW_OK != (result = glewInit()) || GLEW_OK != (result = wglewInit())) {
            ce_logging_fatal("graphic context: %s", glewGetErrorString(result));
            ce_graphics_context_del(graphics_context);
            return NULL;
        }

        return graphics_context;
    }

    void ce_graphics_context_del(ce_graphics_context* graphics_context)
    {
        if (NULL != graphics_context) {
            assert(wglGetCurrentContext() == graphics_context->context);
            if (NULL != graphics_context->context) {
                wglMakeCurrent(wglGetCurrentDC(), NULL);
                wglDeleteContext(graphics_context->context);
            }
            ce_free(graphics_context, sizeof(ce_graphics_context));
        }
    }

    void ce_graphics_context_swap(ce_graphics_context* graphics_context)
    {
        assert(NULL != wglGetCurrentContext());
        assert(wglGetCurrentContext() == graphics_context->context);
        SwapBuffers(wglGetCurrentDC());
    }
}
