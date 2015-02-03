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

#ifndef CE_VIDEOOBJECT_HPP
#define CE_VIDEOOBJECT_HPP

namespace cursedearth
{
    typedef unsigned long int ce_video_object;

    ce_video_object ce_video_object_new(const char* name);
    void ce_video_object_del(ce_video_object video_object);

    void ce_video_object_advance(ce_video_object video_object, float elapsed);
    void ce_video_object_progress(ce_video_object video_object, int percents);

    void ce_video_object_render(ce_video_object video_object);

    bool ce_video_object_is_stopped(ce_video_object video_object);

    void ce_video_object_play(ce_video_object video_object);
    void ce_video_object_pause(ce_video_object video_object);
    void ce_video_object_stop(ce_video_object video_object);
}

#endif
