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

#include <memory>

#include <boost/noncopyable.hpp>

#include "vec3.hpp"
#include "quat.hpp"
#include "aabb.hpp"
#include "bbox.hpp"

namespace cursedearth
{
    typedef std::shared_ptr<class render_item_t> render_item_ptr_t;
    typedef std::shared_ptr<const class render_item_t> render_item_const_ptr_t;

    class render_item_t: boost::noncopyable
    {
    public:
        virtual ~render_item_t() = default;

        virtual void update() = 0;
        virtual void render() = 0;

        virtual render_item_ptr_t clone() const = 0;

    protected:
        bool visible = true;
        aabb_t aabb;
        vec3_t position;
        ce_quat orientation;
        bbox_t bbox;
        vec3_t world_position;
        ce_quat world_orientation;
        bbox_t world_bbox;
    };
}

#endif /* CE_RENDERITEM_HPP */
