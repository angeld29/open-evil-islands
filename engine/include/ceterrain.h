/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#ifndef CE_TERRAIN_H
#define CE_TERRAIN_H

#include <stdbool.h>

#include "cevec3.h"
#include "cequat.h"
#include "cevector.h"
#include "cestring.h"
#include "cethread.h"
#include "cemprfile.h"
#include "cetexture.h"
#include "cematerial.h"
#include "cescenenode.h"
#include "cerenderqueue.h"

#ifdef __cplusplus
extern "C" {
#endif

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

extern ce_terrain_sector* ce_terrain_sector_new(ce_terrain* terrain,
                                                const char* name,
                                                int x, int z, bool water);
extern void ce_terrain_sector_del(ce_terrain_sector* sector);

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
extern ce_terrain* ce_terrain_new(ce_mprfile* mprfile,
                                    ce_renderqueue* renderqueue,
                                    const ce_vec3* position,
                                    const ce_quat* orientation,
                                    ce_scenenode* scenenode);
extern void ce_terrain_del(ce_terrain* terrain);

extern ce_scenenode* ce_terrain_find_scenenode(ce_terrain* terrain,
                                                float x, float z);

#ifdef __cplusplus
}
#endif

#endif /* CE_TERRAIN_H */
