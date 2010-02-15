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
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cebyteorder.h"
#include "celogging.h"
#include "cealloc.h"
#include "cemath.h"
#include "cevec3.h"
#include "cememfile.h"
#include "cefigfile.h"

static const unsigned int CE_FIGFILE_SIGNATURE_FIG8 = 0x38474946;

static void ce_figfile_init_vec2(ce_vec2* vec, float* v, int n)
{
	for (int i = 0; i < n; ++i, ++vec, v += 2) {
		ce_vec2_init_vector(vec, v);
	}
}

static void ce_figfile_init_vec3(ce_vec3* vec, float* v, int n)
{
	for (int i = 0; i < n; ++i, ++vec, v += 3) {
		ce_vec3_init_vector(vec, v);
	}
}

static void ce_figfile_init_vertex(ce_figfile_vertex* ver, float* v, int n)
{
	for (int i = 0; i < n; ++i, ++ver) {
		memcpy(ver->x, v, sizeof(float) * 4 * 8); v += 4 * 8;
		memcpy(ver->y, v, sizeof(float) * 4 * 8); v += 4 * 8;
		memcpy(ver->z, v, sizeof(float) * 4 * 8); v += 4 * 8;
	}
}

static void ce_figfile_init_normal(ce_figfile_normal* nor, float* v, int n)
{
	for (int i = 0; i < n; ++i, ++nor) {
		memcpy(nor->x, v, sizeof(float) * 4); v += 4;
		memcpy(nor->y, v, sizeof(float) * 4); v += 4;
		memcpy(nor->z, v, sizeof(float) * 4); v += 4;
		memcpy(nor->w, v, sizeof(float) * 4); v += 4;
	}
}

static void
ce_figfile_init_component2(ce_figfile_component2* cmp, uint16_t* v, int n)
{
	for (int i = 0; i < n; ++i, ++cmp) {
		cmp->a = ce_le2cpu16(*v++);
		cmp->b = ce_le2cpu16(*v++);
	}
}

static void
ce_figfile_init_component3(ce_figfile_component3* cmp, uint16_t* v, int n)
{
	for (int i = 0; i < n; ++i, ++cmp) {
		cmp->a = ce_le2cpu16(*v++);
		cmp->b = ce_le2cpu16(*v++);
		cmp->c = ce_le2cpu16(*v++);
	}
}

