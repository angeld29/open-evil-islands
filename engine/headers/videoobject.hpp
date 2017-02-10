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

#ifndef CE_VIDEOOBJECT_HPP
#define CE_VIDEOOBJECT_HPP

#include <string>

namespace cursedearth
{
    typedef unsigned long video_object_t;

    video_object_t make_video_object(const std::string&);
    void remove_video_object(video_object_t);

    bool video_object_is_valid(video_object_t);

    void video_object_advance(video_object_t, float elapsed);
    void video_object_progress(video_object_t, int percents);
    void video_object_render(video_object_t);

    bool video_object_is_stopped(video_object_t);
    void play_video_object(video_object_t);
    void pause_video_object(video_object_t);
    void stop_video_object(video_object_t);
}

#endif
