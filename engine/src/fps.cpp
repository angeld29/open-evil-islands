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

#include <cstdio>

#include "str.hpp"
#include "alloc.hpp"
#include "fps.hpp"

namespace cursedearth
{
    fps_t* ce_fps_new(void)
    {
        fps_t* fps = (fps_t*)ce_alloc(sizeof(fps_t));
        fps->frame_count = 0;
        fps->time = 0.0f;
        ce_strlcpy(fps->text, "---", sizeof(fps->text));
        return fps;
    }

    void ce_fps_del(fps_t* fps)
    {
        ce_free(fps, sizeof(fps_t));
    }

    void ce_fps_advance(fps_t* fps, float elapsed)
    {
        ++fps->frame_count;
        if ((fps->time += elapsed) >= 1.0f) {
            snprintf(fps->text, sizeof(fps->text), "%d", fps->frame_count);
            fps->frame_count = 0;
            fps->time = 0.0f;
        }
    }
}
