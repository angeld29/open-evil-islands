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

#ifndef CE_RENDERLAYER_HPP
#define CE_RENDERLAYER_HPP

#include "vector.hpp"
#include "texture.hpp"
#include "renderitem.hpp"

namespace cursedearth
{
    typedef struct {
        ce_texture* texture;
        ce_vector* renderitems;
    } ce_renderlayer;

    ce_renderlayer* ce_renderlayer_new(ce_texture* texture);
    void ce_renderlayer_del(ce_renderlayer* renderlayer);

    void ce_renderlayer_clear(ce_renderlayer* renderlayer);

    void ce_renderlayer_add(ce_renderlayer* renderlayer, ce_renderitem* renderitem);

    void ce_renderlayer_render(ce_renderlayer* renderlayer);
}

#endif
