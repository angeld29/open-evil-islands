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

#ifndef CE_MPRHELPER_HPP
#define CE_MPRHELPER_HPP

#include <cstdint>

#include "vec3.hpp"
#include "aabb.hpp"
#include "vector.hpp"
#include "thread.hpp"
#include "mmpfile.hpp"
#include "mprfile.hpp"
#include "material.hpp"

namespace cursedearth
{
    enum {
        CE_MPR_TEXTURE_VERSION = 1
    };

    const float CE_MPR_OFFSET_XZ_COEF;
    const float CE_MPR_HEIGHT_Y_COEF;

    ce_aabb* ce_mpr_get_aabb(ce_aabb* aabb, const ce_mprfile* mprfile, int sector_x, int sector_z, bool water);

    inline ce_vec3* ce_mpr_unpack_normal(ce_vec3* normal, uint32_t value)
    {
        return ce_vec3_init(normal, (((value >> 11) & 0x7ff) - 1000.0f) / 1000.0f, (value >> 22) / 1000.0f, ((value & 0x7ff) - 1000.0f) / 1000.0f);
    }

    inline int ce_mpr_texture_index(uint16_t value)
    {
        return value & 0x003f;
    }

    inline int ce_mpr_texture_number(uint16_t value)
    {
        return (value & 0x3fc0) >> 6;
    }

    inline int ce_mpr_texture_angle(uint16_t value)
    {
        return (value & 0xc000) >> 14;
    }

    float ce_mpr_get_height(const ce_mprfile* mprfile, const ce_vec3* position);

    ce_material* ce_mpr_create_material(const ce_mprfile* mprfile, bool water);
    ce_mmpfile* ce_mpr_generate_texture(const ce_mprfile* mprfile, const ce_vector* tile_mmp_files, int x, int z, bool water);
}

#endif
