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

#include "cegl.h"
#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cemprhlp.h"
#include "cetexture.h"
#include "ceshadermng.h"
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

// tessellated geometry by AMD vertex shader tessellator

enum {
	CE_HWTESS_SAMPLER_COUNT = 3
};

typedef struct {
	GLsizei vertex_count;
	GLuint vertex_buffer;
	GLuint buffers[CE_HWTESS_SAMPLER_COUNT];
	GLuint textures[CE_HWTESS_SAMPLER_COUNT];
	GLuint program;
} ce_mprrenderitem_hwtess;

// TODO: share it
static void ce_mprrenderitem_hwtess_print_log(GLuint object)
{
	GLint length;
	if (glIsShader(object)) {
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
	} else {
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
	}

	char buffer[length];
	if (glIsShader(object)) {
		glGetShaderInfoLog(object, length, NULL, buffer);
	} else {
		glGetProgramInfoLog(object, length, NULL, buffer);
	}

	ce_logging_error("mprrenderitem: %s", buffer);
}

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

	mprrenderitem->vertex_count = 6;

	// needed for first tessellation pass
	const int vertex_offsets[][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 },
										{ 0, 0 }, { 1, 1 }, { 0, 1 } };

	glGenBuffers(1, &mprrenderitem->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, mprrenderitem->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) *
		mprrenderitem->vertex_count, NULL, GL_STATIC_DRAW);

	float* vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	for (GLsizei i = 0; i < mprrenderitem->vertex_count; ++i) {
		*vertices++ = sector_x * (CE_MPRFILE_VERTEX_SIDE - 1) +
						vertex_offsets[i][0] * (CE_MPRFILE_VERTEX_SIDE - 1);
		*vertices++ = 0.0f;
		*vertices++ = -1.0f * (sector_z * (CE_MPRFILE_VERTEX_SIDE - 1) +
						vertex_offsets[i][1] * (CE_MPRFILE_VERTEX_SIDE - 1));
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLint max_texture_buffer_size;
	glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &max_texture_buffer_size);

	glGenBuffers(CE_HWTESS_SAMPLER_COUNT, mprrenderitem->buffers);
	glGenTextures(CE_HWTESS_SAMPLER_COUNT, mprrenderitem->textures);

	GLsizeiptr buffer_sizes[CE_HWTESS_SAMPLER_COUNT] = { 3, 2, 1 };
	float* elements[CE_HWTESS_SAMPLER_COUNT];

	for (int i = 0; i < CE_HWTESS_SAMPLER_COUNT; ++i) {
		glBindBuffer(GL_TEXTURE_BUFFER, mprrenderitem->buffers[i]);
		glBufferData(GL_TEXTURE_BUFFER, buffer_sizes[i] * sizeof(float) *
			CE_MPRFILE_VERTEX_COUNT, NULL, GL_STATIC_DRAW);

		// sampler 0 reserved by AMD vertex
		glActiveTexture(GL_TEXTURE1 + i);
		glBindTexture(GL_TEXTURE_BUFFER, mprrenderitem->textures[i]);

		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, mprrenderitem->buffers[i]);

		glBindTexture(GL_TEXTURE_BUFFER, 0);
		glActiveTexture(GL_TEXTURE0);

		elements[i] = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
	}

	float* normals = elements[0];
	float* xz_offsets = elements[1];
	float* height_map = elements[2];

	for (int z = 0; z < CE_MPRFILE_VERTEX_SIDE; ++z) {
		for (int x = 0; x < CE_MPRFILE_VERTEX_SIDE; ++x) {
			ce_mprvertex* mprvertex = mprvertices + z * CE_MPRFILE_VERTEX_SIDE + x;

			ce_mprhlp_normal2vector(normals, mprvertex->normal);
			normals[2] = -normals[2];
			normals += 3;

			*xz_offsets++ = offset_xz_coef * mprvertex->offset_x;
			*xz_offsets++ = offset_xz_coef * mprvertex->offset_z;

			*height_map++ = y_coef * mprvertex->coord_y;
		}
	}

	for (int i = 0; i < CE_HWTESS_SAMPLER_COUNT; ++i) {
		glBindBuffer(GL_TEXTURE_BUFFER, mprrenderitem->buffers[i]);
		glUnmapBuffer(GL_TEXTURE_BUFFER);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);
	}

	const char* vert_source = ce_shadermng_find_source("mpramdvst_vert");
	const char* frag_source = ce_shadermng_find_source(water ? "mprwater_frag" :
																"mprland_frag");

	assert(NULL != vert_source);
	assert(NULL != frag_source);

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vert_source, NULL);
	glCompileShader(vertex_shader);

	GLint result;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &result);
	if (0 == result) {
		ce_mprrenderitem_hwtess_print_log(vertex_shader);
		abort(); // hmmm...
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &frag_source, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &result);
	if (0 == result) {
		ce_mprrenderitem_hwtess_print_log(fragment_shader);
		abort(); // hmmm...
	}

	mprrenderitem->program = glCreateProgram();

	glAttachShader(mprrenderitem->program, vertex_shader);
	glAttachShader(mprrenderitem->program, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glLinkProgram(mprrenderitem->program);

	glGetProgramiv(mprrenderitem->program, GL_LINK_STATUS, &result);
	if (0 == result) {
		ce_mprrenderitem_hwtess_print_log(mprrenderitem->program);
		abort(); // hmmm...
	}

	glUseProgram(mprrenderitem->program);

	glUniform1i(glGetUniformLocation(mprrenderitem->program, "vertices"), 0);
	glUniform1i(glGetUniformLocation(mprrenderitem->program, "normals"), 1);
	glUniform1i(glGetUniformLocation(mprrenderitem->program, "xz_offsets"), 2);
	glUniform1i(glGetUniformLocation(mprrenderitem->program, "height_map"), 3);

	glUniform1f(glGetUniformLocation(mprrenderitem->program,
		"vertex_side"), CE_MPRFILE_VERTEX_SIDE);
	glUniform1f(glGetUniformLocation(mprrenderitem->program,
		"vertex_count"), CE_MPRFILE_VERTEX_COUNT);

	glUniform1f(glGetUniformLocation(mprrenderitem->program,
		"sector_x"), sector_x);
	glUniform1f(glGetUniformLocation(mprrenderitem->program,
		"sector_z"), sector_z);

	glUniform1f(glGetUniformLocation(mprrenderitem->program,
		"vertex_side_offset_inv"), 1.0f / (CE_MPRFILE_VERTEX_SIDE - 1));

	glUniform1f(glGetUniformLocation(mprrenderitem->program,
		"sector_x_offset"), sector_x * (CE_MPRFILE_VERTEX_SIDE - 1));
	glUniform1f(glGetUniformLocation(mprrenderitem->program,
		"sector_z_offset"), sector_z * (CE_MPRFILE_VERTEX_SIDE - 1));

	glTessellationFactorAMD(15.0f);
	glTessellationModeAMD(GL_DISCRETE_AMD);

	glUseProgram(0);
}

