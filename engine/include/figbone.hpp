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

#include "vector3.hpp"
#include "quaternion.hpp"
#include "bbox.hpp"
#include "vector.hpp"
#include "anmstate.hpp"
#include "fignode.hpp"

namespace cursedearth
{
    typedef struct ce_figbone ce_figbone;

    struct ce_figbone {
        vector3_t position;
        quaternion_t orientation;
        vector3_t bone_position;
        quaternion_t bone_orientation;
        ce_anmstate* anmstate;
        ce_figbone* parent;
        ce_vector* childs;
    };

    ce_figbone* ce_figbone_new(const ce_fignode* fignode, const complection_t* complection, ce_figbone* parent);
    void ce_figbone_del(ce_figbone* figbone);

    void ce_figbone_advance(ce_figbone* figbone, float distance);
    void ce_figbone_update(ce_figbone* figbone, const ce_fignode* fignode, ce_vector* renderitems);

    bool ce_figbone_play_animation(ce_figbone* figbone, const ce_fignode* fignode, const char* name);
    void ce_figbone_stop_animation(ce_figbone* figbone, const ce_fignode* fignode);
}

#endif
