/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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
#include "cemprfile.h"
#include "cetexmng.h"
#include "cetexture.h"
#include "cematerial.h"
#include "cescenenode.h"
#include "cerenderqueue.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_mprfile* mprfile;
	ce_vector* tile_textures;
	ce_material* materials[CE_MPRFILE_MATERIAL_COUNT];
	ce_vector* sectors;
	ce_scenenode* scenenode;
} ce_terrain;

// terrain takes ownership of the mprfile
extern ce_terrain* ce_terrain_new(ce_mprfile* mprfile, bool tiling,
									ce_texmng* texmng, int thread_count,
									ce_renderqueue* renderqueue,
									const ce_vec3* position,
									const ce_quat* orientation,
									ce_scenenode* scenenode);
extern void ce_terrain_del(ce_terrain* terrain);

extern ce_scenenode* ce_terrain_find_scenenode(ce_terrain* terrain,
												float x, float z);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_TERRAIN_H */
