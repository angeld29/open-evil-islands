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
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

#include <GL/gl.h>

#include "celogging.h"
#include "cealloc.h"
#include "cemprfile.h"
#include "cemprhlp.h"
#include "cemprmng.h"
#include "cetexmng.h"
#include "ceroot.h"
#include "ceterrain.h"

typedef struct {
	GLuint id;
} ce_renditem_terrain;

static void ce_renditem_terrain_ctor(ce_renditem* renditem, va_list args)
{
	ce_terrain* terrain = va_arg(args, ce_terrain*);
	ce_mprfile* mprfile = va_arg(args, ce_mprfile*);
	int sector_x = va_arg(args, int);
	int sector_z = va_arg(args, int);
	ce_mprfile_vertex* vertices = va_arg(args, ce_mprfile_vertex*);
	uint16_t* textures = va_arg(args, uint16_t*);
	int16_t* water_allow = va_arg(args, int16_t*);

	ce_vec3_init(&renditem->bounding_box.min,
		sector_x * (CE_MPRFILE_VERTEX_SIDE - 1), 0.0f, -1.0f *
		(sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) + (CE_MPRFILE_VERTEX_SIDE - 1)));
	ce_vec3_init(&renditem->bounding_box.max,
		sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) + (CE_MPRFILE_VERTEX_SIDE - 1),
		mprfile->max_y, -1.0f * (sector_z * (CE_MPRFILE_VERTEX_SIDE - 1)));
	ce_vec3_mid(&renditem->bounding_box.center,
				&renditem->bounding_box.min, &renditem->bounding_box.max);

	ce_vec3_copy(&renditem->bounding_sphere.center,
					&renditem->bounding_box.center);
	renditem->bounding_sphere.radius =
		ce_vec3_dist(&renditem->bounding_box.min,
					&renditem->bounding_box.center);

	renditem->transparent = NULL != water_allow;

	ce_mprfile_material* material =
		ce_mprhlp_find_material(mprfile, NULL != water_allow ?
											CE_MPRFILE_MATERIAL_TYPE_WATER :
											CE_MPRFILE_MATERIAL_TYPE_GROUND);
	assert(material);

	const float ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	const float emission[] = {
		material->selfillum * material->color[0],
		material->selfillum * material->color[1],
		material->selfillum * material->color[2],
		material->selfillum * material->color[3]
	};

	const float offset_xz_coef = 1.0f / (INT8_MAX - INT8_MIN);
	const float y_coef = mprfile->max_y / (UINT16_MAX - 0);

	const float tile_uv_step = 1.0f / 8.0f;
	const float tile_uv_half_step = 1.0f / 16.0f;

	const float tile_border_size = 8.0f; // in pixels
	const float tile_uv_border_offset = tile_border_size / mprfile->texture_size;

	float vertex_array[3 * 9];
	float normal_array[3 * 9];

	/**
	 *  Tile traverse order:
	 *
	 *   6 ____5____ 4
	 *    |    |    |
	 *  7 |____|____| 3
	 *    |   8|    |
	 *    |____|____|
	 *   0     1     2
	 *
	 *
	 *  Triangle render order:
	 *
	 *  1) 7 __8__ 3   2) 6 __5__ 4
	 *      | /| /|        | /| /|
	 *      |/_|/_|        |/_|/_|
	 *     0   1   2      7   8   3
	*/

	const int offset_x[9] = { 0, 1, 2, 2, 2, 1, 0, 0, 1 };
	const int offset_z[9] = { 0, 0, 0, 1, 2, 2, 2, 1, 1 };

	const int indices[2][6] = {
		{ 7, 0, 8, 1, 3, 2 },
		{ 6, 7, 5, 8, 4, 3 }
	};

	ce_renditem_terrain* renditem_terrain = (ce_renditem_terrain*)renditem->impl;
	renditem_terrain->id = glGenLists(1);

	glNewList(renditem_terrain->id, GL_COMPILE);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material->color);
	glMaterialfv(GL_FRONT, GL_EMISSION, emission);

	if (NULL != water_allow) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	for (unsigned int z = 0; z < CE_MPRFILE_VERTEX_SIDE - 2; z += 2) {
		for (unsigned int x = 0; x < CE_MPRFILE_VERTEX_SIDE - 2; x += 2) {
			if (NULL != water_allow &&
					-1 == water_allow[z / 2 * CE_MPRFILE_TEXTURE_SIDE + x / 2]) {
				continue;
			}

			for (int i = 0; i < 9; ++i) {
				ce_mprfile_vertex* vertex = vertices +
					(z + offset_z[i]) * CE_MPRFILE_VERTEX_SIDE + (x + offset_x[i]);

				vertex_array[3 * i + 0] = x + offset_x[i] +
					sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) +
					offset_xz_coef * vertex->offset_x;
				vertex_array[3 * i + 1] = y_coef * vertex->coord_y;
				vertex_array[3 * i + 2] = -1.0f * (z + offset_z[i] +
					sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) +
					offset_xz_coef * vertex->offset_z);

				ce_mprhlp_normal2vector(normal_array + 3 * i, vertex->normal);
				normal_array[3 * i + 2] = -normal_array[3 * i + 2];
			}

			uint16_t texture = textures[z / 2 * CE_MPRFILE_TEXTURE_SIDE + x / 2];

			int texture_index = ce_mprhlp_texture_index(texture);
			float u = (texture_index - texture_index / 8 * 8) / 8.0f;
			float v = (7 - texture_index / 8) / 8.0f;

			float texcoord_array[2 * 9] = {
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

			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glTranslatef(u + tile_uv_half_step, v + tile_uv_half_step, 0.0f);
			glRotatef(-90.0f * ce_mprhlp_texture_angle(texture), 0.0f, 0.0f, 1.0f);
			glTranslatef(-u - tile_uv_half_step, -v - tile_uv_half_step, 0.0f);
			glMatrixMode(GL_MODELVIEW);

			ce_texture_bind(ce_vector_at(terrain->textures,
							ce_mprhlp_texture_number(texture)));

			for (int i = 0; i < 2; ++i) {
				glBegin(GL_TRIANGLE_STRIP);
				for (int j = 0; j < 6; ++j) {
					glTexCoord2fv(texcoord_array + 2 * indices[i][j]);
					glNormal3fv(normal_array + 3 * indices[i][j]);
					glVertex3fv(vertex_array + 3 * indices[i][j]);
				}
				glEnd();
			}

			ce_texture_unbind(ce_vector_at(terrain->textures,
							ce_mprhlp_texture_number(texture)));
		}
	}

	if (NULL != water_allow) {
		glDisable(GL_BLEND);
	}

	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glEndList();
}

