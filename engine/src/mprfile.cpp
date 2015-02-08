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

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include "alloc.hpp"
#include "byteorder.hpp"
#include "resball.hpp"
#include "mprfile.hpp"

namespace cursedearth
{
    const unsigned int MP_SIGNATURE = 0xce4af672;
    const unsigned int SEC_SIGNATURE = 0xcf4bf774;

    void read_vertex(ce_mprvertex* ver, ce_mem_file* mem)
    {
        ce_mem_file_read(mem, &ver->offset_x, sizeof(int8_t), 1);
        ce_mem_file_read(mem, &ver->offset_z, sizeof(int8_t), 1);
        ce_mem_file_read(mem, &ver->coord_y, sizeof(uint16_t), 1);
        ce_mem_file_read(mem, &ver->normal, sizeof(uint32_t), 1);
        ver->coord_y = le2cpu(ver->coord_y);
        ver->normal = le2cpu(ver->normal);
    }

    void read_sectors(ce_mprfile* mpr, ce_res_file* res)
    {
        mpr->sectors = (ce_mprsector*)ce_alloc_zero(sizeof(ce_mprsector) * mpr->sector_x_count * mpr->sector_z_count);

        // mpr name + xxxzzz.sec
        std::vector<char> sec_name(mpr->name->length + 3 + 3 + 4 + 1);

        for (int z = 0, z_count = mpr->sector_z_count; z < z_count; ++z) {
            for (int x = 0, x_count = mpr->sector_x_count; x < x_count; ++x) {
                snprintf(sec_name.data(), sec_name.size(), "%s%03d%03d.sec", mpr->name->str, x, z);

                ce_mprsector* sec = mpr->sectors + z * x_count + x;
                ce_mem_file* mem = ce_res_ball_extract_mem_file_by_name(res, sec_name.data());

                uint32_t signature;
                ce_mem_file_read(mem, &signature, sizeof(uint32_t), 1);
                assert(SEC_SIGNATURE == le2cpu(signature) && "wrong signature");

                ce_mem_file_read(mem, &sec->water, sizeof(uint8_t), 1);

                sec->land_vertices = (ce_mprvertex*)ce_alloc(sizeof(ce_mprvertex) * CE_MPRFILE_VERTEX_COUNT);
                for (unsigned int i = 0; i < CE_MPRFILE_VERTEX_COUNT; ++i) {
                    read_vertex(sec->land_vertices + i, mem);
                }

                if (0 != sec->water) {
                    sec->water_vertices = (ce_mprvertex*)ce_alloc(sizeof(ce_mprvertex) * CE_MPRFILE_VERTEX_COUNT);
                    for (unsigned int i = 0; i < CE_MPRFILE_VERTEX_COUNT; ++i) {
                        read_vertex(sec->water_vertices + i, mem);
                    }
                }

                sec->land_textures = (uint16_t*)ce_alloc(sizeof(uint16_t) * CE_MPRFILE_TEXTURE_COUNT);
                ce_mem_file_read(mem, sec->land_textures, sizeof(uint16_t), CE_MPRFILE_TEXTURE_COUNT);
                for (unsigned int i = 0; i < CE_MPRFILE_TEXTURE_COUNT; ++i) {
                    sec->land_textures[i] = le2cpu(sec->land_textures[i]);
                }

                if (0 != sec->water) {
                    sec->water_textures = (uint16_t*)ce_alloc(sizeof(uint16_t) * CE_MPRFILE_TEXTURE_COUNT);
                    sec->water_allow = (int16_t*)ce_alloc(sizeof(int16_t) * CE_MPRFILE_TEXTURE_COUNT);

                    ce_mem_file_read(mem, sec->water_textures, sizeof(uint16_t), CE_MPRFILE_TEXTURE_COUNT);
                    ce_mem_file_read(mem, sec->water_allow, sizeof(int16_t), CE_MPRFILE_TEXTURE_COUNT);

                    for (unsigned int i = 0; i < CE_MPRFILE_TEXTURE_COUNT; ++i) {
                        sec->water_textures[i] = le2cpu(sec->water_textures[i]);
                        sec->water_allow[i] = le2cpu(sec->water_allow[i]);
                    }
                }

                ce_mem_file_del(mem);
            }
        }
    }

