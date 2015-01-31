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

#ifndef CE_TRANSFORM_HPP
#define CE_TRANSFORM_HPP

#include "vec3.hpp"
#include "quat.hpp"

namespace cursedearth
{
    typedef struct {
        ce_vec3 translation;
        ce_quat rotation;
        ce_vec3 scaling;
    } ce_transform;

    extern const ce_transform CE_TRANSFORM_IDENTITY;

    inline ce_transform* ce_transform_init(ce_transform* transform, const ce_vec3* translation, const ce_quat* rotation, const ce_vec3* scaling)
    {
        ce_vec3_copy(&transform->translation, translation);
        ce_quat_copy(&transform->rotation, rotation);
        ce_vec3_copy(&transform->scaling, scaling);
        return transform;
    }

    inline ce_transform* ce_transform_init_identity(ce_transform* transform)
    {
        ce_vec3_init_zero(&transform->translation);
        ce_quat_init_identity(&transform->rotation);
        ce_vec3_init_unit_scale(&transform->scaling);
        return transform;
    }

    inline ce_transform* ce_transform_copy(ce_transform* transform, const ce_transform* other)
    {
        ce_vec3_copy(&transform->translation, &other->translation);
        ce_quat_copy(&transform->rotation, &other->rotation);
        ce_vec3_copy(&transform->scaling, &other->scaling);
        return transform;
    }
}

#endif /* CE_TRANSFORM_HPP */
