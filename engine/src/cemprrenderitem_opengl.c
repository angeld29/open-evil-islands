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
#include <limits.h>
#include <assert.h>

#include <GL/gl.h>

#include "cegl.h"
#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cemprhlp.h"
#include "cetexture.h"
#include "cemprrenderitem.h"

// simple & fast triangulated geometry

typedef struct {
	GLuint list;
} ce_mprrenderitem_fast;

static void ce_mprrenderitem_fast_ctor(ce_renderitem* renderitem, va_list args)
{
	ce_mprrenderitem_fast* mprrenderitem =
		(ce_mprrenderitem_fast*)renderitem->impl;

	ce_mprfile* mprfile = va_arg(args, ce_mprfile*);
	int sector_x = va_arg(args, int);
	int sector_z = va_arg(args, int);
	int water = va_arg(args, int);

	ce_mprsector* sector = mprfile->sectors + sector_z *
							mprfile->sector_x_count + sector_x;

	ce_mprvertex* vertices = water ? sector->water_vertices : sector->land_vertices;
	int16_t* water_allow = water ? sector->water_allow : NULL;

	const float offset_xz_coef = 1.0f / (INT8_MAX - INT8_MIN);
	const float y_coef = mprfile->max_y / (UINT16_MAX - 0);

	/**
	 *  Sector rendering: just simple triangle strips!
	 *
	 *   0___1___2__...__32
	 *   |\  |\  |\  |\  |
	 *   | \ | \ | \ | \ | --->
	 *  1|__\|__\|__\|__\|
	 *   |\  |\  |\  |\  |
	 *  .| \ | \ | \ | \ | --->
	 *  .|__\|__\|__\|__\|
	 *  .|\  |\  |\  |\  |
	 *   | \ | \ | \ | \ | --->
	 *   |__\|__\|__\|__\|
	 *  32
	*/

	float normal[3];

	glNewList(mprrenderitem->list = glGenLists(1), GL_COMPILE);

	for (int i = 1; i < CE_MPRFILE_VERTEX_SIDE; ++i) {
		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j < 2 * CE_MPRFILE_VERTEX_SIDE; ++j) {
			int z = i - j % 2;
			int x = j / 2;

			if (NULL != water_allow) {
				/**
				 *  This fu... clever code needs to remove some
				 *  not allowed water triangles.
				 *
				 *  1 tile = 9 vertices = 8 triangles
				 *  1/2 tile = 6 vertices = 4 triangles
				 *
				 *  For performance reasons, I render 1/2 each tile
				 *  in continuous strip. If this half of tile is not
				 *  allowed, break the strip and remove last one entirely.
				*/

				// map 33x33 vertices to 16x16 tiles
				int tz = (i - 1) / 2;

				int tx_cur = x / 2;
				int tx_prev = 1 == x % 2 ? -1 : tx_cur - 1;

				tx_cur = ce_min(tx_cur, CE_MPRFILE_TEXTURE_SIDE - 1);
				tx_prev = -1 == tx_prev ? tx_cur : tx_prev;

				if (-1 == water_allow[tz * CE_MPRFILE_TEXTURE_SIDE + tx_prev] &&
						-1 == water_allow[tz * CE_MPRFILE_TEXTURE_SIDE + tx_cur]) {
					glEnd();
					glBegin(GL_TRIANGLE_STRIP);
					continue;
				}
			}

			ce_mprvertex* vertex = vertices + z * CE_MPRFILE_VERTEX_SIDE + x;

			// note that opengl's textures are bottom to top
			glTexCoord2f(x / (float)(CE_MPRFILE_VERTEX_SIDE - 1),
				(CE_MPRFILE_VERTEX_SIDE - 1 - z) / (float)(CE_MPRFILE_VERTEX_SIDE - 1));

			ce_mprhlp_normal2vector(normal, vertex->normal);
			normal[2] = -normal[2];
			glNormal3fv(normal);

			glVertex3f(x + sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) +
				offset_xz_coef * vertex->offset_x, y_coef * vertex->coord_y,
				-1.0f * (z + sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) +
				offset_xz_coef * vertex->offset_z));
		}
		glEnd();
	}

	glEndList();
}

