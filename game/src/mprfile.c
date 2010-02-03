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
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <GL/gl.h>

#include "cealloc.h"
#include "cebyteorder.h"
#include "cestr.h"
#include "cemath.h"
#include "vec3.h"
#include "ceaabb.h"
#include "cememfile.h"
#include "ceresfile.h"
#include "cetexture.h"
#include "mprfile.h"

static const uint32_t MP_SIGNATURE = 0xce4af672;
static const uint32_t SEC_SIGNATURE = 0xcf4bf774;

enum {
	MATERIAL_GROUND = 1,
	MATERIAL_WATER = 3,
};

static const unsigned int VERTEX_SIDE = 33;
static const unsigned int VERTEX_COUNT = 33 * 33;

static const unsigned int TEXTURE_SIDE = 16;
static const unsigned int TEXTURE_COUNT = 16 * 16;

typedef struct {
	uint32_t type;
	float color[4];
	float selfillum;
	float wavemult;
} material;

typedef struct {
	uint16_t index;
	uint16_t count;
} anim_tile;

typedef struct {
	int8_t offset_x;
	int8_t offset_z;
	uint16_t coord_y;
	uint32_t normal;
} vertex;

typedef struct {
	unsigned int x, z;
	ce_aabb box;
	float dist2; // for sorting on rendering
	uint8_t water;
	vertex* land_vertices;
	vertex* water_vertices;
	uint16_t* land_textures;
	uint16_t* water_textures;
	int16_t* water_allow;
} sector;

struct mprfile {
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
	material* materials;
	uint32_t* tiles; // TODO: id for bind sound ?
	anim_tile* anim_tiles;
	sector* sectors;
	unsigned int visible_sector_count;
	sector** visible_sectors;
	ce_texture** textures;
	bool night;
};

static void normal2vector(uint32_t normal, float* vector)
{
	vector[0] = (((normal >> 11) & 0x7ff) - 1000.0f) / 1000.0f;
	vector[1] = (normal >> 22) / 1000.0f;
	vector[2] = ((normal & 0x7ff) - 1000.0f) / 1000.0f;
}

static uint8_t texture_index(uint16_t value)
{
	return value & 0x003f;
}

static uint8_t texture_number(uint16_t value)
{
	return (value & 0x3fc0) >> 6;
}

static uint8_t texture_angle(uint16_t value)
{
	return (value & 0xc000) >> 14;
}

static material* find_material(unsigned int type, mprfile* mpr)
{
	for (unsigned int i = 0; i < mpr->material_count; ++i) {
		material* mat = mpr->materials + i;
		if (type == mat->type) {
			return mat;
		}
	}
	return NULL;
}

static bool read_material(material* mat, ce_memfile* mem)
{
	float unknown[4];
	if (1 != ce_memfile_read(mem, &mat->type, sizeof(uint32_t), 1) ||
			4 != ce_memfile_read(mem, mat->color, sizeof(float), 4) ||
			1 != ce_memfile_read(mem, &mat->selfillum, sizeof(float), 1) ||
			1 != ce_memfile_read(mem, &mat->wavemult, sizeof(float), 1) ||
			4 != ce_memfile_read(mem, unknown, sizeof(float), 4)) {
		return false;
	}
	ce_le2cpu32s(&mat->type);
	printf("mat %u: %f %f %f %f, si: %f\n", mat->type, mat->color[0],
		mat->color[1], mat->color[2], mat->color[3], mat->selfillum);
	return true;
}

static bool read_anim_tile(anim_tile* at, ce_memfile* mem)
{
	if (1 != ce_memfile_read(mem, &at->index, sizeof(uint16_t), 1) ||
			1 != ce_memfile_read(mem, &at->count, sizeof(uint16_t), 1)) {
		return false;
	}
	ce_le2cpu16s(&at->index);
	ce_le2cpu16s(&at->count);
	return true;
}

