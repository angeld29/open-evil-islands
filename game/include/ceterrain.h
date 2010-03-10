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
#include "cerenderitem.h"
#include "cescenenode.h"
#include "cetexture.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_mprfile* mprfile;
	ce_texture* stub_texture;
	ce_vector* textures;
	ce_vector* renderitems;
	ce_scenenode* scenenode;
} ce_terrain;

extern ce_terrain* ce_terrain_new(const char* zone_name,
									const ce_vec3* position,
									const ce_quat* orientation,
									ce_scenenode* parent_scenenode);
extern void ce_terrain_del(ce_terrain* terrain);

extern bool ce_terrain_create_impl(ce_terrain* terrain);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_TERRAIN_H */
