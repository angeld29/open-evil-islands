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

#ifndef CE_RENDERITEM_HPP
#define CE_RENDERITEM_HPP

#include <cstddef>
#include <cstdarg>

#include "vec3.hpp"
#include "quat.hpp"
#include "aabb.hpp"
#include "bbox.hpp"

namespace cursedearth
{
    typedef struct ce_renderitem ce_renderitem;

    typedef struct {
        void (*ctor)(ce_renderitem* renderitem, va_list args);
        void (*dtor)(ce_renderitem* renderitem);
        void (*update)(ce_renderitem* renderitem, va_list args);
        void (*render)(ce_renderitem* renderitem);
        void (*clone)(const ce_renderitem* renderitem, ce_renderitem* clone_renderitem);
    } ce_renderitem_vtable;

    struct ce_renderitem {
        bool visible;
        ce_aabb aabb;
        ce_vec3 position;
        ce_quat orientation;
        ce_bbox bbox;
        ce_vec3 world_position;
        ce_quat world_orientation;
        ce_bbox world_bbox;
        ce_renderitem_vtable vtable;
        size_t size;
        void* impl;
    };

    ce_renderitem* ce_renderitem_new(ce_renderitem_vtable vtable, size_t size, ...);
    void ce_renderitem_del(ce_renderitem* renderitem);

    void ce_renderitem_update(ce_renderitem* renderitem, ...);
    void ce_renderitem_render(ce_renderitem* renderitem);

    ce_renderitem* ce_renderitem_clone(const ce_renderitem* renderitem);
}

#endif