static void ce_mprrenderitem_fast_dtor(ce_renderitem* renderitem)
{
	ce_mprrenderitem_fast* mprrenderitem =
		(ce_mprrenderitem_fast*)renderitem->impl;

	glDeleteLists(mprrenderitem->list, 1);
}

static void ce_mprrenderitem_fast_render(ce_renderitem* renderitem)
{
	ce_mprrenderitem_fast* mprrenderitem =
		(ce_mprrenderitem_fast*)renderitem->impl;

	glCallList(mprrenderitem->list);
}

// classic tiling

typedef struct {
	GLuint list;
} ce_mprrenderitem_tile;

static void ce_mprrenderitem_tile_ctor(ce_renderitem* renderitem, va_list args)
{
	ce_mprrenderitem_tile* mprrenderitem =
		(ce_mprrenderitem_tile*)renderitem->impl;

	ce_mprfile* mprfile = va_arg(args, ce_mprfile*);
	int sector_x = va_arg(args, int);
	int sector_z = va_arg(args, int);
	int water = va_arg(args, int);
	ce_vector* tile_textures = va_arg(args, ce_vector*);

	ce_mprsector* sector = mprfile->sectors + sector_z *
							mprfile->sector_x_count + sector_x;

	ce_mprvertex* vertices = water ? sector->water_vertices : sector->land_vertices;
	uint16_t* textures = water ? sector->water_textures : sector->land_textures;
	int16_t* water_allow = water ? sector->water_allow : NULL;

	const float offset_xz_coef = 1.0f / (INT8_MAX - INT8_MIN);
	const float y_coef = mprfile->max_y / (UINT16_MAX - 0);

	const float tile_uv_step = 1.0f / 8.0f;
	const float tile_uv_half_step = 1.0f / 16.0f;

	const float tile_border_size = 8.0f; // in pixels
	const float tile_uv_border_offset = tile_border_size /
										mprfile->texture_size;

	/**
	 *  Tile texturing:
	 *
	 *  [                 ] tile_uv_step
	 *  [        ] tile_uv_half_step
	 *    _______ _______
	 *   |  _____|_____  |
	 *   | |     |     | |
	 *   |_|_____|_____|_|
	 *   | |     |     | |
	 *   | |_____|_____| |
	 *   |_______|_______|
	 *
	 *  [  ] tile_uv_border_offset
	*/

	const float texcoord[10][2] = {
		{ tile_uv_half_step, tile_uv_half_step },
		{ tile_uv_border_offset, tile_uv_step - tile_uv_border_offset },
		{ tile_uv_half_step, tile_uv_step - tile_uv_border_offset },
		{ tile_uv_step - tile_uv_border_offset, tile_uv_step - tile_uv_border_offset },
		{ tile_uv_step - tile_uv_border_offset, tile_uv_half_step },
		{ tile_uv_step - tile_uv_border_offset, tile_uv_border_offset },
		{ tile_uv_half_step, tile_uv_border_offset },
		{ tile_uv_border_offset, tile_uv_border_offset },
		{ tile_uv_border_offset, tile_uv_half_step },
		{ tile_uv_border_offset, tile_uv_step - tile_uv_border_offset }
	};

	/**
	 *  Tile rendering: just simple triangle fan!
	 *
	 *    7___6___5
	 *    |\  |  /|
	 *    | \ | / |
	 *   8|__\|/__|4
	 *    |  /0\  |
	 *    | / | \ |
	 *    |/__|__\|
	 *   1/9  2   3
	*/

	const int offset_x[10] = { 1, 0, 1, 2, 2, 2, 1, 0, 0, 0 };
	const int offset_z[10] = { 1, 0, 0, 0, 1, 2, 2, 2, 1, 0 };

	float normal[3];

	glNewList(mprrenderitem->list = glGenLists(1), GL_COMPILE);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);

	for (int z = 0; z < CE_MPRFILE_VERTEX_SIDE - 2; z += 2) {
		for (int x = 0; x < CE_MPRFILE_VERTEX_SIDE - 2; x += 2) {
			if (NULL != water_allow &&
					-1 == water_allow[z / 2 * CE_MPRFILE_TEXTURE_SIDE + x / 2]) {
				continue;
			}

			uint16_t texture = textures[z / 2 * CE_MPRFILE_TEXTURE_SIDE + x / 2];

			int texture_index = ce_mprhlp_texture_index(texture);
			float u = (texture_index - texture_index / 8 * 8) / 8.0f;
			float v = (7 - texture_index / 8) / 8.0f; // bottom to top

			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glTranslatef(u + tile_uv_half_step, v + tile_uv_half_step, 0.0f);
			glRotatef(-90.0f * ce_mprhlp_texture_angle(texture), 0.0f, 0.0f, 1.0f);
			glTranslatef(-u - tile_uv_half_step, -v - tile_uv_half_step, 0.0f);
			glMatrixMode(GL_MODELVIEW);

			ce_texture_bind(tile_textures->items[ce_mprhlp_texture_number(texture)]);

			glBegin(GL_TRIANGLE_FAN);
			for (int i = 0; i < 10; ++i) {
				ce_mprvertex* vertex = vertices + (z + offset_z[i]) *
					CE_MPRFILE_VERTEX_SIDE + (x + offset_x[i]);

				glTexCoord2f(u + texcoord[i][0], v + texcoord[i][1]);

				ce_mprhlp_normal2vector(normal, vertex->normal);
				normal[2] = -normal[2];
				glNormal3fv(normal);

				glVertex3f(x + offset_x[i] +
					sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) +
					offset_xz_coef * vertex->offset_x,
					y_coef * vertex->coord_y, -1.0f * (z + offset_z[i] +
					sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) +
					offset_xz_coef * vertex->offset_z));
			}
			glEnd();

			ce_texture_unbind(tile_textures->items[ce_mprhlp_texture_number(texture)]);
		}
	}

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEndList();
}

