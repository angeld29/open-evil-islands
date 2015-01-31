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

#include "cestr.h"
#include "cealloc.h"
#include "cefps.h"

ce_fps* ce_fps_new(void)
{
    ce_fps* fps = ce_alloc(sizeof(ce_fps));
    fps->frame_count = 0;
    fps->time = 0.0f;
    ce_strlcpy(fps->text, "---", sizeof(fps->text));
    return fps;
}

void ce_fps_del(ce_fps* fps)
{
    ce_free(fps, sizeof(ce_fps));
}

void ce_fps_advance(ce_fps* fps, float elapsed)
{
    ++fps->frame_count;
    if ((fps->time += elapsed) >= 1.0f) {
        snprintf(fps->text, sizeof(fps->text), "%d", fps->frame_count);
        fps->frame_count = 0;
        fps->time = 0.0f;
    }
}
