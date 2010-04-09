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
#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "cemmphlp.h"
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

	const ce_mprsector* sector = mprfile->sectors +
		sector_z * mprfile->sector_x_count + sector_x;
	const ce_mprvertex* vertex = sector->land_vertices +
		vertex_z * CE_MPRFILE_VERTEX_SIDE + vertex_x;

	return mprfile->max_y / (UINT16_MAX - 0) * vertex->coord_y;
}

ce_material* ce_mprhlp_create_material(const ce_mprfile* mprfile, bool water)
{
	if (NULL == mprfile->materials[water]) {
		return NULL;
	}

	ce_material* material = ce_material_new();
	material->mode = CE_MATERIAL_MODE_DECAL;

	ce_color_init(&material->ambient, 0.5f, 0.5f, 0.5f, 1.0f);
	ce_color_init_array(&material->diffuse, mprfile->materials[water]);
	ce_color_init(&material->emission,
		mprfile->materials[water][4] * mprfile->materials[water][0],
		mprfile->materials[water][4] * mprfile->materials[water][1],
		mprfile->materials[water][4] * mprfile->materials[water][2],
		mprfile->materials[water][4] * mprfile->materials[water][3]);

	if (water) {
		material->blend = true;
	}

	return material;
}

static void ce_mprhlp_rotate0(uint32_t* restrict dst,
								const uint32_t* restrict src,
								int width, int height)
{
	memcpy(dst, src, sizeof(uint32_t) * width * height);
}

static void ce_mprhlp_rotate90(uint32_t* restrict dst,
								const uint32_t* restrict src,
								int width, int height)
{
	for (int i = 0; i < width; ++i) {
		for (int j = height - 1; j >= 0; --j) {
			*dst++ = src[j * width + i];
		}
	}
}

static void ce_mprhlp_rotate180(uint32_t* restrict dst,
								const uint32_t* restrict src,
								int width, int height)
{
	for (int i = width * height - 1; i >= 0; --i) {
		*dst++ = src[i];
	}
}

static void ce_mprhlp_rotate270(uint32_t* restrict dst,
								const uint32_t* restrict src,
								int width, int height)
{
	for (int i = width - 1; i >= 0; --i) {
		for (int j = 0; j < height; ++j) {
			*dst++ = src[j * width + i];
		}
	}
}

static void (*ce_mprhlp_rotations[])(uint32_t* restrict,
									const uint32_t* restrict,
									int, int) = {
	ce_mprhlp_rotate0, ce_mprhlp_rotate90,
	ce_mprhlp_rotate180, ce_mprhlp_rotate270
};

ce_mmpfile* ce_mprhlp_generate_mmpfile(const ce_mprfile* mprfile,
										int sector_x, int sector_z,
										ce_mmpfile* mmpfiles[])
{
	ce_mprsector* sector = mprfile->sectors + sector_z *
							mprfile->sector_x_count + sector_x;

	uint16_t* textures = sector->land_textures;

	int tile_size = mprfile->tile_size - 16; // minus borders
	int tile_size_sqr = tile_size * tile_size;

	uint32_t* tile = ce_alloc(sizeof(uint32_t) * tile_size_sqr);
	uint32_t* tile2 = ce_alloc(sizeof(uint32_t) * tile_size_sqr);

	ce_mmpfile* mmpfile = ce_mmpfile_new(tile_size * CE_MPRFILE_TEXTURE_SIDE,
										tile_size * CE_MPRFILE_TEXTURE_SIDE,
										mmpfiles[0]->mipmap_count,
										CE_MMPFILE_FORMAT_ARGB8);

	uint32_t* texels = mmpfile->texels;

	for (int i = 0; i < CE_MPRFILE_TEXTURE_SIDE; ++i) {
		for (int j = 0; j < CE_MPRFILE_TEXTURE_SIDE; ++j) {
			uint16_t texture = textures[i * CE_MPRFILE_TEXTURE_SIDE + j];

			ce_mmpfile* mmpfile2 = mmpfiles[ce_mprhlp_texture_number(texture)];
			uint32_t* texels2 = mmpfile2->texels;

			int texture_index = ce_mprhlp_texture_index(texture);
			int u = texture_index - texture_index / 8 * 8;
			int v = 7 - texture_index / 8;

			int p = v * mprfile->tile_size + 8; // skip border
			int q = u * mprfile->tile_size + 8; // skip border

			for (int k = 0; k < tile_size; ++k) {
				for (int l = 0; l < tile_size; ++l) {
					tile2[k * tile_size + l] =
						texels2[(p + k) * mmpfile2->width + (q + l)];
				}
			}

			(*ce_mprhlp_rotations[ce_mprhlp_texture_angle(texture)])
									(tile, tile2, tile_size, tile_size);

			int s = (CE_MPRFILE_TEXTURE_SIDE - 1 - i) * tile_size;
			int t = j * tile_size;

			for (int k = 0; k < tile_size; ++k) {
				for (int l = 0; l < tile_size; ++l) {
					texels[(s + k) * mmpfile->width + (t + l)] =
						tile[k * tile_size + l];
				}
			}
		}
	}

	ce_free(tile2, sizeof(uint32_t) * tile_size_sqr);
	ce_free(tile, sizeof(uint32_t) * tile_size_sqr);

	ce_mmphlp_rgba8_compress_dxt(mmpfile, CE_MMPFILE_FORMAT_DXT1);
	return mmpfile;
}