static void ce_mprrenderitem_tile_dtor(ce_renderitem* renderitem)
{
	ce_mprrenderitem_tile* mprrenderitem =
		(ce_mprrenderitem_tile*)renderitem->impl;

	glDeleteLists(mprrenderitem->list, 1);
}

static void ce_mprrenderitem_tile_render(ce_renderitem* renderitem)
{
	ce_mprrenderitem_tile* mprrenderitem =
		(ce_mprrenderitem_tile*)renderitem->impl;

	glCallList(mprrenderitem->list);
}

// HW tessellated geometry

typedef struct {
	int index_count;
	GLuint vertex_buffer;
	GLuint index_buffer;
	GLhandle program;
} ce_mprrenderitem_hwtess;

static void ce_mprrenderitem_hwtess_ctor(ce_renderitem* renderitem, va_list args)
{
	ce_mprrenderitem_hwtess* mprrenderitem =
		(ce_mprrenderitem_hwtess*)renderitem->impl;

	ce_mprfile* mprfile = va_arg(args, ce_mprfile*);
	int sector_x = va_arg(args, int);
	int sector_z = va_arg(args, int);
	int water = va_arg(args, int);

	ce_mprsector* sector = mprfile->sectors + sector_z *
							mprfile->sector_x_count + sector_x;

	ce_mprvertex* mprvertices = water ? sector->water_vertices :
										sector->land_vertices;
	int16_t* water_allow = water ? sector->water_allow : NULL;

	const float offset_xz_coef = 1.0f / (INT8_MAX - INT8_MIN);
	const float y_coef = mprfile->max_y / (UINT16_MAX - 0);

	mprrenderitem->index_count = 4;

	const int vb_size = 3 * sizeof(float) * 4;
	const int ib_size = sizeof(GLushort) * mprrenderitem->index_count;

	ce_gl_gen_buffers(1, &mprrenderitem->vertex_buffer);
	ce_gl_gen_buffers(1, &mprrenderitem->index_buffer);

	ce_gl_bind_buffer(CE_GL_ARRAY_BUFFER, mprrenderitem->vertex_buffer);
	ce_gl_buffer_data(CE_GL_ARRAY_BUFFER, vb_size, NULL, CE_GL_STATIC_DRAW);
	float* vertices = ce_gl_map_buffer(CE_GL_ARRAY_BUFFER, CE_GL_WRITE_ONLY);

	ce_gl_bind_buffer(CE_GL_ELEMENT_ARRAY_BUFFER, mprrenderitem->index_buffer);
	ce_gl_buffer_data(CE_GL_ELEMENT_ARRAY_BUFFER, ib_size, NULL, CE_GL_STATIC_DRAW);
	GLushort* indices = ce_gl_map_buffer(CE_GL_ELEMENT_ARRAY_BUFFER, CE_GL_WRITE_ONLY);

	*vertices++ = sector_x * (CE_MPRFILE_VERTEX_SIDE - 1);
	*vertices++ = 0.0f;
	*vertices++ = -1.0f * (sector_z * (CE_MPRFILE_VERTEX_SIDE - 1));

	*vertices++ = sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) + (CE_MPRFILE_VERTEX_SIDE - 1);
	*vertices++ = 0.0f;
	*vertices++ = -1.0f * (sector_z * (CE_MPRFILE_VERTEX_SIDE - 1));

	*vertices++ = sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) + (CE_MPRFILE_VERTEX_SIDE - 1);
	*vertices++ = 0.0f;
	*vertices++ = -1.0f * (sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) + (CE_MPRFILE_VERTEX_SIDE - 1));

	*vertices++ = sector_x * (CE_MPRFILE_VERTEX_SIDE - 1);
	*vertices++ = 0.0f;
	*vertices++ = -1.0f * (sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) + (CE_MPRFILE_VERTEX_SIDE - 1));

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 3;

	ce_gl_unmap_buffer(CE_GL_ELEMENT_ARRAY_BUFFER);
	ce_gl_unmap_buffer(CE_GL_ARRAY_BUFFER);

	// FIXME: hard coded !!!
	FILE* vert_file = fopen("engine/src/cemprrenderitem_hwtess_vertex.glsl", "rb");
	FILE* frag_file = fopen("engine/src/cemprrenderitem_hwtess_fragment_land.glsl", "rb");

	char buffer[8192];
	const char* buf = buffer;
	GLint result;

	memset(buffer, '\0', sizeof(buffer));
	fread(buffer, 1, sizeof(buffer), vert_file);

	GLhandle vertex_object = ce_gl_create_shader_object(CE_GL_VERTEX_SHADER);
	ce_gl_shader_source(vertex_object, 1, &buf, NULL);
	ce_gl_compile_shader(vertex_object);

	ce_gl_get_object_parameter_iv(vertex_object, CE_GL_OBJECT_COMPILE_STATUS, &result);
	if (0 == result) {
		GLsizei length;
		ce_gl_get_info_log(vertex_object, sizeof(buffer), &length, buffer);
		ce_logging_error("mprrenderitem: %s", buffer);
		abort();
	}

	memset(buffer, '\0', sizeof(buffer));
	fread(buffer, 1, sizeof(buffer), frag_file);

	fclose(frag_file);
	fclose(vert_file);

	GLhandle fragment_object = ce_gl_create_shader_object(CE_GL_FRAGMENT_SHADER);
	ce_gl_shader_source(fragment_object, 1, &buf, NULL);
	ce_gl_compile_shader(fragment_object);

	ce_gl_get_object_parameter_iv(fragment_object, CE_GL_OBJECT_COMPILE_STATUS, &result);
	if (0 == result) {
		GLsizei length;
		ce_gl_get_info_log(fragment_object, sizeof(buffer), &length, buffer);
		ce_logging_error("mprrenderitem: %s", buffer);
		abort();
	}

	mprrenderitem->program = ce_gl_create_program_object();

	ce_gl_attach_object(mprrenderitem->program, vertex_object);
	ce_gl_attach_object(mprrenderitem->program, fragment_object);

	ce_gl_delete_object(fragment_object);
	ce_gl_delete_object(vertex_object);

	ce_gl_link_program_object(mprrenderitem->program);

	ce_gl_get_object_parameter_iv(mprrenderitem->program, CE_GL_OBJECT_LINK_STATUS, &result);
	if (0 == result) {
		GLsizei length;
		ce_gl_get_info_log(mprrenderitem->program, sizeof(buffer), &length, buffer);
		ce_logging_error("mprrenderitem: %s", buffer);
		abort();
	}

	ce_gl_use_program_object(mprrenderitem->program);

	ce_gl_uniform_2fv(ce_gl_get_uniform_location(mprrenderitem->program,
		"uv_lookup_table"), 4, (GLfloat[]){ 0.0f, 0.0f, 1.0f, 0.0f,
											1.0f, 1.0f, 0.0f, 1.0f });

	ce_gl_vst_set_tessellation_factor(10.0f);
	ce_gl_vst_set_tessellation_mode(CE_GL_VST_CONTINUOUS);
	//CE_GL_VST_DISCRETE
}

