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
#include "graphicscontext_x11.hpp"

namespace cursedearth
{
    enum {
        CE_GLX_MAJOR_VERSION_REQUIRED = 1,
        CE_GLX_MINOR_VERSION_MINIMUM = 2,
    };

    ce_graphics_context* ce_graphics_context_new(Display* display)
    {
        ce_graphics_context* graphics_context = (ce_graphics_context*)ce_alloc_zero(sizeof(ce_graphics_context));

        if (!glXQueryExtension(display, &graphics_context->error_base, &graphics_context->event_base) ||
                !glXQueryVersion(display, &graphics_context->major_version, &graphics_context->minor_version)) {
            ce_logging_fatal("graphic context: no GLX support available");
            ce_graphics_context_del(graphics_context);
            return NULL;
        }

        ce_logging_info("graphic context: using GLX %d.%d", graphics_context->major_version, graphics_context->minor_version);

        if (CE_GLX_MAJOR_VERSION_REQUIRED != graphics_context->major_version || graphics_context->minor_version < CE_GLX_MINOR_VERSION_MINIMUM) {
            ce_logging_fatal("graphic context: GLX %d.>=%d required", CE_GLX_MAJOR_VERSION_REQUIRED, CE_GLX_MINOR_VERSION_MINIMUM);
            ce_graphics_context_del(graphics_context);
            return NULL;
        }

        int attrs[] = { GLX_DOUBLEBUFFER, GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1, GLX_ALPHA_SIZE, 1, GLX_DEPTH_SIZE, 1, GLX_STENCIL_SIZE, 1, None };
        graphics_context->visual_info = glXChooseVisual(display, XDefaultScreen(display), attrs);
        if (NULL == graphics_context->visual_info) {
            ce_logging_fatal("graphic context: no appropriate visual found");
            ce_graphics_context_del(graphics_context);
            return NULL;
        }

        int db, sz, r, g, b, a, dp, st;
        glXGetConfig(display, graphics_context->visual_info, GLX_DOUBLEBUFFER, &db);
        glXGetConfig(display, graphics_context->visual_info, GLX_BUFFER_SIZE, &sz);
        glXGetConfig(display, graphics_context->visual_info, GLX_RED_SIZE, &r);
        glXGetConfig(display, graphics_context->visual_info, GLX_GREEN_SIZE, &g);
        glXGetConfig(display, graphics_context->visual_info, GLX_BLUE_SIZE, &b);
        glXGetConfig(display, graphics_context->visual_info, GLX_ALPHA_SIZE, &a);
        glXGetConfig(display, graphics_context->visual_info, GLX_DEPTH_SIZE, &dp);
        glXGetConfig(display, graphics_context->visual_info, GLX_STENCIL_SIZE, &st);

        ce_graphics_context_visual_info(graphics_context->visual_info->visualid, db, sz, r, g, b, a, dp, st);

        graphics_context->context = glXCreateContext(display, graphics_context->visual_info, NULL, True);
        if (NULL == graphics_context->context) {
            ce_logging_fatal("graphic context: could not create context");
            ce_graphics_context_del(graphics_context);
            return NULL;
        }

        if (glXIsDirect(display, graphics_context->context)) {
            ce_logging_info("graphic context: you have direct rendering");
        } else {
            ce_logging_warning("graphic context: no direct rendering possible");
        }

        return graphics_context;
    }

    void ce_graphics_context_del(ce_graphics_context* graphics_context)
    {
        if (NULL != graphics_context) {
            assert(glXGetCurrentContext() == graphics_context->context);
            if (NULL != graphics_context->context) {
                Display* display = glXGetCurrentDisplay();
                glXMakeCurrent(display, None, NULL);
                glXDestroyContext(display, graphics_context->context);
            }
            if (NULL != graphics_context->visual_info) {
                XFree(graphics_context->visual_info);
            }
            ce_free(graphics_context, sizeof(ce_graphics_context));
        }
    }

    bool ce_graphics_context_make_current(ce_graphics_context* graphics_context, Display* display, GLXDrawable drawable)
    {
        assert(NULL == glXGetCurrentContext());
        glXMakeCurrent(display, drawable, graphics_context->context);

        GLenum result;
        if (GLEW_OK != (result = glewInit()) || GLEW_OK != (result = glxewInit())) {
            ce_logging_fatal("graphic context: %s", glewGetErrorString(result));
            return false;
        }

        return true;
    }

    void ce_graphics_context_swap(ce_graphics_context* graphics_context)
    {
        assert(NULL != glXGetCurrentContext());
        assert(glXGetCurrentContext() == graphics_context->context);
        glXSwapBuffers(glXGetCurrentDisplay(), glXGetCurrentDrawable());
    }
}
