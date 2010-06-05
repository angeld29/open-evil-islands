/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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

#include "ceglew_win32.h"

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cegraphiccontext.h"

#include "cegraphiccontext_windows.h"

void ce_graphiccontext_del(ce_graphiccontext* graphiccontext)
{
	if (NULL != graphiccontext) {
		assert(wglGetCurrentContext() == graphiccontext->context);
		if (NULL != graphiccontext->context) {
			wglMakeCurrent(wglGetCurrentDC(), NULL);
			wglDeleteContext(graphiccontext->context);
		}
		ce_free(graphiccontext, sizeof(ce_graphiccontext));
	}
}

void ce_graphiccontext_swap(ce_graphiccontext* graphiccontext)
{
	ce_unused(graphiccontext);
	assert(NULL != wglGetCurrentContext());
	assert(wglGetCurrentContext() == graphiccontext->context);
	SwapBuffers(wglGetCurrentDC());
}

ce_graphiccontext* ce_graphiccontext_create(HDC dc)
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
		ce_logging_fatal("graphiccontext: no appropriate visual found");
		return NULL;
	}

	DescribePixelFormat(dc, pixel_format, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	ce_graphiccontext_visualinfo(pixel_format, 0 != (pfd.dwFlags & PFD_DOUBLEBUFFER),
		pfd.cColorBits, pfd.cRedBits, pfd.cGreenBits,
		pfd.cBlueBits, pfd.cAlphaBits, pfd.cDepthBits, pfd.cStencilBits);

	if (!SetPixelFormat(dc, pixel_format, &pfd)) {
		ce_logging_fatal("graphiccontext: could not set pixel format");
		return NULL;
	}

	ce_graphiccontext* graphiccontext = ce_alloc(sizeof(ce_graphiccontext));
	graphiccontext->context = wglCreateContext(dc);

	assert(NULL == wglGetCurrentContext());
	wglMakeCurrent(dc, graphiccontext->context);

	GLenum result;
	if (GLEW_OK != (result = glewInit()) || GLEW_OK != (result = wglewInit())) {
		ce_logging_fatal("graphiccontext: %s", glewGetErrorString(result));
		ce_graphiccontext_del(graphiccontext);
		return NULL;
	}

	return graphiccontext;
}
