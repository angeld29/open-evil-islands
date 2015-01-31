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
#include <assert.h>

#include "cevideomanager.h"
#include "cevideoobject.h"

ce_video_object ce_video_object_new(const char* name)
{
    ce_video_instance* video_instance = ce_video_manager_create_instance(name);
    return NULL != video_instance ? video_instance->video_object : 0;
}

void ce_video_object_del(ce_video_object CE_UNUSED(video_object))
{
    // TODO
}

void ce_video_object_advance(ce_video_object video_object, float elapsed)
{
    ce_video_instance* video_instance = ce_video_manager_find_instance(video_object);
    if (NULL != video_instance) {
        ce_video_instance_advance(video_instance, elapsed);
    }
}

void ce_video_object_progress(ce_video_object video_object, int percents)
{
    ce_video_instance* video_instance = ce_video_manager_find_instance(video_object);
    if (NULL != video_instance) {
        ce_video_instance_progress(video_instance, percents);
    }
}

void ce_video_object_render(ce_video_object video_object)
{
    ce_video_instance* video_instance = ce_video_manager_find_instance(video_object);
    if (NULL != video_instance) {
        ce_video_instance_render(video_instance);
    }
}

bool ce_video_object_is_stopped(ce_video_object video_object)
{
    ce_video_instance* video_instance = ce_video_manager_find_instance(video_object);
    return NULL != video_instance ? ce_video_instance_is_stopped(video_instance) : true;
}

void ce_video_object_play(ce_video_object video_object)
{
    ce_video_instance* video_instance = ce_video_manager_find_instance(video_object);
    if (NULL != video_instance) {
        ce_video_instance_play(video_instance);
    }
}

void ce_video_object_pause(ce_video_object video_object)
{
    ce_video_instance* video_instance = ce_video_manager_find_instance(video_object);
    if (NULL != video_instance) {
        ce_video_instance_pause(video_instance);
    }
}

void ce_video_object_stop(ce_video_object video_object)
{
    ce_video_instance* video_instance = ce_video_manager_find_instance(video_object);
    if (NULL != video_instance) {
        ce_video_instance_stop(video_instance);
    }
}