static void ce_renditem_terrain_dtor(ce_renditem* renditem)
{
	ce_renditem_terrain* renditem_terrain = (ce_renditem_terrain*)renditem->impl;
	glDeleteLists(renditem_terrain->id, 1);
}

static void ce_renditem_terrain_render(ce_renditem* renditem)
{
	ce_renditem_terrain* renditem_terrain = (ce_renditem_terrain*)renditem->impl;
	glCallList(renditem_terrain->id);
}

static const ce_renditem_vtable ce_renditem_terrain_vtable = {
	ce_renditem_terrain_ctor, ce_renditem_terrain_dtor,
	ce_renditem_terrain_render
};

static bool ce_terrain_create_sector(ce_terrain* terrain, ce_mprfile* mprfile,
									int sector_x, int sector_z,
									ce_mprfile_sector* sector, bool opacity)
{
	if (!opacity && NULL == sector->water_allow) {
		return true;
	}

	ce_renditem* renditem =
		ce_renditem_new(ce_renditem_terrain_vtable,
						sizeof(ce_renditem_terrain),
						terrain, mprfile, sector_x, sector_z,
						opacity ? sector->land_vertices : sector->water_vertices,
						opacity ? sector->land_textures : sector->water_textures,
						opacity ? NULL : sector->water_allow);

	if (NULL != renditem) {
		ce_rendlayer_add_renditem(terrain->rendlayer, renditem);
		return true;
	}

	return false;
}

