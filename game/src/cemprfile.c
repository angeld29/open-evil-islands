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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "cestr.h"
#include "cebyteorder.h"
#include "celogging.h"
#include "cealloc.h"
#include "cereshlp.h"
#include "cemprfile.h"

static const unsigned int MP_SIGNATURE = 0xce4af672;
static const unsigned int SEC_SIGNATURE = 0xcf4bf774;

static bool read_material(ce_mprfile_material* mat, ce_memfile* mem)
{
	if (1 != ce_memfile_read(mem, &mat->type, sizeof(uint32_t), 1) ||
			4 != ce_memfile_read(mem, mat->color, sizeof(float), 4) ||
			1 != ce_memfile_read(mem, &mat->selfillum, sizeof(float), 1) ||
			1 != ce_memfile_read(mem, &mat->wavemult, sizeof(float), 1) ||
			4 != ce_memfile_read(mem, mat->unknown, sizeof(float), 4)) {
		ce_logging_error("mprfile: io error occured");
		return false;
	}
	ce_le2cpu32s(&mat->type);
	return true;
}

static bool read_anim_tile(ce_mprfile_anim_tile* at, ce_memfile* mem)
{
	if (1 != ce_memfile_read(mem, &at->index, sizeof(uint16_t), 1) ||
			1 != ce_memfile_read(mem, &at->count, sizeof(uint16_t), 1)) {
		ce_logging_error("mprfile: io error occured");
		return false;
	}
	ce_le2cpu16s(&at->index);
	ce_le2cpu16s(&at->count);
	return true;
}

static bool read_vertex(ce_mprfile_vertex* ver, ce_memfile* mem)
{
	if (1 != ce_memfile_read(mem, &ver->offset_x, sizeof(int8_t), 1) ||
			1 != ce_memfile_read(mem, &ver->offset_z, sizeof(int8_t), 1) ||
			1 != ce_memfile_read(mem, &ver->coord_y, sizeof(uint16_t), 1) ||
			1 != ce_memfile_read(mem, &ver->normal, sizeof(uint32_t), 1)) {
		ce_logging_error("mprfile: io error occured");
		return false;
	}
	ce_le2cpu16s(&ver->coord_y);
	ce_le2cpu32s(&ver->normal);
	return true;
}

static bool read_sector_impl(ce_mprfile_sector* sec, ce_memfile* mem)
{
	uint32_t signature;
	if (1 != ce_memfile_read(mem, &signature, sizeof(uint32_t), 1)) {
		ce_logging_error("mprfile: io error occured");
		return false;
	}

	ce_le2cpu32s(&signature);
	if (SEC_SIGNATURE != signature) {
		ce_logging_error("mprfile: wrong sec signature");
		return false;
	}

	if (1 != ce_memfile_read(mem, &sec->water, sizeof(uint8_t), 1)) {
		ce_logging_error("mprfile: io error occured");
		return false;
	}

	if (NULL == (sec->land_vertices = ce_alloc(sizeof(ce_mprfile_vertex) *
												CE_MPRFILE_VERTEX_COUNT))) {
		ce_logging_error("mprfile: could not allocate memory");
		return false;
	}

	for (unsigned int i = 0; i < CE_MPRFILE_VERTEX_COUNT; ++i) {
		if (!read_vertex(sec->land_vertices + i, mem)) {
			return false;
		}
	}

	if (0 != sec->water) {
		if (NULL == (sec->water_vertices = ce_alloc(sizeof(ce_mprfile_vertex) *
													CE_MPRFILE_VERTEX_COUNT))) {
			ce_logging_error("mprfile: could not allocate memory");
			return false;
		}

		for (unsigned int i = 0; i < CE_MPRFILE_VERTEX_COUNT; ++i) {
			if (!read_vertex(sec->water_vertices + i, mem)) {
				return false;
			}
		}
	}

	if (NULL == (sec->land_textures = ce_alloc(sizeof(uint16_t) *
												CE_MPRFILE_TEXTURE_COUNT))) {
		ce_logging_error("mprfile: could not allocate memory");
		return false;
	}

	if (CE_MPRFILE_TEXTURE_COUNT != ce_memfile_read(mem, sec->land_textures,
								sizeof(uint16_t), CE_MPRFILE_TEXTURE_COUNT)) {
		ce_logging_error("mprfile: io error occured");
		return false;
	}

	for (unsigned int i = 0; i < CE_MPRFILE_TEXTURE_COUNT; ++i) {
		ce_le2cpu16s(sec->land_textures + i);
	}

	if (0 != sec->water) {
		sec->water_textures = ce_alloc(sizeof(uint16_t) * CE_MPRFILE_TEXTURE_COUNT);
		sec->water_allow = ce_alloc(sizeof(int16_t) * CE_MPRFILE_TEXTURE_COUNT);

		if (NULL == sec->water_textures || NULL == sec->water_allow) {
			ce_logging_error("mprfile: could not allocate memory");
			return false;
		}

		if (CE_MPRFILE_TEXTURE_COUNT != ce_memfile_read(mem, sec->water_textures,
									sizeof(uint16_t), CE_MPRFILE_TEXTURE_COUNT) ||
				CE_MPRFILE_TEXTURE_COUNT != ce_memfile_read(mem, sec->water_allow,
									sizeof(int16_t), CE_MPRFILE_TEXTURE_COUNT)) {
			ce_logging_error("mprfile: io error occured");
			return false;
		}

		for (unsigned int i = 0; i < CE_MPRFILE_TEXTURE_COUNT; ++i) {
			ce_le2cpu16s(sec->water_textures + i);
			ce_le2cpu16s((uint16_t*)(sec->water_allow + i));
		}
	}

	return true;
}

