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

#ifndef CE_GRAPHICSCONTEXT_PLATFORM_HPP
#define CE_GRAPHICSCONTEXT_PLATFORM_HPP

#include "graphicscontext.hpp"
#include "glew_x11.hpp"

namespace cursedearth
{
    struct ce_graphics_context {
        int error_base, event_base;
        int major_version, minor_version;
        XVisualInfo* visual_info;
        GLXContext context;
    };

    ce_graphics_context* ce_graphics_context_new(Display* display);

    bool ce_graphics_context_make_current(ce_graphics_context* graphics_context, Display* display, GLXDrawable drawable);
}

#endif