static bool ce_terrain_create(ce_terrain* terrain, ce_mprfile* mprfile)
{
	if (NULL == (terrain->textures =
					ce_vector_new_reserved(mprfile->texture_count)) ||
			NULL == (terrain->rendlayer =
						ce_rendlayer_new(mprfile->sector_x_count *
										mprfile->sector_z_count))) {
		return false;
	}

	// mpr name + nnn
	char texture_name[ce_string_length(mprfile->name) + 3 + 1];

	for (int i = 0, n = mprfile->texture_count; i < n; ++i) {
		snprintf(texture_name, sizeof(texture_name), "%s%03d",
				ce_string_cstr(mprfile->name), i);

		ce_texture* texture =
			ce_texmng_acquire_texture(ce_root_get_texmng(), texture_name);
		if (NULL == texture) {
			return false;
		}

		ce_vector_push_back(terrain->textures, texture);
	}

	ce_vec3_init(&terrain->rendlayer->bounding_box.min, 0.0f, 0.0f,
		-1.0f * (mprfile->sector_z_count * (CE_MPRFILE_VERTEX_SIDE - 1)));
	ce_vec3_init(&terrain->rendlayer->bounding_box.max,
		mprfile->sector_x_count * (CE_MPRFILE_VERTEX_SIDE - 1),
		mprfile->max_y, 0.0f);
	ce_vec3_mid(&terrain->rendlayer->bounding_box.center,
				&terrain->rendlayer->bounding_box.min,
				&terrain->rendlayer->bounding_box.max);

	ce_vec3_copy(&terrain->rendlayer->bounding_sphere.center,
				&terrain->rendlayer->bounding_box.center);
	terrain->rendlayer->bounding_sphere.radius =
		ce_vec3_dist(&terrain->rendlayer->bounding_box.min,
					&terrain->rendlayer->bounding_box.center);

	for (int z = 0, z_count = mprfile->sector_z_count; z < z_count; ++z) {
		for (int x = 0, x_count = mprfile->sector_x_count; x < x_count; ++x) {
			ce_mprfile_sector* sector = mprfile->sectors + z * x_count + x;
			if (!ce_terrain_create_sector(terrain, mprfile,
											x, z, sector, true) ||
					!ce_terrain_create_sector(terrain, mprfile,
												x, z, sector, false)) {
				return false;
			}
		}
	}

	return true;
}

static bool ce_terrain_new_impl(ce_terrain* terrain, const char* zone_name)
{
	ce_mprfile* mprfile = ce_mprmng_open_mprfile(ce_root_get_mprmng(), zone_name);
	if (NULL == mprfile) {
		return false;
	}

	bool ok = ce_terrain_create(terrain, mprfile);
	return ce_mprfile_close(mprfile), ok;
}

ce_terrain* ce_terrain_new(const char* zone_name)
{
	ce_terrain* terrain = ce_alloc_zero(sizeof(ce_terrain));
	if (NULL == terrain) {
		ce_logging_error("terrain: could not allocate memory");
		return NULL;
	}

	if (!ce_terrain_new_impl(terrain, zone_name)) {
		ce_terrain_del(terrain);
		return NULL;
	}

	return terrain;
}

void ce_terrain_del(ce_terrain* terrain)
{
	if (NULL != terrain) {
		if (NULL != terrain->textures) {
			for (int i = 0, n = ce_vector_count(terrain->textures); i < n; ++i) {
				ce_texmng_release_texture(ce_root_get_texmng(),
										ce_vector_at(terrain->textures, i));
			}
		}
		ce_rendlayer_del(terrain->rendlayer);
		ce_vector_del(terrain->textures);
		ce_free(terrain, sizeof(ce_terrain));
	}
}
