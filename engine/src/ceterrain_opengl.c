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

#include <limits.h>
#include <assert.h>

#include <GL/gl.h>

#include "celib.h"
#include "cetexture.h"
#include "cerenderitem.h"
#include "cemprhlp.h"
#include "ceterrain.h"

typedef struct {
	GLuint id;
} ce_terrain_renderitem;

static void ce_terrain_renderitem_ctor(ce_renderitem* renderitem, va_list args)
{
	ce_terrain_renderitem* terrain_renderitem =
		(ce_terrain_renderitem*)renderitem->impl;

	ce_terrain* terrain = va_arg(args, ce_terrain*);
	int sector_x = va_arg(args, int);
	int sector_z = va_arg(args, int);
	ce_mprfile_vertex* vertices = va_arg(args, ce_mprfile_vertex*);
	uint16_t* textures = va_arg(args, uint16_t*);
	int16_t* water_allow = va_arg(args, int16_t*);

	ce_mprhlp_get_aabb(&renderitem->aabb, terrain->mprfile, sector_x, sector_z);

	renderitem->position = CE_VEC3_ZERO;
	renderitem->orientation = CE_QUAT_IDENTITY;
	renderitem->bbox.aabb = renderitem->aabb;
	renderitem->bbox.axis = CE_QUAT_IDENTITY;
	renderitem->transparent = NULL != water_allow;

	const float offset_xz_coef = 1.0f / (INT8_MAX - INT8_MIN);
	const float y_coef = terrain->mprfile->max_y / (UINT16_MAX - 0);

	const float tile_uv_step = 1.0f / 8.0f;
	const float tile_uv_half_step = 1.0f / 16.0f;

	const float tile_border_size = 8.0f; // in pixels
	const float tile_uv_border_offset = tile_border_size /
										terrain->mprfile->texture_size;

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

	glNewList(terrain_renderitem->id = glGenLists(1), GL_COMPILE);

	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);

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

			ce_texture_bind(terrain->textures->items[
							ce_mprhlp_texture_number(texture)]);

			for (int i = 0; i < 2; ++i) {
				glBegin(GL_TRIANGLE_STRIP);
				for (int j = 0; j < 6; ++j) {
					glTexCoord2fv(texcoord_array + 2 * indices[i][j]);
					glNormal3fv(normal_array + 3 * indices[i][j]);
					glVertex3fv(vertex_array + 3 * indices[i][j]);
				}
				glEnd();
			}

			ce_texture_unbind(terrain->textures->items[
							ce_mprhlp_texture_number(texture)]);
		}
	}

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glPopAttrib();

	glEndList();
}

static void ce_terrain_renderitem_dtor(ce_renderitem* renderitem)
{
	ce_terrain_renderitem* terrain_renderitem =
		(ce_terrain_renderitem*)renderitem->impl;

	glDeleteLists(terrain_renderitem->id, 1);
}

static void ce_terrain_renderitem_render(ce_renderitem* renderitem)
{
	ce_terrain_renderitem* terrain_renderitem =
		(ce_terrain_renderitem*)renderitem->impl;

	glCallList(terrain_renderitem->id);
}

static const ce_renderitem_vtable ce_terrain_renderitem_vtable = {
	ce_terrain_renderitem_ctor, ce_terrain_renderitem_dtor,
	NULL, ce_terrain_renderitem_render, NULL
};

static void ce_terrain_create_sector(ce_terrain* terrain,
									int sector_x, int sector_z,
									ce_mprfile_sector* sector,
									bool opacity)
{
	// skip empty geometry
	if (!opacity && NULL == sector->water_allow) {
		return;
	}

	ce_renderlayer* renderlayer = ce_renderlayer_new(
		ce_mprhlp_create_material(terrain->mprfile, !opacity,
									terrain->stub_texture));
	ce_renderlayer_add_renderitem(renderlayer, ce_renderitem_new(
		ce_terrain_renderitem_vtable, sizeof(ce_terrain_renderitem),
		terrain, sector_x, sector_z,
		opacity ? sector->land_vertices : sector->water_vertices,
		opacity ? sector->land_textures : sector->water_textures,
		opacity ? NULL : sector->water_allow));

	ce_scenenode* scenenode = ce_scenenode_new(terrain->scenenode);
	ce_scenenode_add_renderlayer(scenenode, renderlayer);
}

bool ce_terrain_create(ce_terrain* terrain)
{
	for (int z = 0, z_count = terrain->mprfile->sector_z_count; z < z_count; ++z) {
		for (int x = 0, x_count = terrain->mprfile->sector_x_count; x < x_count; ++x) {
			ce_mprfile_sector* sector = terrain->mprfile->sectors + z * x_count + x;
			ce_terrain_create_sector(terrain, x, z, sector, true);
			ce_terrain_create_sector(terrain, x, z, sector, false);
		}
	}
	return true;
}