static bool read_header_impl(mprfile* mpr, ce_memfile* mem)
{
	uint32_t signature;
	if (1 != ce_memfile_read(mem, &signature, sizeof(uint32_t), 1)) {
		return false;
	}

	ce_le2cpu32s(&signature);
	if (MP_SIGNATURE != signature) {
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

	mpr->sector_count = mpr->sector_x_count * mpr->sector_z_count;

	mpr->materials = ce_alloc(sizeof(material) * mpr->material_count);
	if (NULL == mpr->materials) {
		return false;
	}

	for (unsigned int i = 0; i < mpr->material_count; ++i) {
		if (!read_material(mpr->materials + i, mem)) {
			return false;
		}
	}

	mpr->tiles = ce_alloc(sizeof(uint32_t) * mpr->tile_count);
	if (NULL == mpr->tiles || (size_t)mpr->tile_count !=
			ce_memfile_read(mem, mpr->tiles, sizeof(uint32_t), mpr->tile_count)) {
		return false;
	}

	for (unsigned int i = 0; i < mpr->tile_count; ++i) {
		ce_le2cpu32s(mpr->tiles + i);
	}

	mpr->anim_tiles = ce_alloc(sizeof(anim_tile) * mpr->anim_tile_count);
	if (NULL == mpr->anim_tiles) {
		return false;
	}

	for (unsigned int i = 0; i < mpr->anim_tile_count; ++i) {
		if (!read_anim_tile(mpr->anim_tiles + i, mem)) {
			return false;
		}
	}

	return true;
}

static bool read_header(mprfile* mpr, const char* mpr_name,
						size_t mpr_name_length, ce_resfile* res)
{
	if (0 == mpr_name_length) {
		return false;
	}

	// make mp_name by truncation of one last mpr_name character
	char mp_name[mpr_name_length];
	ce_strlcpy(mp_name, mpr_name, sizeof(mp_name));

	int index = ce_resfile_node_index(res, mp_name);
	if (index < 0) {
		return false;
	}

	const size_t data_size = ce_resfile_node_size(res, index);
	void* data = ce_alloc(data_size);
	if (NULL == data || !ce_resfile_node_data(res, index, data)) {
		ce_free(data, data_size);
		return false;
	}

	ce_memfile* mem = ce_memfile_open_data(data, data_size, "rb");
	if (NULL == mem) {
		ce_free(data, data_size);
		return false;
	}

	bool ok = read_header_impl(mpr, mem);

	ce_memfile_close(mem);
	ce_free(data, data_size);

	return ok;
}

static bool read_vertex(vertex* ver, ce_memfile* mem)
{
	if (1 != ce_memfile_read(mem, &ver->offset_x, sizeof(int8_t), 1) ||
			1 != ce_memfile_read(mem, &ver->offset_z, sizeof(int8_t), 1) ||
			1 != ce_memfile_read(mem, &ver->coord_y, sizeof(uint16_t), 1) ||
			1 != ce_memfile_read(mem, &ver->normal, sizeof(uint32_t), 1)) {
		return false;
	}
	ce_le2cpu16s(&ver->coord_y);
	ce_le2cpu32s(&ver->normal);
	return true;
}

static bool read_sector_impl(sector* sec, ce_memfile* mem)
{
	uint32_t signature;
	if (1 != ce_memfile_read(mem, &signature, sizeof(uint32_t), 1)) {
		return false;
	}

	ce_le2cpu32s(&signature);
	if (SEC_SIGNATURE != signature) {
		return false;
	}

	if (1 != ce_memfile_read(mem, &sec->water, sizeof(uint8_t), 1)) {
		return false;
	}

	sec->land_vertices = ce_alloc(sizeof(vertex) * VERTEX_COUNT);
	if (NULL == sec->land_vertices) {
		return false;
	}

	for (unsigned int i = 0; i < VERTEX_COUNT; ++i) {
		if (!read_vertex(sec->land_vertices + i, mem)) {
			return false;
		}
	}

	if (0 != sec->water) {
		sec->water_vertices = ce_alloc(sizeof(vertex) * VERTEX_COUNT);
		if (NULL == sec->water_vertices) {
			return false;
		}

		for (unsigned int i = 0; i < VERTEX_COUNT; ++i) {
			if (!read_vertex(sec->water_vertices + i, mem)) {
				return false;
			}
		}
	}

	sec->land_textures = ce_alloc(sizeof(uint16_t) * TEXTURE_COUNT);
	if (NULL == sec->land_textures || TEXTURE_COUNT != ce_memfile_read(mem,
			sec->land_textures, sizeof(uint16_t), TEXTURE_COUNT)) {
		return false;
	}

	for (unsigned int i = 0; i < TEXTURE_COUNT; ++i) {
		ce_le2cpu16s(sec->land_textures + i);
	}

	if (0 != sec->water) {
		sec->water_textures = ce_alloc(sizeof(uint16_t) * TEXTURE_COUNT);
		sec->water_allow = ce_alloc(sizeof(int16_t) * TEXTURE_COUNT);

		if (NULL == sec->water_textures || NULL == sec->water_allow ||
				TEXTURE_COUNT != ce_memfile_read(mem, sec->water_textures,
					sizeof(uint16_t), TEXTURE_COUNT) ||
				TEXTURE_COUNT != ce_memfile_read(mem, sec->water_allow,
					sizeof(int16_t), TEXTURE_COUNT)) {
			return false;
		}

		for (unsigned int i = 0; i < TEXTURE_COUNT; ++i) {
			ce_le2cpu16s(sec->water_textures + i);
			ce_le2cpu16s((uint16_t*)(sec->water_allow + i));
		}
	}

	return true;
}

static bool read_sector(sector* sec, const char* name, ce_resfile* res)
{
	int index = ce_resfile_node_index(res, name);
	if (index < 0) {
		return false;
	}

	const size_t data_size = ce_resfile_node_size(res, index);
	void* data = ce_alloc(data_size);
	if (NULL == data || !ce_resfile_node_data(res, index, data)) {
		ce_free(data, data_size);
		return false;
	}

	ce_memfile* mem = ce_memfile_open_data(data, data_size, "rb");
	if (NULL == mem) {
		ce_free(data, data_size);
		return false;
	}

	bool ok = read_sector_impl(sec, mem);

	ce_memfile_close(mem);
	ce_free(data, data_size);

	return ok;
}

static bool read_sectors(mprfile* mpr, const char* mpr_name,
							size_t mpr_name_length, ce_resfile* res)
{
	if (NULL == (mpr->sectors =
					ce_alloc_zero(sizeof(sector) * mpr->sector_count)) ||
			NULL == (mpr->visible_sectors =
						ce_alloc(sizeof(sector*) * mpr->sector_count))) {
		return false;
	}

	if (mpr_name_length < 4) {
		return false;
	}

	// mpr_name without extension
	char sec_tmpl_name[mpr_name_length - 4 + 1];
	ce_strlcpy(sec_tmpl_name, mpr_name, sizeof(sec_tmpl_name));

	// sec_tmpl_name + zzzxxx.sec
	char sec_name[sizeof(sec_tmpl_name) + 3 + 3 + 4];

	for (unsigned int z = 0, i; z < mpr->sector_z_count; ++z) {
		for (unsigned int x = 0; x < mpr->sector_x_count; ++x) {
			i = z * mpr->sector_x_count + x;

			snprintf(sec_name, sizeof(sec_name),
				"%s%03u%03u.sec", sec_tmpl_name, x, z);

			sector* sec = mpr->sectors + i;

			if (!read_sector(sec, sec_name, res)) {
				return false;
			}

			sec->x = x;
			sec->z = z;

			vec3_init(x * (VERTEX_SIDE - 1), 0.0f, -1.0f *
				(z * (VERTEX_SIDE - 1) + (VERTEX_SIDE - 1)), &sec->box.min);
			vec3_init(x * (VERTEX_SIDE - 1) + (VERTEX_SIDE - 1), mpr->max_y,
				-1.0f * (z * (VERTEX_SIDE - 1)), &sec->box.max);

			mpr->visible_sectors[i] = sec;
		}
	}

	mpr->visible_sector_count = mpr->sector_count;

	return true;
}

static bool create_texture(ce_texture** tex, const char* name, ce_resfile* res)
{
	int index = ce_resfile_node_index(res, name);
	if (index < 0) {
		return false;
	}

	const size_t data_size = ce_resfile_node_size(res, index);
	void* data = ce_alloc(data_size);
	if (NULL == data || !ce_resfile_node_data(res, index, data)) {
		ce_free(data, data_size);
		return false;
	}

	*tex = ce_texture_open(data);

	ce_free(data, data_size);

	return NULL != *tex;
}

static bool create_textures(mprfile* mpr, const char* mpr_name,
							size_t mpr_name_length, ce_resfile* res)
{
	mpr->textures = ce_alloc_zero(sizeof(ce_texture*) * mpr->texture_count);
	if (NULL == mpr->textures) {
		return false;
	}

	if (mpr_name_length < 4) {
		return false;
	}

	// mpr_name without extension
	char mmp_tmpl_name[mpr_name_length - 4 + 1];
	ce_strlcpy(mmp_tmpl_name, mpr_name, sizeof(mmp_tmpl_name));

	// mmp_tmpl_name + nnn.mmp
	char mmp_name[sizeof(mmp_tmpl_name) + 3 + 4];

	for (unsigned int i = 0; i < mpr->texture_count; ++i) {
		snprintf(mmp_name, sizeof(mmp_name), "%s%03u.mmp", mmp_tmpl_name, i);

		if (!create_texture(mpr->textures + i, mmp_name, res)) {
			return false;
		}
	}

	return true;
}

mprfile* mprfile_open(ce_resfile* mpr_res, ce_resfile* textures_res)
{
	mprfile* mpr = ce_alloc_zero(sizeof(mprfile));
	if (NULL == mpr) {
		return NULL;
	}

	const char* mpr_name = ce_resfile_name(mpr_res);
	size_t mpr_name_length = strlen(mpr_name);

	if (!read_header(mpr, mpr_name, mpr_name_length, mpr_res)) {
		mprfile_close(mpr);
		return NULL;
	}

	if (!read_sectors(mpr, mpr_name, mpr_name_length, mpr_res)) {
		mprfile_close(mpr);
		return NULL;
	}

	if (!create_textures(mpr, mpr_name, mpr_name_length, textures_res)) {
		mprfile_close(mpr);
		return NULL;
	}

	mpr->night = false;

	return mpr;
}

void mprfile_close(mprfile* mpr)
{
	if (NULL == mpr) {
		return;
	}

	if (NULL != mpr->textures) {
		for (unsigned int i = 0; i < mpr->texture_count; ++i) {
			ce_texture_close(mpr->textures[i]);
		}
	}

	if (NULL != mpr->sectors) {
		for (unsigned int i = 0; i < mpr->sector_count; ++i) {
			sector* sec = mpr->sectors + i;
			ce_free(sec->water_allow, sizeof(int16_t) * TEXTURE_COUNT);
			ce_free(sec->water_textures, sizeof(uint16_t) * TEXTURE_COUNT);
			ce_free(sec->land_textures, sizeof(uint16_t) * TEXTURE_COUNT);
			ce_free(sec->water_vertices, sizeof(vertex) * VERTEX_COUNT);
			ce_free(sec->land_vertices, sizeof(vertex) * VERTEX_COUNT);
		}
	}

	ce_free(mpr->textures, sizeof(ce_texture*) * mpr->texture_count);
	ce_free(mpr->visible_sectors, sizeof(sector*) * mpr->sector_count);
	ce_free(mpr->sectors, sizeof(sector) * mpr->sector_count);
	ce_free(mpr->anim_tiles, sizeof(anim_tile) * mpr->anim_tile_count);
	ce_free(mpr->tiles, sizeof(uint32_t) * mpr->tile_count);
	ce_free(mpr->materials, sizeof(material) * mpr->material_count);

	ce_free(mpr, sizeof(mprfile));
}

float mprfile_get_max_height(const mprfile* mpr)
{
	return mpr->max_y;
}

bool mprfile_get_night(mprfile* mpr)
{
	return mpr->night;
}

void mprfile_set_night(bool value, mprfile* mpr)
{
	mpr->night = value;
}

static int sector_dist_comp(const void* lhs, const void* rhs)
{
	const sector* sec1 = *(const sector**)lhs;
	const sector* sec2 = *(const sector**)rhs;
	return ce_fisequal(sec1->dist2, sec2->dist2, CE_EPS_E4) ? 0 :
		(sec1->dist2 < sec2->dist2 ? 1 : -1);
}

void mprfile_apply_frustum(const vec3* eye, const ce_frustum* f, mprfile* mpr)
{
	mpr->visible_sector_count = 0;

	for (unsigned int i = 0; i < mpr->sector_count; ++i) {
		sector* sec = mpr->sectors + i;
		if (ce_frustum_test_box(f, &sec->box)) {
			vec3 mid;
			vec3_mid(&sec->box.min, &sec->box.max, &mid);
			sec->dist2 = vec3_dist2(eye, &mid);
			mpr->visible_sectors[mpr->visible_sector_count++] = sec;
		}
	}

	qsort(mpr->visible_sectors, mpr->visible_sector_count,
		sizeof(sector*), sector_dist_comp);
}

static void render_sector(unsigned int sector_x, unsigned int sector_z,
							vertex* vertices, uint16_t* textures,
							int16_t* water_allow, mprfile* mpr)
{
	GLfloat varray[3 * VERTEX_COUNT];
	GLfloat narray[3 * VERTEX_COUNT];
	GLfloat tcarray[2 * VERTEX_COUNT];

	static const float offset_xz_coef = 1.0f / (INT8_MAX - INT8_MIN);
	const float y_coef = mpr->max_y / (UINT16_MAX - 0);

	for (unsigned int z = 0; z < VERTEX_SIDE; ++z) {
		for (unsigned int x = 0; x < VERTEX_SIDE; ++x) {
			unsigned int i = z * VERTEX_SIDE * 3 + x * 3;
			vertex* ver = vertices + (z * VERTEX_SIDE + x);

			varray[i + 0] = x + sector_x * (VERTEX_SIDE - 1) +
								offset_xz_coef * ver->offset_x;
			varray[i + 1] = y_coef * ver->coord_y;
			varray[i + 2] = -1.0f * (z + sector_z * (VERTEX_SIDE - 1) +
								offset_xz_coef * ver->offset_z);

			normal2vector(ver->normal, narray + i);
			narray[i + 2] = -narray[i + 2];
		}
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, varray);
	glNormalPointer(GL_FLOAT, 0, narray);
	glTexCoordPointer(2, GL_FLOAT, 0, tcarray);

	for (unsigned int z = 0; z < VERTEX_SIDE - 2; z += 2) {
		for (unsigned int x = 0; x < VERTEX_SIDE - 2; x += 2) {
			if (NULL != water_allow &&
					-1 == water_allow[z / 2 * TEXTURE_SIDE + x / 2]) {
				continue;
			}

			uint16_t tex = textures[z / 2 * TEXTURE_SIDE + x / 2];

			int tex_idx = texture_index(tex);
			float u = (tex_idx - tex_idx / 8 * 8) / 8.0f;
			float v = (7 - tex_idx / 8) / 8.0f;

			static const float tile_uv_step = 1.0f / 8.0f;
			static const float tile_uv_half_step = 1.0f / 16.0f;

			static const float tile_border_size = 8.0f; // in pixels
			const float tile_uv_border_offset = tile_border_size /
													mpr->texture_size;

			float texcoords[2 * 9] = {
				u + tile_uv_border_offset,
					v + tile_uv_step - tile_uv_border_offset,
				u + tile_uv_half_step, v + tile_uv_step - tile_uv_border_offset,
				u + tile_uv_step - tile_uv_border_offset,
					v + tile_uv_step - tile_uv_border_offset,
				u + tile_uv_step - tile_uv_border_offset, v + tile_uv_half_step,
				u + tile_uv_step - tile_uv_border_offset,
					v + tile_uv_border_offset,
				u + tile_uv_half_step, v + tile_uv_border_offset,
				u + tile_uv_border_offset, v + tile_uv_border_offset,
				u + tile_uv_border_offset, v + tile_uv_half_step,
				u + tile_uv_half_step, v + tile_uv_half_step
			};

			static unsigned int offx[9] = { 0, 1, 2, 2, 2, 1, 0, 0, 1 };
			static unsigned int offz[9] = { 0, 0, 0, 1, 2, 2, 2, 1, 1 };

			for (unsigned int i = 0; i < 9; ++i) {
				unsigned int tci = (z + offz[i]) * VERTEX_SIDE * 2 +
													(x + offx[i]) * 2;
				tcarray[tci + 0] = texcoords[i * 2 + 0];
				tcarray[tci + 1] = texcoords[i * 2 + 1];
			}

			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glTranslatef(u + tile_uv_half_step,
							v + tile_uv_half_step, 0.0f);
			glRotatef(-90.0f * texture_angle(tex), 0.0f, 0.0f, 1.0f);
			glTranslatef(-u - tile_uv_half_step,
							-v - tile_uv_half_step, 0.0f);
			glMatrixMode(GL_MODELVIEW);

			GLushort vind[9];
			for (unsigned int i = 0; i < 9; ++i) {
				vind[i] = (z + offz[i]) * VERTEX_SIDE + (x + offx[i]);
			}

			GLushort indices[2][6] = {
				{ vind[7], vind[0], vind[8], vind[1], vind[3], vind[2] },
				{ vind[6], vind[7], vind[5], vind[8], vind[4], vind[3] }
			};

			material* mat = find_material(NULL != water_allow ?
							MATERIAL_WATER : MATERIAL_GROUND, mpr);
			assert(mat);
			glMaterialfv(GL_FRONT, GL_AMBIENT, (float[]){0.3f,0.3f,0.3f,1.0f});
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->color);
			//glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat->color);

			//float em[4] = { mat->selfillum * mat->color[0], mat->selfillum * mat->color[1],
			//				mat->selfillum * mat->color[2], mat->selfillum * mat->color[3] };
			//glMaterialfv(GL_FRONT, GL_EMISSION, em);

			if (NULL != water_allow) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			ce_texture_bind(mpr->textures[texture_number(tex)]);

			glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, indices[0]);
			glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, indices[1]);

			ce_texture_unbind(mpr->textures[texture_number(tex)]);

			if (NULL != water_allow) {
				glDisable(GL_BLEND);
			}
		}
	}

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

static void render_sectors(bool opacity, mprfile* mpr)
{
	for (unsigned int i = 0; i < mpr->visible_sector_count; ++i) {
		sector* sec = mpr->visible_sectors[i];
		if (opacity) {
			render_sector(sec->x, sec->z, sec->land_vertices,
							sec->land_textures, NULL, mpr);
		} else if (0 != sec->water) {
			render_sector(sec->x, sec->z, sec->water_vertices,
							sec->water_textures, sec->water_allow, mpr);
		}
	}
}

void mprfile_render(mprfile* mpr)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	render_sectors(true, mpr); // opacity geometry first
	render_sectors(false, mpr); // then water/swamp/lava

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}