    ce_mprfile* ce_mprfile_open(ce_res_file* res_file)
    {
        ce_mprfile* mprfile = (ce_mprfile*)ce_alloc_zero(sizeof(ce_mprfile));

        // mpr name = res name without extension (.mpr)
        mprfile->name = ce_string_dup_n(res_file->name, res_file->name->length - 4);

        // mpr name + .mp
        std::vector<char> mp_name(mprfile->name->length + 3 + 1);
        snprintf(mp_name.data(), mp_name.size(), "%s.mp", mprfile->name->str);

        int mp_index = ce_res_file_node_index(res_file, mp_name.data());
        mprfile->size = ce_res_file_node_size(res_file, mp_index);
        mprfile->data = ce_res_file_node_data(res_file, mp_index);

        union {
            float* f;
            uint16_t* u16;
            uint32_t* u32;
        } ptr = { (float*)mprfile->data };

        uint32_t signature = le2cpu(*ptr.u32++);
        assert(MP_SIGNATURE == signature && "wrong signature");

        mprfile->max_y = *ptr.f++;
        mprfile->sector_x_count = le2cpu(*ptr.u32++);
        mprfile->sector_z_count = le2cpu(*ptr.u32++);
        mprfile->texture_count = le2cpu(*ptr.u32++);
        mprfile->texture_size = le2cpu(*ptr.u32++);
        mprfile->tile_count = le2cpu(*ptr.u32++);
        mprfile->tile_size = le2cpu(*ptr.u32++);
        mprfile->material_count = le2cpu(*ptr.u16++);
        mprfile->anim_tile_count = le2cpu(*ptr.u32++);

        for (int i = 0; i < mprfile->material_count; ++i, ptr.f += 10) {
            int type = le2cpu(*ptr.u32++);
            mprfile->materials[1 != type] = ptr.f;
        }

        mprfile->tiles = ptr.u32; ptr.u32 += mprfile->tile_count;
        mprfile->anim_tiles = ptr.u16;

        for (int i = 0; i < mprfile->tile_count; ++i) {
            mprfile->tiles[i] = le2cpu(mprfile->tiles[i]);
        }

        for (int i = 0, n = 2 * mprfile->anim_tile_count; i < n; ++i) {
            mprfile->anim_tiles[i] = le2cpu(mprfile->anim_tiles[i]);
        }

        read_sectors(mprfile, res_file);

        return mprfile;
    }

    void ce_mprfile_close(ce_mprfile* mprfile)
    {
        if (NULL != mprfile) {
            ce_free(mprfile->data, mprfile->size);
            for (int i = 0, n = mprfile->sector_x_count * mprfile->sector_z_count; i < n; ++i) {
                ce_mprsector* sector = mprfile->sectors + i;
                ce_free(sector->water_allow, sizeof(int16_t) * CE_MPRFILE_TEXTURE_COUNT);
                ce_free(sector->water_textures, sizeof(uint16_t) * CE_MPRFILE_TEXTURE_COUNT);
                ce_free(sector->land_textures, sizeof(uint16_t) * CE_MPRFILE_TEXTURE_COUNT);
                ce_free(sector->water_vertices, sizeof(ce_mprvertex) * CE_MPRFILE_VERTEX_COUNT);
                ce_free(sector->land_vertices, sizeof(ce_mprvertex) * CE_MPRFILE_VERTEX_COUNT);
            }
            ce_free(mprfile->sectors, sizeof(ce_mprsector) * mprfile->sector_x_count * mprfile->sector_z_count);
            ce_string_del(mprfile->name);
            ce_free(mprfile, sizeof(ce_mprfile));
        }
    }
}
