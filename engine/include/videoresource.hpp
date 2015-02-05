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

#ifndef CE_VIDEORESOURCE_HPP
#define CE_VIDEORESOURCE_HPP

#include <cstddef>

#include "memfile.hpp"
#include "ycbcr.hpp"

namespace cursedearth
{
    typedef struct ce_video_resource ce_video_resource;

    typedef struct {
        size_t (*size_hint)(ce_mem_file* mem_file);
        bool (*test)(ce_mem_file* mem_file);
        bool (*ctor)(ce_video_resource* video_resource);
        void (*dtor)(ce_video_resource* video_resource);
        bool (*read)(ce_video_resource* video_resource);
        bool (*reset)(ce_video_resource* video_resource);
    } ce_video_resource_vtable;

    struct ce_video_resource {
        unsigned int width, height;
        float fps, time;
        size_t frame_index, frame_count;
        ce_ycbcr ycbcr;
        ce_mem_file* mem_file;
        ce_video_resource_vtable vtable;
        size_t size;
        void* impl;
    };

    extern const size_t CE_VIDEO_RESOURCE_BUILTIN_COUNT;
    extern const ce_video_resource_vtable ce_video_resource_builtins[];

    ce_video_resource* ce_video_resource_new(ce_mem_file* mem_file);
    void ce_video_resource_del(ce_video_resource* video_resource);

    inline bool ce_video_resource_read(ce_video_resource* video_resource)
    {
        return (*video_resource->vtable.read)(video_resource);
    }

    bool ce_video_resource_reset(ce_video_resource* video_resource);
}

#endif