static bool ce_figfile_read_model_data(ce_figfile* fig, ce_memfile* mem)
{
	// TODO: more comments
	/**
	 *  FIG model data:
	 *   vertex_size - 3 items, 128 bytes per item
	 *   normal_size - 4 items, 16 bytes per item
	 *   texcoord_size - 2 items, 4 bytes per item
	 *   index_size - 1 item, 2 bytes per item
	 *   component_size - 3 items, 2 byte per item
	 *   light_component_size - 2 items, 2 bytes per item
	*/

	fig->vertices = ce_alloc(sizeof(ce_figfile_vertex) * fig->vertex_count);
	fig->normals = ce_alloc(sizeof(ce_figfile_normal) * fig->normal_count);
	fig->texcoords = ce_alloc(sizeof(ce_vec2) * fig->texcoord_count);
	fig->indices = ce_alloc(sizeof(unsigned short) * fig->index_count);
	fig->components = ce_alloc(sizeof(ce_figfile_component3) *
										fig->component_count);
	fig->light_components = ce_alloc(sizeof(ce_figfile_component2) *
										fig->light_component_count);

	if (NULL == fig->vertices || NULL == fig->normals ||
			NULL == fig->texcoords || NULL == fig->indices ||
			NULL == fig->components || NULL == fig->light_components) {
		return false;
	}

	size_t vertex_size = 4 * 4 * 8 * 3;
	size_t normal_size = 4 * 4 * 4;
	size_t texcoord_size = 4 * 2;
	size_t index_size = 2;
	size_t component_size = 2 * 3;
	size_t light_component_size = 2 * 2;

	char model_data[ce_smax(vertex_size * fig->vertex_count,
		ce_smax(normal_size * fig->normal_count,
		ce_smax(texcoord_size * fig->texcoord_count,
		ce_smax(index_size * fig->index_count,
		ce_smax(component_size * fig->component_count,
		light_component_size * fig->light_component_count)))))];

	if (fig->vertex_count != ce_memfile_read(mem, model_data,
								vertex_size, fig->vertex_count)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	ce_figfile_init_vertex(fig->vertices, (float*)model_data, fig->vertex_count);

	if (fig->normal_count != ce_memfile_read(mem, model_data,
								normal_size, fig->normal_count)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	ce_figfile_init_normal(fig->normals, (float*)model_data, fig->normal_count);

	if (fig->texcoord_count != ce_memfile_read(mem, model_data,
							texcoord_size, fig->texcoord_count)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	ce_figfile_init_vec2(fig->texcoords, (float*)model_data, fig->texcoord_count);

	if (fig->index_count != ce_memfile_read(mem, model_data,
								index_size, fig->index_count)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	for (int i = 0, n = fig->index_count; i < n; ++i) {
		fig->indices[i] = ce_le2cpu16(i[(uint16_t*)model_data]);
	}

	if (fig->component_count != ce_memfile_read(mem, model_data,
							component_size, fig->component_count)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	ce_figfile_init_component3(fig->components, (uint16_t*)model_data,
													fig->component_count);

	if (fig->light_component_count != ce_memfile_read(mem, model_data,
						light_component_size, fig->light_component_count)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	ce_figfile_init_component2(fig->light_components, (uint16_t*)model_data,
												fig->light_component_count);

	return true;
}

static bool ce_figfile_read_bound_data(ce_figfile* fig, ce_memfile* mem)
{
	/**
	 *  FIG bound data:
	 *   center - 3 floats x 8
	 *   min    - 3 floats x 8
	 *   max    - 3 floats x 8
	 *   radius - 1 float  x 8
	*/

	if (NULL == (fig->center = ce_alloc(sizeof(ce_vec3) * 8)) ||
			NULL == (fig->min = ce_alloc(sizeof(ce_vec3) * 8)) ||
			NULL == (fig->max = ce_alloc(sizeof(ce_vec3) * 8)) ||
			NULL == (fig->radius = ce_alloc(sizeof(float) * 8))) {
		ce_logging_error("figfile: could not allocate memory");
		return false;
	}

	float bound_data[80];
	if (80 != ce_memfile_read(mem, bound_data, sizeof(float), 80)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	ce_figfile_init_vec3(fig->center, bound_data + 0, 8);
	ce_figfile_init_vec3(fig->min, bound_data + 24, 8);
	ce_figfile_init_vec3(fig->max, bound_data + 48, 8);
	memcpy(fig->radius, bound_data + 72, sizeof(float) * 8);

	return true;
}

static bool ce_figfile_read_header(ce_figfile* fig, ce_memfile* mem)
{
	/**
	 *  FIG header:
	 *   signature
	 *   vertex count
	 *   normal count
	 *   texcoord count
	 *   index count
	 *   component count
	 *   light component count
	 *   unknown - always 0; for alignment?..
	 *   unknown1
	 *   unknown2
	*/

	uint32_t signature;
	if (1 != ce_memfile_read(mem, &signature, sizeof(uint32_t), 1)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	ce_le2cpu32s(&signature);
	if (CE_FIGFILE_SIGNATURE_FIG8 != signature) {
		ce_logging_error("figfile: wrong signature");
		return false;
	}

	uint32_t header[9];
	if (9 != ce_memfile_read(mem, header, sizeof(uint32_t), 9)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	assert(0 == ce_le2cpu32(header[6]));

	fig->type = CE_FIGFILE_TYPE_FIG8;
	fig->vertex_count = ce_le2cpu32(header[0]);
	fig->normal_count = ce_le2cpu32(header[1]);
	fig->texcoord_count = ce_le2cpu32(header[2]);
	fig->index_count = ce_le2cpu32(header[3]);
	fig->component_count = ce_le2cpu32(header[4]);
	fig->light_component_count = ce_le2cpu32(header[5]);
	fig->unknown1 = ce_le2cpu32(header[7]);
	fig->unknown2 = ce_le2cpu32(header[8]);

	return true;
}

static bool ce_figfile_open_memfile_impl(ce_figfile* fig, ce_memfile* mem)
{
	return ce_figfile_read_header(fig, mem) &&
		ce_figfile_read_bound_data(fig, mem) &&
		ce_figfile_read_model_data(fig, mem);
}

static ce_figfile* ce_figfile_open_memfile(ce_memfile* mem)
{
	ce_figfile* fig = ce_alloc_zero(sizeof(ce_figfile));
	if (NULL == fig) {
		ce_logging_error("figfile: could not allocate memory");
		return NULL;
	}

	if (!ce_figfile_open_memfile_impl(fig, mem)) {
		ce_figfile_close(fig);
		return NULL;
	}

	return fig;
}

ce_figfile* ce_figfile_open(const char* path)
{
	ce_memfile* mem = ce_memfile_open_path(path, "rb");
	if (NULL == mem) {
		return NULL;
	}

	ce_figfile* fig = ce_figfile_open_memfile(mem);
	ce_memfile_close(mem);

	return fig;
}

void ce_figfile_close(ce_figfile* fig)
{
	if (NULL != fig) {
		ce_free(fig->light_components, sizeof(ce_figfile_component2) *
										fig->light_component_count);
		ce_free(fig->components, sizeof(ce_figfile_component3) *
										fig->component_count);
		ce_free(fig->indices, sizeof(unsigned short) * fig->index_count);
		ce_free(fig->texcoords, sizeof(ce_vec2) * fig->texcoord_count);
		ce_free(fig->normals, sizeof(ce_figfile_normal) * fig->normal_count);
		ce_free(fig->vertices, sizeof(ce_figfile_vertex) * fig->vertex_count);
		ce_free(fig->radius, sizeof(float) * 8);
		ce_free(fig->max, sizeof(ce_vec3) * 8);
		ce_free(fig->min, sizeof(ce_vec3) * 8);
		ce_free(fig->center, sizeof(ce_vec3) * 8);
		ce_free(fig, sizeof(ce_figfile));
	}
}
