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

#ifndef CE_GRAPHICCONTEXT_PLATFORM_H
#define CE_GRAPHICCONTEXT_PLATFORM_H

#include <stdbool.h>

#include "cegraphiccontext.h"
#include "ceglew_posix.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ce_graphic_context {
    int error_base, event_base;
    int major_version, minor_version;
    XVisualInfo* visual_info;
    GLXContext context;
};

extern ce_graphic_context* ce_graphic_context_new(Display* display);

extern bool ce_graphic_context_make_current(ce_graphic_context* graphic_context,
                                    Display* display, GLXDrawable drawable);

#ifdef __cplusplus
}
#endif

#endif /* CE_GRAPHICCONTEXT_PLATFORM_H */
