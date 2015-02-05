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

#ifndef CE_TERRAIN_HPP
#define CE_TERRAIN_HPP

#include "vector3.hpp"
#include "quaternion.hpp"
#include "vector.hpp"
#include "string.hpp"
#include "thread.hpp"
#include "mprfile.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "scenenode.hpp"
#include "renderqueue.hpp"

namespace cursedearth
{
    typedef struct ce_terrain ce_terrain;

    typedef struct {
        bool water;
        int x, z;
        ce_string* name;
        ce_mmpfile* mmpfile;
        ce_texture* texture;
        ce_renderlayer* renderlayer;
        ce_renderitem* renderitem;
        ce_scenenode* scenenode;
        ce_terrain* terrain;
    } ce_terrain_sector;

    ce_terrain_sector* ce_terrain_sector_new(ce_terrain* terrain, const char* name, int x, int z, bool water);
    void ce_terrain_sector_del(ce_terrain_sector* sector);

    struct ce_terrain {
        size_t completed_job_count;
        size_t queued_job_count;
        ce_mprfile* mprfile;
        ce_material* materials[CE_MPRFILE_MATERIAL_COUNT];
        ce_rendergroup* rendergroups[CE_MPRFILE_MATERIAL_COUNT];
        ce_vector* tile_mmpfiles;
        ce_vector* tile_textures;
        ce_once* tile_once;
        ce_vector* sectors;
        ce_scenenode* scenenode;
    };

    // terrain takes ownership of the mprfile
    ce_terrain* ce_terrain_new(ce_mprfile* mprfile, ce_renderqueue* renderqueue, const ce_vec3* position, const ce_quat* orientation, ce_scenenode* scenenode);
    void ce_terrain_del(ce_terrain* terrain);

    ce_scenenode* ce_terrain_find_scenenode(ce_terrain* terrain, float x, float z);
}

#endif
