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

#include "videoresource.hpp"
#include "alloc.hpp"

namespace cursedearth
{
    ce_video_resource* ce_video_resource_new(ce_mem_file* mem_file)
    {
        size_t index;
        for (index = 0; index < CE_VIDEO_RESOURCE_BUILTIN_COUNT; ++index) {
            ce_mem_file_rewind(mem_file);
            if ((*ce_video_resource_builtins[index].test)(mem_file)) {
                break;
            }
        }

        ce_mem_file_rewind(mem_file);
        if (CE_VIDEO_RESOURCE_BUILTIN_COUNT == index) {
            return NULL;
        }

        size_t size = (*ce_video_resource_builtins[index].size_hint)(mem_file);
        ce_mem_file_rewind(mem_file);

        ce_video_resource* video_resource =(ce_video_resource*)ce_alloc_zero(sizeof(ce_video_resource));
        video_resource->impl = ce_alloc_zero(size);
        video_resource->mem_file = mem_file;
        video_resource->vtable = ce_video_resource_builtins[index];
        video_resource->size = size;

        if (!(*video_resource->vtable.ctor)(video_resource)) {
            // do not take ownership if failed
            video_resource->mem_file = NULL;
            ce_video_resource_del(video_resource);
            return NULL;
        }

        return video_resource;
    }

    void ce_video_resource_del(ce_video_resource* video_resource)
    {
        if (NULL != video_resource) {
            if (NULL != video_resource->vtable.dtor) {
                (*video_resource->vtable.dtor)(video_resource);
            }
            ce_mem_file_del(video_resource->mem_file);
            ce_free(video_resource->impl, video_resource->size);
            ce_free(video_resource, sizeof(ce_video_resource));
        }
    }

    bool ce_video_resource_reset(ce_video_resource* video_resource)
    {
        video_resource->time = 0.0f;
        video_resource->frame_index = 0;
        ce_mem_file_rewind(video_resource->mem_file);
        return (*video_resource->vtable.reset)(video_resource);
    }
}