static void ce_mprrenderitem_hwtess_dtor(ce_renderitem* renderitem)
{
	ce_mprrenderitem_hwtess* mprrenderitem =
		(ce_mprrenderitem_hwtess*)renderitem->impl;

	glDeleteProgram(mprrenderitem->program);
	glDeleteTextures(CE_HWTESS_SAMPLER_COUNT, mprrenderitem->textures);
	glDeleteBuffers(CE_HWTESS_SAMPLER_COUNT, mprrenderitem->buffers);
	glDeleteBuffers(1, &mprrenderitem->vertex_buffer);
}

static void ce_mprrenderitem_hwtess_render(ce_renderitem* renderitem)
{
	ce_mprrenderitem_hwtess* mprrenderitem =
		(ce_mprrenderitem_hwtess*)renderitem->impl;

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, mprrenderitem->vertex_buffer);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	for (int i = 0; i < CE_HWTESS_SAMPLER_COUNT; ++i) {
		glActiveTexture(GL_TEXTURE1 + i);
		glBindTexture(GL_TEXTURE_BUFFER, mprrenderitem->textures[i]);
	}

	glUseProgram(mprrenderitem->program);
	glDrawArrays(GL_TRIANGLES, 0, mprrenderitem->vertex_count);
	glUseProgram(0);

	for (int i = 0; i < CE_HWTESS_SAMPLER_COUNT; ++i) {
		glActiveTexture(GL_TEXTURE1 + i);
		glBindTexture(GL_TEXTURE_BUFFER, 0);
	}

	glActiveTexture(GL_TEXTURE0);

	glPopClientAttrib();
}

ce_renderitem* ce_mprrenderitem_new(ce_mprfile* mprfile, bool tiling,
									int sector_x, int sector_z,
									int water, ce_vector* tile_textures)
{
	if (tiling) {
		// tiling? no speed...
		ce_renderitem_vtable ce_mprrenderitem_tile_vtable = {
			ce_mprrenderitem_tile_ctor, ce_mprrenderitem_tile_dtor,
			NULL, ce_mprrenderitem_tile_render, NULL
		};
		return ce_renderitem_new(ce_mprrenderitem_tile_vtable,
							sizeof(ce_mprrenderitem_tile), mprfile,
							sector_x, sector_z, water, tile_textures);
	}

	if (GLEW_VERSION_3_1 && GLEW_AMD_vertex_shader_tessellator) {
		// use AMD tessellation shader!
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
