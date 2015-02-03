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

#ifndef CE_FIGBONE_HPP
#define CE_FIGBONE_HPP

#include "vec3.hpp"
#include "quat.hpp"
#include "bbox.hpp"
#include "vector.hpp"
#include "anmstate.hpp"
#include "fignode.hpp"

namespace cursedearth
{
    typedef struct ce_figbone ce_figbone;

    struct ce_figbone {
        ce_vec3 position;
        ce_quat orientation;
        ce_vec3 bone_position;
        ce_quat bone_orientation;
        ce_anmstate* anmstate;
        ce_figbone* parent;
        ce_vector* childs;
    };

    extern ce_figbone* ce_figbone_new(const ce_fignode* fignode, const ce_complection* complection, ce_figbone* parent);
    extern void ce_figbone_del(ce_figbone* figbone);

    extern void ce_figbone_advance(ce_figbone* figbone, float distance);
    extern void ce_figbone_update(ce_figbone* figbone, const ce_fignode* fignode, ce_vector* renderitems);

    extern bool ce_figbone_play_animation(ce_figbone* figbone, const ce_fignode* fignode, const char* name);
    extern void ce_figbone_stop_animation(ce_figbone* figbone, const ce_fignode* fignode);
}

#endif