static bool read_sector(ce_mprfile_sector* sec, const char* name, ce_resfile* res)
{
	ce_memfile* memfile = ce_reshlp_extract_memfile_by_name(res, name);
	if (NULL == memfile) {
		return false;
	}

	bool ok = read_sector_impl(sec, memfile);
	return ce_memfile_close(memfile), ok;
}

static bool read_sectors(ce_mprfile* mpr, ce_resfile* res)
{
	if (NULL == (mpr->sectors = ce_alloc_zero(sizeof(ce_mprfile_sector) *
								mpr->sector_x_count * mpr->sector_z_count))) {
		ce_logging_error("mprfile: could not allocate memory");
		return false;
	}

	// mpr name + xxxzzz.sec
	char sec_name[mpr->name->length + 3 + 3 + 4 + 1];

	for (int z = 0, z_count = mpr->sector_z_count; z < z_count; ++z) {
		for (int x = 0, x_count = mpr->sector_x_count; x < x_count; ++x) {
			int i = z * x_count + x;

			snprintf(sec_name, sizeof(sec_name),
				"%s%03d%03d.sec", mpr->name->str, x, z);

			ce_mprfile_sector* sec = mpr->sectors + i;

			if (!read_sector(sec, sec_name, res)) {
				return false;
			}
		}
	}

	return true;
}