static void ce_mprrenderitem_hwtess_dtor(ce_renderitem* renderitem)
{
	ce_mprrenderitem_hwtess* mprrenderitem =
		(ce_mprrenderitem_hwtess*)renderitem->impl;

	ce_gl_delete_object(mprrenderitem->program);
	ce_gl_delete_buffers(1, &mprrenderitem->index_buffer);
	ce_gl_delete_buffers(1, &mprrenderitem->vertex_buffer);
}

static void ce_mprrenderitem_hwtess_render(ce_renderitem* renderitem)
{
	ce_mprrenderitem_hwtess* mprrenderitem =
		(ce_mprrenderitem_hwtess*)renderitem->impl;

	ce_gl_use_program_object(mprrenderitem->program);

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	glPolygonMode(GL_FRONT, GL_LINE);

	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);
	//glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	ce_gl_bind_buffer(CE_GL_ARRAY_BUFFER, mprrenderitem->vertex_buffer);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	ce_gl_bind_buffer(CE_GL_ELEMENT_ARRAY_BUFFER, mprrenderitem->index_buffer);
	glDrawElements(GL_QUADS, mprrenderitem->index_count, GL_UNSIGNED_SHORT, NULL);

	ce_gl_bind_buffer(CE_GL_ELEMENT_ARRAY_BUFFER, 0);
	ce_gl_bind_buffer(CE_GL_ARRAY_BUFFER, 0);

	glPopClientAttrib();

	ce_gl_use_program_object(0);
}

