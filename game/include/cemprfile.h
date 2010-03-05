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

#ifndef CE_MPRFILE_H
#define CE_MPRFILE_H

#include <stdint.h>

#include "ceresfile.h"
#include "cefrustum.h"
#include "cetexture.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

enum {
	CE_MPRFILE_MATERIAL_GROUND = 1,
	CE_MPRFILE_MATERIAL_WATER = 3
};

enum {
	CE_MPRFILE_VERTEX_SIDE = 33,
	CE_MPRFILE_VERTEX_COUNT = 33 * 33
};

enum {
	CE_MPRFILE_TEXTURE_SIDE = 16,
	CE_MPRFILE_TEXTURE_COUNT = 16 * 16
};

typedef struct {
	uint32_t type;
	float color[4];
	float selfillum;
	float wavemult;
	float unknown[4];
} ce_mprfile_material;

typedef struct {
	uint16_t index;
	uint16_t count;
} ce_mprfile_anim_tile;

typedef struct {
	int8_t offset_x;
	int8_t offset_z;
	uint16_t coord_y;
	uint32_t normal;
} ce_mprfile_vertex;

typedef struct {
	unsigned int x, z;
	ce_aabb bounding_box;
	float dist2; // for sorting on rendering
	uint8_t water;
	ce_mprfile_vertex* land_vertices;
	ce_mprfile_vertex* water_vertices;
	uint16_t* land_textures;
	uint16_t* water_textures;
	int16_t* water_allow;
} ce_mprfile_sector;

typedef struct {
	float max_y;
	uint32_t sector_x_count;
	uint32_t sector_z_count;
	unsigned int sector_count;
	uint32_t texture_count;
	uint32_t texture_size;
	uint32_t tile_count;
	uint32_t tile_size;
	uint16_t material_count;
	uint32_t anim_tile_count;
	ce_mprfile_material* materials;
	uint32_t* tiles; // TODO: id for bind sound ?
	ce_mprfile_anim_tile* anim_tiles;
	ce_mprfile_sector* sectors;
	unsigned int visible_sector_count;
	ce_mprfile_sector** visible_sectors;
	ce_texture** textures;
} ce_mprfile;

extern ce_mprfile* ce_mprfile_open(ce_resfile* mpr_res, ce_resfile* textures_res);
extern void ce_mprfile_close(ce_mprfile* mpr);

extern float ce_mprfile_get_max_height(const ce_mprfile* mpr);

extern void ce_mprfile_apply_frustum(ce_mprfile* mpr, const ce_vec3* eye,
														const ce_frustum* f);

extern void ce_mprfile_render(ce_mprfile* mpr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MPRFILE_H */
