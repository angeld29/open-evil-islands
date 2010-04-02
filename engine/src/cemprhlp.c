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

#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

#include "celib.h"
#include "cemprhlp.h"

ce_aabb* ce_mprhlp_get_aabb(ce_aabb* aabb,
							const ce_mprfile* mprfile,
							int sector_x, int sector_z)
{
	// FIXME: negative z in generic code?..
	ce_vec3 min, max;
	ce_vec3_init(&min, sector_x * (CE_MPRFILE_VERTEX_SIDE - 1), 0.0f, -1.0f *
		(sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) + (CE_MPRFILE_VERTEX_SIDE - 1)));
	ce_vec3_init(&max,
		sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) + (CE_MPRFILE_VERTEX_SIDE - 1),
		mprfile->max_y, -1.0f * (sector_z * (CE_MPRFILE_VERTEX_SIDE - 1)));

	aabb->radius = 0.5f * ce_vec3_dist(&min, &max);
	ce_vec3_mid(&aabb->origin, &min, &max);
	ce_vec3_sub(&aabb->extents, &max, &aabb->origin);

	return aabb;
}

const ce_mprfile_material*
ce_mprhlp_find_material(const ce_mprfile* mprfile,
						ce_mprfile_material_type type)
{
	for (int i = 0, n = mprfile->material_count; i < n; ++i) {
		ce_mprfile_material* material = mprfile->materials + i;
		if (type == material->type) {
			return material;
		}
	}
	return NULL;
}

float* ce_mprhlp_normal2vector(float* vector, uint32_t normal)
{
	vector[0] = (((normal >> 11) & 0x7ff) - 1000.0f) / 1000.0f;
	vector[1] = (normal >> 22) / 1000.0f;
	vector[2] = ((normal & 0x7ff) - 1000.0f) / 1000.0f;
	return vector;
}

int ce_mprhlp_texture_index(uint16_t texture)
{
	return texture & 0x003f;
}

int ce_mprhlp_texture_number(uint16_t texture)
{
	return (texture & 0x3fc0) >> 6;
}

int ce_mprhlp_texture_angle(uint16_t texture)
{
	return (texture & 0xc000) >> 14;
}

float ce_mprhlp_get_height(const ce_mprfile* mprfile, float x, float z)
{
	if (modff(x, &x) > 0.5f) {
		x += 1.0f;
	}

	if (modff(z, &z) > 0.5f) {
		z += 1.0f;
	}

	int sector_x = (int)x / (CE_MPRFILE_VERTEX_SIDE - 1);
	int sector_z = (int)z / (CE_MPRFILE_VERTEX_SIDE - 1);
	int vertex_x = (int)x % (CE_MPRFILE_VERTEX_SIDE - 1);
	int vertex_z = (int)z % (CE_MPRFILE_VERTEX_SIDE - 1);

	const ce_mprfile_sector* sector = mprfile->sectors +
								sector_z * mprfile->sector_x_count + sector_x;
	const ce_mprfile_vertex* vertex = sector->land_vertices +
								vertex_z * CE_MPRFILE_VERTEX_SIDE + vertex_x;

	return mprfile->max_y / (UINT16_MAX - 0) * vertex->coord_y;
}

ce_material* ce_mprhlp_create_material(const ce_mprfile* mprfile,
										bool opacity, ce_texture* texture)
{
	ce_material* material = ce_material_new(texture);
	material->mode = CE_MATERIAL_MODE_DECAL;
	material->wrap = CE_MATERIAL_WRAP_CLAMP_TO_EDGE;

	const ce_mprfile_material* mpr_material =
		ce_mprhlp_find_material(mprfile, opacity ?
										CE_MPRFILE_MATERIAL_TYPE_GROUND :
										CE_MPRFILE_MATERIAL_TYPE_WATER);
	assert(NULL != mpr_material);

	ce_color_init(&material->ambient, 0.5f, 0.5f, 0.5f, 1.0f);
	ce_color_init_array(&material->diffuse, mpr_material->color);
	ce_color_init(&material->emission,
		mpr_material->selfillum * mpr_material->color[0],
		mpr_material->selfillum * mpr_material->color[1],
		mpr_material->selfillum * mpr_material->color[2],
		mpr_material->selfillum * mpr_material->color[3]);

	return material;
}
