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

#include "videomanager.hpp"
#include "videoobject.hpp"

namespace cursedearth
{
    video_object_t make_video_object(const std::string& name)
    {
        return video_manager_t::instance()->make_instance(name);
    }

    void remove_video_object(video_object_t)
    {
    }

    bool video_object_is_valid(video_object_t video_object)
    {
        return 0 != video_object && video_manager_t::instance()->find_instance(video_object);
    }

    void video_object_advance(video_object_t video_object, float elapsed)
    {
        if (video_instance_ptr_t instance = video_manager_t::instance()->find_instance(video_object)) {
            instance->advance(elapsed);
        }
    }

    void video_object_progress(video_object_t video_object, int percents)
    {
        if (video_instance_ptr_t instance = video_manager_t::instance()->find_instance(video_object)) {
            instance->progress(percents);
        }
    }

    void video_object_render(video_object_t video_object)
    {
        if (video_instance_ptr_t instance = video_manager_t::instance()->find_instance(video_object)) {
            instance->render();
        }
    }

    bool video_object_is_stopped(video_object_t video_object)
    {
        video_instance_ptr_t instance = video_manager_t::instance()->find_instance(video_object);
        return instance ? instance->is_stopped() : true;
    }

    void play_video_object(video_object_t video_object)
    {
        if (video_instance_ptr_t instance = video_manager_t::instance()->find_instance(video_object)) {
            instance->play();
        }
    }

    void pause_video_object(video_object_t video_object)
    {
        if (video_instance_ptr_t instance = video_manager_t::instance()->find_instance(video_object)) {
            instance->pause();
        }
    }

    void stop_video_object(video_object_t video_object)
    {
        if (video_instance_ptr_t instance = video_manager_t::instance()->find_instance(video_object)) {
            instance->stop();
        }
    }
}