static bool read_header_impl(ce_mprfile* mpr, ce_memfile* mem)
{
	uint32_t signature;
	if (1 != ce_memfile_read(mem, &signature, sizeof(uint32_t), 1)) {
		ce_logging_error("mprfile: io error occured");
		return false;
	}

	ce_le2cpu32s(&signature);
	if (MP_SIGNATURE != signature) {
		ce_logging_error("mprfile: wrong mp signature");
		return false;
	}

	if (1 != ce_memfile_read(mem, &mpr->max_y, sizeof(float), 1) ||
			1 != ce_memfile_read(mem, &mpr->sector_x_count, sizeof(uint32_t), 1) ||
			1 != ce_memfile_read(mem, &mpr->sector_z_count, sizeof(uint32_t), 1) ||
			1 != ce_memfile_read(mem, &mpr->texture_count, sizeof(uint32_t), 1) ||
			1 != ce_memfile_read(mem, &mpr->texture_size, sizeof(uint32_t), 1) ||
			1 != ce_memfile_read(mem, &mpr->tile_count, sizeof(uint32_t), 1) ||
			1 != ce_memfile_read(mem, &mpr->tile_size, sizeof(uint32_t), 1) ||
			1 != ce_memfile_read(mem, &mpr->material_count, sizeof(uint16_t), 1) ||
			1 != ce_memfile_read(mem, &mpr->anim_tile_count, sizeof(uint32_t), 1)) {
		ce_logging_error("mprfile: io error occured");
		return false;
	}

	ce_le2cpu32s(&mpr->sector_x_count);
	ce_le2cpu32s(&mpr->sector_z_count);
	ce_le2cpu32s(&mpr->texture_count);
	ce_le2cpu32s(&mpr->texture_size);
	ce_le2cpu32s(&mpr->tile_count);
	ce_le2cpu32s(&mpr->tile_size);
	ce_le2cpu16s(&mpr->material_count);
	ce_le2cpu32s(&mpr->anim_tile_count);

	if (NULL == (mpr->materials = ce_alloc(sizeof(ce_mprfile_material) *
											mpr->material_count))) {
		ce_logging_error("mprfile: could not allocate memory");
		return false;
	}

	for (unsigned int i = 0; i < mpr->material_count; ++i) {
		if (!read_material(mpr->materials + i, mem)) {
			return false;
		}
	}

	if (NULL == (mpr->tiles = ce_alloc(sizeof(uint32_t) * mpr->tile_count))) {
		ce_logging_error("mprfile: could not allocate memory");
		return false;
	}

	if (mpr->tile_count != ce_memfile_read(mem, mpr->tiles,
							sizeof(uint32_t), mpr->tile_count)) {
		ce_logging_error("mprfile: io error occured");
		return false;
	}

	for (unsigned int i = 0; i < mpr->tile_count; ++i) {
		ce_le2cpu32s(mpr->tiles + i);
	}

	if (NULL == (mpr->anim_tiles = ce_alloc(sizeof(ce_mprfile_anim_tile) *
											mpr->anim_tile_count))) {
		ce_logging_error("mprfile: could not allocate memory");
		return false;
	}

	for (unsigned int i = 0; i < mpr->anim_tile_count; ++i) {
		if (!read_anim_tile(mpr->anim_tiles + i, mem)) {
			return false;
		}
	}

	return true;
}

static bool read_header(ce_mprfile* mpr, ce_resfile* res)
{
	// mpr name - res name without extension (.mpr)
	mpr->name = ce_string_dup_n(res->name, res->name->length - 4);

	// mpr name + .mp
	char mp_name[mpr->name->length + 3 + 1];
	snprintf(mp_name, sizeof(mp_name), "%s.mp", mpr->name->str);

	ce_memfile* memfile = ce_reshlp_extract_memfile_by_name(res, mp_name);
	if (NULL == memfile) {
		return false;
	}

	bool ok = read_header_impl(mpr, memfile);
	return ce_memfile_close(memfile), ok;
}

ce_mprfile* ce_mprfile_open(ce_resfile* res)
{
	ce_mprfile* mpr = ce_alloc_zero(sizeof(ce_mprfile));
	if (NULL == mpr) {
		ce_logging_error("mprfile: could not allocate memory");
		return NULL;
	}

	if (!read_header(mpr, res) || !read_sectors(mpr, res)) {
		ce_mprfile_close(mpr);
		return NULL;
	}

	return mpr;
}

void ce_mprfile_close(ce_mprfile* mpr)
{
	if (NULL != mpr) {
		if (NULL != mpr->sectors) {
			for (int i = 0, n = mpr->sector_x_count *
								mpr->sector_z_count; i < n; ++i) {
				ce_mprfile_sector* sector = mpr->sectors + i;
				ce_free(sector->water_allow,
						sizeof(int16_t) * CE_MPRFILE_TEXTURE_COUNT);
				ce_free(sector->water_textures,
						sizeof(uint16_t) * CE_MPRFILE_TEXTURE_COUNT);
				ce_free(sector->land_textures,
						sizeof(uint16_t) * CE_MPRFILE_TEXTURE_COUNT);
				ce_free(sector->water_vertices,
						sizeof(ce_mprfile_vertex) * CE_MPRFILE_VERTEX_COUNT);
				ce_free(sector->land_vertices,
						sizeof(ce_mprfile_vertex) * CE_MPRFILE_VERTEX_COUNT);
			}
		}
		ce_free(mpr->sectors, sizeof(ce_mprfile_sector) *
							mpr->sector_x_count * mpr->sector_z_count);
		ce_free(mpr->anim_tiles,
				sizeof(ce_mprfile_anim_tile) * mpr->anim_tile_count);
		ce_free(mpr->tiles, sizeof(uint32_t) * mpr->tile_count);
		ce_free(mpr->materials,
				sizeof(ce_mprfile_material) * mpr->material_count);
		ce_string_del(mpr->name);
		ce_free(mpr, sizeof(ce_mprfile));
	}
}
