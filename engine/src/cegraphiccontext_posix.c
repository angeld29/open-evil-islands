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

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cegraphiccontext.h"
#include "cegraphiccontext_posix.h"

void ce_graphiccontext_del(ce_graphiccontext* graphiccontext)
{
	if (NULL != graphiccontext) {
		assert(glXGetCurrentContext() == graphiccontext->context);
		if (NULL != graphiccontext->context) {
			Display* display = glXGetCurrentDisplay();
			glXMakeCurrent(display, None, NULL);
			glXDestroyContext(display, graphiccontext->context);
		}
		if (NULL != graphiccontext->visualinfo) {
			XFree(graphiccontext->visualinfo);
		}
		ce_free(graphiccontext, sizeof(ce_graphiccontext));
	}
}

void ce_graphiccontext_swap(ce_graphiccontext* graphiccontext)
{
	ce_unused(graphiccontext);
	assert(NULL != glXGetCurrentContext());
	assert(glXGetCurrentContext() == graphiccontext->context);
	glXSwapBuffers(glXGetCurrentDisplay(), glXGetCurrentDrawable());
}

ce_graphiccontext* ce_graphiccontext_create(Display* display)
{
	int error_base, event_base;
	const int major_version_req = 1, minor_version_req = 2;
	int major_version, minor_version;

	if (!glXQueryExtension(display, &error_base, &event_base) ||
			!glXQueryVersion(display, &major_version, &minor_version)) {
		ce_logging_fatal("graphiccontext: no GLX support available");
		return NULL;
	}

	ce_logging_write("graphiccontext: using GLX %d.%d", major_version, minor_version);

	if (major_version_req != major_version || minor_version < minor_version_req) {
		ce_logging_fatal("graphiccontext: GLX %d.>=%d required",
			major_version_req, minor_version_req);
		return NULL;
	}

	XVisualInfo* visualinfo = glXChooseVisual(display, XDefaultScreen(display),
		(int[]) { GLX_DOUBLEBUFFER, GLX_RGBA,
					GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
					GLX_BLUE_SIZE, 1, GLX_ALPHA_SIZE, 1,
					GLX_DEPTH_SIZE, 1, GLX_STENCIL_SIZE, 1, None });
	if (NULL == visualinfo) {
		ce_logging_fatal("graphiccontext: no appropriate visual found");
		return NULL;
	}

	int db, sz, r, g, b, a, dp, st;

	glXGetConfig(display, visualinfo, GLX_DOUBLEBUFFER, &db);
	glXGetConfig(display, visualinfo, GLX_BUFFER_SIZE, &sz);
	glXGetConfig(display, visualinfo, GLX_RED_SIZE, &r);
	glXGetConfig(display, visualinfo, GLX_GREEN_SIZE, &g);
	glXGetConfig(display, visualinfo, GLX_BLUE_SIZE, &b);
	glXGetConfig(display, visualinfo, GLX_ALPHA_SIZE, &a);
	glXGetConfig(display, visualinfo, GLX_DEPTH_SIZE, &dp);
	glXGetConfig(display, visualinfo, GLX_STENCIL_SIZE, &st);

	ce_graphiccontext_visualinfo(visualinfo->visualid, db, sz, r, g, b, a, dp, st);

	ce_graphiccontext* graphiccontext = ce_alloc(sizeof(ce_graphiccontext));
	graphiccontext->error_base = error_base;
	graphiccontext->event_base = event_base;
	graphiccontext->major_version = major_version;
	graphiccontext->minor_version = minor_version;
	graphiccontext->visualinfo = visualinfo;
	graphiccontext->context = glXCreateContext(display, visualinfo, NULL, True);

	if (glXIsDirect(display, graphiccontext->context)) {
		ce_logging_write("graphiccontext: you have direct rendering");
	} else {
		ce_logging_warning("graphiccontext: no direct rendering possible");
	}

	return graphiccontext;
}

bool ce_graphiccontext_make_current(ce_graphiccontext* graphiccontext,
									Display* display, GLXDrawable drawable)
{
	assert(NULL == glXGetCurrentContext());
	glXMakeCurrent(display, drawable, graphiccontext->context);

	GLenum result;
	if (GLEW_OK != (result = glewInit()) || GLEW_OK != (result = glxewInit())) {
		ce_logging_fatal("graphiccontext: %s", glewGetErrorString(result));
		return false;
	}

	return true;
}