ce_renderitem* ce_mprrenderitem_new(ce_mprfile* mprfile, bool tiling,
									int sector_x, int sector_z,
									int water, ce_vector* tile_textures)
{
	if (tiling) {
		// tiling? no speed, no tesselation...
		ce_renderitem_vtable ce_mprrenderitem_tile_vtable = {
			ce_mprrenderitem_tile_ctor, ce_mprrenderitem_tile_dtor,
			NULL, ce_mprrenderitem_tile_render, NULL
		};
		return ce_renderitem_new(ce_mprrenderitem_tile_vtable,
							sizeof(ce_mprrenderitem_tile), mprfile,
							sector_x, sector_z, water, tile_textures);
	}

	if (ce_gl_query_feature(CE_GL_FEATURE_VERTEX_BUFFER_OBJECT) &&
			// also includes shader objects
			ce_gl_query_feature(CE_GL_FEATURE_VERTEX_SHADER_TESSELLATOR)) {
		ce_renderitem_vtable ce_renderitem_hwtess_vtable = {
			ce_mprrenderitem_hwtess_ctor, ce_mprrenderitem_hwtess_dtor,
			NULL, ce_mprrenderitem_hwtess_render, NULL
		};
		return ce_renderitem_new(ce_renderitem_hwtess_vtable,
								sizeof(ce_mprrenderitem_hwtess), mprfile,
								sector_x, sector_z, water, tile_textures);
	}

	ce_renderitem_vtable ce_renderitem_fast_vtable = {
		ce_mprrenderitem_fast_ctor, ce_mprrenderitem_fast_dtor,
		NULL, ce_mprrenderitem_fast_render, NULL
	};
	return ce_renderitem_new(ce_renderitem_fast_vtable,
							sizeof(ce_mprrenderitem_fast), mprfile,
							sector_x, sector_z, water, tile_textures);
}
