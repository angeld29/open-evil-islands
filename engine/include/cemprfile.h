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

#include <stdbool.h>
#include <stdint.h>

#include "cestring.h"
#include "ceresfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

enum {
	CE_MPRFILE_VERTEX_SIDE = 33,
	CE_MPRFILE_VERTEX_COUNT = 33 * 33
};

enum {
	CE_MPRFILE_TEXTURE_SIDE = 16,
	CE_MPRFILE_TEXTURE_COUNT = 16 * 16
};

typedef struct {
	int8_t offset_x;
	int8_t offset_z;
	uint16_t coord_y;
	uint32_t normal;
} ce_mprfile_vertex;

typedef struct {
	uint8_t water;
	ce_mprfile_vertex* land_vertices;
	ce_mprfile_vertex* water_vertices;
	uint16_t* land_textures;
	uint16_t* water_textures;
	int16_t* water_allow;
} ce_mprfile_sector;

typedef struct {
	ce_string* name;
	float max_y;
	int sector_x_count;
	int sector_z_count;
	int texture_count;
	int texture_size;
	int tile_count;
	int tile_size;
	int material_count;
	int anim_tile_count;
	float* materials[2]; // land and water
	uint32_t* tiles;
	uint16_t* anim_tiles;
	ce_mprfile_sector* sectors;
	size_t size;
	void* data;
} ce_mprfile;

extern ce_mprfile* ce_mprfile_open(ce_resfile* resfile);
extern void ce_mprfile_close(ce_mprfile* mprfile);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MPRFILE_H */
