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

/*
 *  See doc/formats/mprfile.txt for more details.
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "cebyteorder.h"
#include "cemprfile.h"

#include "cereshlp.h"

static const unsigned int MP_SIGNATURE = 0xce4af672;
static const unsigned int SEC_SIGNATURE = 0xcf4bf774;

static void read_vertex(ce_mprvertex* ver, ce_mem_file* mem)
{
	ce_mem_file_read(mem, &ver->offset_x, sizeof(int8_t), 1);
	ce_mem_file_read(mem, &ver->offset_z, sizeof(int8_t), 1);
	ce_mem_file_read(mem, &ver->coord_y, sizeof(uint16_t), 1);
	ce_mem_file_read(mem, &ver->normal, sizeof(uint32_t), 1);
	ce_le2cpu16s(&ver->coord_y);
	ce_le2cpu32s(&ver->normal);
}

static void read_sectors(ce_mprfile* mpr, ce_res_file* res)
{
	mpr->sectors = ce_alloc_zero(sizeof(ce_mprsector) *
								mpr->sector_x_count * mpr->sector_z_count);

	// mpr name + xxxzzz.sec
	char sec_name[mpr->name->length + 3 + 3 + 4 + 1];

	for (int z = 0, z_count = mpr->sector_z_count; z < z_count; ++z) {
		for (int x = 0, x_count = mpr->sector_x_count; x < x_count; ++x) {
			snprintf(sec_name, sizeof(sec_name),
				"%s%03d%03d.sec", mpr->name->str, x, z);

			ce_mprsector* sec = mpr->sectors + z * x_count + x;
			ce_mem_file* mem = ce_reshlp_extract_mem_file_by_name(res, sec_name);

			uint32_t signature;
			ce_mem_file_read(mem, &signature, sizeof(uint32_t), 1);

			ce_le2cpu32s(&signature);
			assert(SEC_SIGNATURE == signature && "wrong signature");

			ce_mem_file_read(mem, &sec->water, sizeof(uint8_t), 1);

			sec->land_vertices = ce_alloc(sizeof(ce_mprvertex) *
											CE_MPRFILE_VERTEX_COUNT);

			for (unsigned int i = 0; i < CE_MPRFILE_VERTEX_COUNT; ++i) {
				read_vertex(sec->land_vertices + i, mem);
			}

			if (0 != sec->water) {
				sec->water_vertices = ce_alloc(sizeof(ce_mprvertex) *
												CE_MPRFILE_VERTEX_COUNT);

				for (unsigned int i = 0; i < CE_MPRFILE_VERTEX_COUNT; ++i) {
					read_vertex(sec->water_vertices + i, mem);
				}
			}

			sec->land_textures = ce_alloc(sizeof(uint16_t) * CE_MPRFILE_TEXTURE_COUNT);

			ce_mem_file_read(mem, sec->land_textures,
				sizeof(uint16_t), CE_MPRFILE_TEXTURE_COUNT);

			for (unsigned int i = 0; i < CE_MPRFILE_TEXTURE_COUNT; ++i) {
				ce_le2cpu16s(sec->land_textures + i);
			}

			if (0 != sec->water) {
				sec->water_textures = ce_alloc(sizeof(uint16_t) * CE_MPRFILE_TEXTURE_COUNT);
				sec->water_allow = ce_alloc(sizeof(int16_t) * CE_MPRFILE_TEXTURE_COUNT);

				ce_mem_file_read(mem, sec->water_textures,
					sizeof(uint16_t), CE_MPRFILE_TEXTURE_COUNT);
				ce_mem_file_read(mem, sec->water_allow,
					sizeof(int16_t), CE_MPRFILE_TEXTURE_COUNT);

				for (unsigned int i = 0; i < CE_MPRFILE_TEXTURE_COUNT; ++i) {
					ce_le2cpu16s(sec->water_textures + i);
					ce_le2cpu16s((uint16_t*)(sec->water_allow + i));
				}
			}

			ce_mem_file_del(mem);
		}
	}
}

ce_mprfile* ce_mprfile_open(ce_res_file* res_file)
{
	ce_mprfile* mprfile = ce_alloc_zero(sizeof(ce_mprfile));

	// mpr name = res name without extension (.mpr)
	mprfile->name = ce_string_dup_n(res_file->name, res_file->name->length - 4);

	// mpr name + .mp
	char mp_name[mprfile->name->length + 3 + 1];
	snprintf(mp_name, sizeof(mp_name), "%s.mp", mprfile->name->str);

	int mp_index = ce_res_file_node_index(res_file, mp_name);
	mprfile->size = ce_res_file_node_size(res_file, mp_index);
	mprfile->data = ce_res_file_node_data(res_file, mp_index);

	union {
		float* f;
		uint16_t* u16;
		uint32_t* u32;
	} ptr = { mprfile->data };

	uint32_t signature = ce_le2cpu32(*ptr.u32++);
	assert(MP_SIGNATURE == signature && "wrong signature");
	ce_unused(signature);

	mprfile->max_y = *ptr.f++;
	mprfile->sector_x_count = ce_le2cpu32(*ptr.u32++);
	mprfile->sector_z_count = ce_le2cpu32(*ptr.u32++);
	mprfile->texture_count = ce_le2cpu32(*ptr.u32++);
	mprfile->texture_size = ce_le2cpu32(*ptr.u32++);
	mprfile->tile_count = ce_le2cpu32(*ptr.u32++);
	mprfile->tile_size = ce_le2cpu32(*ptr.u32++);
	mprfile->material_count = ce_le2cpu16(*ptr.u16++);
	mprfile->anim_tile_count = ce_le2cpu32(*ptr.u32++);

	for (int i = 0; i < mprfile->material_count; ++i, ptr.f += 10) {
		int type = ce_le2cpu32(*ptr.u32++);
		mprfile->materials[1 != type] = ptr.f;
	}

	mprfile->tiles = ptr.u32; ptr.u32 += mprfile->tile_count;
	mprfile->anim_tiles = ptr.u16;

	for (int i = 0; i < mprfile->tile_count; ++i) {
		ce_le2cpu32s(mprfile->tiles + i);
	}

	for (int i = 0, n = 2 * mprfile->anim_tile_count; i < n; ++i) {
		ce_le2cpu16s(mprfile->anim_tiles + i);
	}

	read_sectors(mprfile, res_file);

	return mprfile;
}

void ce_mprfile_close(ce_mprfile* mprfile)
{
	if (NULL != mprfile) {
		ce_free(mprfile->data, mprfile->size);
		for (int i = 0, n = mprfile->sector_x_count *
							mprfile->sector_z_count; i < n; ++i) {
			ce_mprsector* sector = mprfile->sectors + i;
			ce_free(sector->water_allow,
					sizeof(int16_t) * CE_MPRFILE_TEXTURE_COUNT);
			ce_free(sector->water_textures,
					sizeof(uint16_t) * CE_MPRFILE_TEXTURE_COUNT);
			ce_free(sector->land_textures,
					sizeof(uint16_t) * CE_MPRFILE_TEXTURE_COUNT);
			ce_free(sector->water_vertices,
					sizeof(ce_mprvertex) * CE_MPRFILE_VERTEX_COUNT);
			ce_free(sector->land_vertices,
					sizeof(ce_mprvertex) * CE_MPRFILE_VERTEX_COUNT);
		}
		ce_free(mprfile->sectors, sizeof(ce_mprsector) *
				mprfile->sector_x_count * mprfile->sector_z_count);
		ce_string_del(mprfile->name);
		ce_free(mprfile, sizeof(ce_mprfile));
	}
}
