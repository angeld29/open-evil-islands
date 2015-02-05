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

#include <cstring>
#include <climits>
#include <cmath>
#include <vector>

#include "lib.hpp"
#include "math.hpp"
#include "plane.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "mprhelpers.hpp"

namespace cursedearth
{
    const float CE_MPR_OFFSET_XZ_COEF = 1.0f / (INT8_MAX - INT8_MIN);
    const float CE_MPR_HEIGHT_Y_COEF = 1.0f / (UINT16_MAX - 0);

    ce_aabb* ce_mpr_get_aabb(ce_aabb* aabb, const ce_mprfile* mprfile, int sector_x, int sector_z, bool water)
    {
        const float y_coef = CE_MPR_HEIGHT_Y_COEF * mprfile->max_y;
        float y = 0.0f;

        ce_mprsector* sector = mprfile->sectors + sector_z * mprfile->sector_x_count + sector_x;
        ce_mprvertex* vertices = water ? sector->water_vertices : sector->land_vertices;
        int16_t* water_allow = water ? sector->water_allow : nullptr;

        for (int z = 0; z < CE_MPRFILE_VERTEX_SIDE; ++z) {
            for (int x = 0; x < CE_MPRFILE_VERTEX_SIDE; ++x) {
                if (NULL != water_allow && -1 == water_allow[z / 2 * CE_MPRFILE_TEXTURE_SIDE + x / 2]) {
                    continue;
                }
                y = fmaxf(y, y_coef * vertices[z * CE_MPRFILE_VERTEX_SIDE + x].coord_y);
            }
        }

        // FIXME: negative z?..
        ce_vec3 min, max;
        ce_vec3_init(&min, sector_x * (CE_MPRFILE_VERTEX_SIDE - 1), 0.0f, -1.0f *
            (sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) + (CE_MPRFILE_VERTEX_SIDE - 1)));
        ce_vec3_init(&max,
            sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) + (CE_MPRFILE_VERTEX_SIDE - 1),
            y, -1.0f * (sector_z * (CE_MPRFILE_VERTEX_SIDE - 1)));

        aabb->radius = 0.5f * ce_vec3_dist(&min, &max);
        ce_vec3_mid(&aabb->origin, &min, &max);
        ce_vec3_sub(&aabb->extents, &max, &aabb->origin);

        return aabb;
    }

    bool ce_mpr_get_height_triangle(const ce_mprfile* mprfile, int sector_x, int sector_z, int vertex_x1, int vertex_z1,
                                    int vertex_x2, int vertex_z2, int vertex_x3, int vertex_z3, float x, float z, float* y)
    {
        const ce_mprsector* sector = mprfile->sectors + sector_z * mprfile->sector_x_count + sector_x;
        const ce_mprvertex* vertex1 = sector->land_vertices + vertex_z1 * CE_MPRFILE_VERTEX_SIDE + vertex_x1;
        const ce_mprvertex* vertex2 = sector->land_vertices + vertex_z2 * CE_MPRFILE_VERTEX_SIDE + vertex_x2;
        const ce_mprvertex* vertex3 = sector->land_vertices + vertex_z3 * CE_MPRFILE_VERTEX_SIDE + vertex_x3;
        const float y_coef = CE_MPR_HEIGHT_Y_COEF * mprfile->max_y;

        ce_triangle triangle;

        ce_vec3_init(&triangle.a,
            vertex_x1 + sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) +
                        CE_MPR_OFFSET_XZ_COEF * vertex1->offset_x,
            y_coef * vertex1->coord_y,
            vertex_z1 + sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) +
                        CE_MPR_OFFSET_XZ_COEF * vertex1->offset_z);

        ce_vec3_init(&triangle.b,
            vertex_x2 + sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) +
                        CE_MPR_OFFSET_XZ_COEF * vertex2->offset_x,
            y_coef * vertex2->coord_y,
            vertex_z2 + sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) +
                        CE_MPR_OFFSET_XZ_COEF * vertex2->offset_z);

        ce_vec3_init(&triangle.c,
            vertex_x3 + sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) +
                        CE_MPR_OFFSET_XZ_COEF * vertex3->offset_x,
            y_coef * vertex3->coord_y,
            vertex_z3 + sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) +
                        CE_MPR_OFFSET_XZ_COEF * vertex3->offset_z);

        ce_plane plane;
        ce_plane_init_triangle(&plane, &triangle);

        ce_ray ray;
        ce_vec3_init(&ray.origin, x, mprfile->max_y + 1.0f, z);
        ce_vec3_init_neg_unit_y(&ray.direction);

        ce_vec3 point;
        if (!ce_plane_isect_ray(&plane, &ray, &point) || !ce_triangle_test(&triangle, &point)) {
            return false;
        }

        *y = point.y;
        return true;
    }

    bool ce_mpr_get_height_tile(const ce_mprfile* mprfile, float tile_offset_x, float tile_offset_z, float x, float z, float* y)
    {
        int round_x = x + tile_offset_x;
        int round_z = z + tile_offset_z;

        int sector_x = round_x / (CE_MPRFILE_VERTEX_SIDE - 1);
        int sector_z = round_z / (CE_MPRFILE_VERTEX_SIDE - 1);

        if (sector_x >= mprfile->sector_x_count || sector_z >= mprfile->sector_z_count) {
            ce_logging_debug("mpr helper: out of bounds");
            return false;
        }

        int vertex_x = round_x % (CE_MPRFILE_VERTEX_SIDE - 1);
        int vertex_z = round_z % (CE_MPRFILE_VERTEX_SIDE - 1);

        // round the vertex to left-bottom in tile
        vertex_x &= ~1;
        vertex_z &= ~1;

        // 8 triangles in one tile
        const int tri_offset_x[] = {1, 0, 1, 2, 2, 2, 1, 0, 0};
        const int tri_offset_z[] = {1, 0, 0, 0, 1, 2, 2, 2, 1};

        const size_t indices[][3] = {{2, 1, 0}, {0, 1, 8}, {0, 8, 6}, {6, 8, 7},
                                    {3, 2, 4}, {4, 2, 0}, {4, 0, 5}, {5, 0, 6}};

        for (size_t i = 0; i < 8; ++i) {
            if (ce_mpr_get_height_triangle(mprfile, sector_x, sector_z,
                    vertex_x + tri_offset_x[indices[i][0]], vertex_z + tri_offset_z[indices[i][0]],
                    vertex_x + tri_offset_x[indices[i][1]], vertex_z + tri_offset_z[indices[i][1]],
                    vertex_x + tri_offset_x[indices[i][2]], vertex_z + tri_offset_z[indices[i][2]],
                    x, z, y)) {
                return true;
            }
        }

        return false;
    }

    float ce_mpr_get_height(const ce_mprfile* mprfile, const ce_vec3* position)
    {
        float x = position->x;
        float z = position->z;

        // FIXME: negative z?..
        z = fabsf(z);

        // 8 neighbor tiles
        const float tile_offset_x[] = {0.0f, -2.0f, 2.0f, 0.0f, 0.0f, -2.0f, 2.0f, -2.0f, 2.0f};
        const float tile_offset_z[] = {0.0f, 0.0f, 0.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f};

        for (size_t i = 0; i < 9; ++i) {
            float y;
            if (ce_mpr_get_height_tile(mprfile, tile_offset_x[i], tile_offset_z[i], x, z, &y)) {
                return y;
            }
        }

        ce_logging_debug("mpr helper: triangle not found");
        return mprfile->max_y;
    }

    ce_material* ce_mpr_create_material(const ce_mprfile* mprfile, bool water)
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

    void ce_mpr_rotate_texture_0(uint32_t* dst, const uint32_t* src, int size)
    {
        memcpy(dst, src, sizeof(uint32_t) * size * size);
    }

    void ce_mpr_rotate_texture_90(uint32_t* dst, const uint32_t* src, int size)
    {
        for (int i = 0; i < size; ++i) {
            for (int j = size - 1; j >= 0; --j) {
                *dst++ = src[j * size + i];
            }
        }
    }

    void ce_mpr_rotate_texture_180(uint32_t* dst, const uint32_t* src, int size)
    {
        for (int i = size * size - 1; i >= 0; --i) {
            *dst++ = src[i];
        }
    }

    void ce_mpr_rotate_texture_270(uint32_t* dst, const uint32_t* src, int size)
    {
        for (int i = size - 1; i >= 0; --i) {
            for (int j = 0; j < size; ++j) {
                *dst++ = src[j * size + i];
            }
        }
    }

    void (*ce_mpr_rotate_texture_procs[])(uint32_t*, const uint32_t*, int) = {
        ce_mpr_rotate_texture_0, ce_mpr_rotate_texture_90,
        ce_mpr_rotate_texture_180, ce_mpr_rotate_texture_270,
    };

    ce_mmpfile* ce_mpr_generate_texture(const ce_mprfile* mprfile, const ce_vector* tile_mmp_files, int x, int z, bool water)
    {
        // WARNING: draft code, refactoring is needed...
        // TODO: comments

        ce_mmpfile* first_mmpfile = (ce_mmpfile*)tile_mmp_files->items[0];
        ce_mprsector* sector = mprfile->sectors + z * mprfile->sector_x_count + x;

        uint16_t* textures = water ? sector->water_textures :
                                    sector->land_textures;
        int16_t* water_allow = water ? sector->water_allow : NULL;

        int tile_size = mprfile->tile_size - 2 * 8; // minus borders
        int tile_size_sqr = tile_size * tile_size;

        uint32_t* tile = (uint32_t*)ce_alloc(sizeof(uint32_t) * tile_size_sqr);
        uint32_t* tile2 = (uint32_t*)ce_alloc(sizeof(uint32_t) * tile_size_sqr);

        ce_mmpfile* mmpfile = ce_mmpfile_new(tile_size * CE_MPRFILE_TEXTURE_SIDE,
            tile_size * CE_MPRFILE_TEXTURE_SIDE, first_mmpfile->mipmap_count,
            CE_MMPFILE_FORMAT_R8G8B8A8, CE_MPR_TEXTURE_VERSION);

        uint32_t* texels = static_cast<uint32_t*>(mmpfile->texels);
        std::vector<uint32_t*> texels2(mprfile->texture_count);
        for (int i = 0; i < mprfile->texture_count; ++i) {
            ce_mmpfile* tile_mmpfile = (ce_mmpfile*)tile_mmp_files->items[i];
            texels2[i] = static_cast<uint32_t*>(tile_mmpfile->texels);
        }

        for (unsigned int m = 0, tex_size = mmpfile->width, tex_size2 = first_mmpfile->width, tile_size2 = mprfile->tile_size;
                m < first_mmpfile->mipmap_count; ++m, tex_size >>= 1, tex_size2 >>= 1, tile_size >>= 1, tile_size2 >>= 1) {
            for (int i = 0; i < CE_MPRFILE_TEXTURE_SIDE; ++i) {
                for (int j = 0; j < CE_MPRFILE_TEXTURE_SIDE; ++j) {
                    if (NULL != water_allow && -1 == water_allow[i * CE_MPRFILE_TEXTURE_SIDE + j]) {
                        memset(tile, 0x0, sizeof(uint32_t) * tile_size_sqr);
                    } else {
                        uint16_t texture = textures[i * CE_MPRFILE_TEXTURE_SIDE + j];

                        int texture_index = ce_mpr_texture_index(texture);
                        int u = texture_index - texture_index / 8 * 8;
                        int v = 7 - texture_index / 8;

                        int p = v * tile_size2 + (8U >> m); // skip border
                        int q = u * tile_size2 + (8U >> m); // skip border

                        int idx = ce_mpr_texture_number(texture);

                        for (int k = 0; k < tile_size; ++k) {
                            for (int l = 0; l < tile_size; ++l) {
                                tile2[k * tile_size + l] = texels2[idx][(p + k) * tex_size2 + (q + l)];
                            }
                        }

                        (*ce_mpr_rotate_texture_procs[ce_mpr_texture_angle(texture)])(tile, tile2, tile_size);
                    }

                    int s = (CE_MPRFILE_TEXTURE_SIDE - 1 - i) * tile_size;
                    int t = j * tile_size;

                    for (int k = 0; k < tile_size; ++k) {
                        for (int l = 0; l < tile_size; ++l) {
                            texels[(s + k) * tex_size + (t + l)] = tile[k * tile_size + l];
                        }
                    }
                }
            }

            texels += ce_mmpfile_storage_size(tex_size, tex_size, 1, mmpfile->format) / sizeof(uint32_t);

            for (int i = 0; i < mprfile->texture_count; ++i) {
                ce_mmpfile* tile_mmpfile = (ce_mmpfile*)tile_mmp_files->items[i];
                texels2[i] += ce_mmpfile_storage_size(tex_size2, tex_size2, 1, tile_mmpfile->format) / sizeof(uint32_t);
            }
        }

        ce_free(tile2, sizeof(uint32_t) * tile_size_sqr);
        ce_free(tile, sizeof(uint32_t) * tile_size_sqr);

        return mmpfile;
    }
}
