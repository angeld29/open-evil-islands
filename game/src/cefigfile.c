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
#include "cefigfile.h"

typedef enum {
	CE_FIGFILE_TYPE_FIG1,
	CE_FIGFILE_TYPE_FIG6,
	CE_FIGFILE_TYPE_FIG8,
	CE_FIGFILE_TYPE_COUNT
} ce_figfile_proto_type;

struct ce_figfile_proto {
	ce_figfile_proto_type type;
	size_t vertex_size;
	size_t normal_size;
	size_t texcoord_size;
	size_t index_size;
	size_t component_size;
	size_t light_component_size;
	uint32_t vertex_count;
	uint32_t normal_count;
	uint32_t texcoord_count;
	uint32_t index_count;
	uint32_t component_count;
	uint32_t light_component_count;
	float* center;
	float* min;
	float* max;
	float* radius;
	float* vertices;
	float* normals;
	float* texcoords;
	uint16_t* indices;
	uint16_t* components;
	uint16_t* light_components;
};

static const unsigned int CE_FIGFILE_SIGNATURE_FIG8 = 0x38474946;

typedef float
(*ce_figfile_proto_value_callback)(const float* params, int stride,
									const ce_complection* cm);

static float
ce_figfile_proto_value_fig1(const float* params, int stride,
							const ce_complection* cm)
{
	ce_unused(stride);
	ce_unused(cm);
	return *params;
}

static float
ce_figfile_proto_value_fig6(const float* params, int stride,
							const ce_complection* cm)
{
	float temp = params[0 * stride] + (params[1 * stride] -
										params[0 * stride]) * cm->strength;
	float value = temp + (params[2 * stride] - temp) * cm->dexterity;
	temp = params[3 * stride] + (params[4 * stride] -
								params[3 * stride]) * cm->strength;
	temp += (params[5 * stride] - temp) * cm->dexterity;
	return value += (temp - value) * cm->height;
}

static float
ce_figfile_proto_value_fig8(const float* params, int stride,
							const ce_complection* cm)
{
	float temp1 = params[0 * stride] + (params[1 * stride] -
										params[0 * stride]) * cm->strength;
	float temp2 = params[2 * stride] + (params[3 * stride] -
										params[2 * stride]) * cm->strength;
	float value = temp1 + (temp2 - temp1) * cm->dexterity;
	temp1 = params[4 * stride] + (params[5 * stride] -
								params[4 * stride]) * cm->strength;
	temp2 = params[6 * stride] + (params[7 * stride] -
								params[6 * stride]) * cm->strength;
	temp1 += (temp2 - temp1) * cm->dexterity;
	return value += (temp1 - value) * cm->height;
}

static const ce_figfile_proto_value_callback
ce_figfile_proto_value_callbacks[CE_FIGFILE_TYPE_COUNT] = {
	ce_figfile_proto_value_fig1,
	ce_figfile_proto_value_fig6,
	ce_figfile_proto_value_fig8
};

static const int ce_figfile_proto_value_count[CE_FIGFILE_TYPE_COUNT] = {
	1,
	6,
	8
};

static bool
ce_figfile_proto_read_model_data(ce_figfile_proto* proto, ce_memfile* mem)
{
	/**
	 *  FIG model data:
	 *   vertex_size - 3 components x n variants x 4 vertices x float size
	 *   normal_size - 4 components x 4 normals x float size
	 *   texcoord_size - 2 components x float size
	 *   index_size - short size
	 *   component_size - 3 components x short size
	 *   light_component_size - 2 components x short size
	*/

	assert(4 == sizeof(float));
	assert(2 == sizeof(uint16_t));

	const size_t value_count = ce_figfile_proto_value_count[proto->type];

	proto->vertex_size = 3 * value_count * 4 * 4;
	proto->normal_size = 4 * 4 * 4;
	proto->texcoord_size = 2 * 4;
	proto->index_size = 2;
	proto->component_size = 3 * 2;
	proto->light_component_size = 2 * 2;

	proto->vertices = ce_alloc(proto->vertex_size * proto->vertex_count);
	proto->normals = ce_alloc(proto->normal_size * proto->normal_count);
	proto->texcoords = ce_alloc(proto->texcoord_size * proto->texcoord_count);
	proto->indices = ce_alloc(proto->index_size * proto->index_count);
	proto->components = ce_alloc(proto->component_size *
								proto->component_count);
	proto->light_components = ce_alloc(proto->light_component_size *
										proto->light_component_count);

	if (NULL == proto->vertices || NULL == proto->normals ||
			NULL == proto->texcoords || NULL == proto->indices ||
			NULL == proto->components || NULL == proto->light_components) {
		ce_logging_error("figfile: could not allocate memory");
		return false;
	}

	if (proto->vertex_count != ce_memfile_read(mem,
									proto->vertices,
									proto->vertex_size,
									proto->vertex_count) ||
			proto->normal_count != ce_memfile_read(mem,
									proto->normals,
									proto->normal_size,
									proto->normal_count) ||
			proto->texcoord_count != ce_memfile_read(mem,
									proto->texcoords,
									proto->texcoord_size,
									proto->texcoord_count) ||
			proto->index_count != ce_memfile_read(mem,
									proto->indices,
									proto->index_size,
									proto->index_count) ||
			proto->component_count != ce_memfile_read(mem,
										proto->components,
										proto->component_size,
										proto->component_count) ||
			proto->light_component_count != ce_memfile_read(mem,
											proto->light_components,
											proto->light_component_size,
											proto->light_component_count)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	for (int i = 0, n = proto->index_count; i < n; ++i) {
		ce_le2cpu16s(proto->indices + i);
	}

	for (int i = 0, n = 3 * proto->component_count; i < n; ++i) {
		ce_le2cpu16s(proto->components + i);
	}

	for (int i = 0, n = 2 * proto->light_component_count; i < n; ++i) {
		ce_le2cpu16s(proto->light_components + i);
	}

	return true;
}

static bool
ce_figfile_proto_read_bound_data(ce_figfile_proto* proto, ce_memfile* mem)
{
	/**
	 *  FIG bound data:
	 *   center - 3 floats x n
	 *   min    - 3 floats x n
	 *   max    - 3 floats x n
	 *   radius - 1 float  x n
	*/

	const size_t value_count = ce_figfile_proto_value_count[proto->type];

	if (NULL == (proto->center = ce_alloc(3 * sizeof(float) * value_count)) ||
			NULL == (proto->min = ce_alloc(3 * sizeof(float) * value_count)) ||
			NULL == (proto->max = ce_alloc(3 * sizeof(float) * value_count)) ||
			NULL == (proto->radius = ce_alloc(sizeof(float) * value_count))) {
		ce_logging_error("figfile: could not allocate memory");
		return false;
	}

	if (value_count != ce_memfile_read(mem, proto->center,
									3 * sizeof(float), value_count) ||
			value_count != ce_memfile_read(mem, proto->min,
									3 * sizeof(float), value_count) ||
			value_count != ce_memfile_read(mem, proto->max,
									3 * sizeof(float), value_count) ||
			value_count != ce_memfile_read(mem, proto->radius,
									sizeof(float), value_count)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	return true;
}

static bool
ce_figfile_proto_read_header(ce_figfile_proto* proto, ce_memfile* mem)
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

	// not tested for FIGn
	if (CE_FIGFILE_SIGNATURE_FIG8 != ce_le2cpu32(signature)) {
		ce_logging_error("figfile: wrong signature");
		return false;
	}

	uint32_t header[9];
	if (9 != ce_memfile_read(mem, header, sizeof(uint32_t), 9)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	assert(0 == ce_le2cpu32(header[6]));

	proto->type = CE_FIGFILE_TYPE_FIG8;
	proto->vertex_count = ce_le2cpu32(header[0]);
	proto->normal_count = ce_le2cpu32(header[1]);
	proto->texcoord_count = ce_le2cpu32(header[2]);
	proto->index_count = ce_le2cpu32(header[3]);
	proto->component_count = ce_le2cpu32(header[4]);
	proto->light_component_count = ce_le2cpu32(header[5]);

	return true;
}

ce_figfile_proto* ce_figfile_proto_open_memfile(ce_memfile* mem)
{
	ce_figfile_proto* proto = ce_alloc_zero(sizeof(ce_figfile_proto));
	if (NULL == proto) {
		ce_logging_error("figfile: could not allocate memory");
		return NULL;
	}

	if (!ce_figfile_proto_read_header(proto, mem) ||
			!ce_figfile_proto_read_bound_data(proto, mem) ||
			!ce_figfile_proto_read_model_data(proto, mem)) {
		ce_figfile_proto_close(proto);
		return NULL;
	}

	return proto;
}

ce_figfile_proto* ce_figfile_proto_open_file(const char* path)
{
	ce_memfile* mem = ce_memfile_open_file(path, "rb");
	if (NULL == mem) {
		return NULL;
	}

	ce_figfile_proto* proto = ce_figfile_proto_open_memfile(mem);
	return ce_memfile_close(mem), proto;
}

void ce_figfile_proto_close(ce_figfile_proto* proto)
{
	if (NULL != proto) {
		const size_t value_count = ce_figfile_proto_value_count[proto->type];

		ce_free(proto->light_components, proto->light_component_size *
										proto->light_component_count);
		ce_free(proto->components, proto->component_size *
									proto->component_count);
		ce_free(proto->indices, proto->index_size * proto->index_count);
		ce_free(proto->texcoords, proto->texcoord_size * proto->texcoord_count);
		ce_free(proto->normals, proto->normal_size * proto->normal_count);
		ce_free(proto->vertices, proto->vertex_size * proto->vertex_count);
		ce_free(proto->radius, sizeof(float) * value_count);
		ce_free(proto->max, 3 * sizeof(float) * value_count);
		ce_free(proto->min, 3 * sizeof(float) * value_count);
		ce_free(proto->center, 3 * sizeof(float) * value_count);
		ce_free(proto, sizeof(ce_figfile_proto));
	}
}

static bool ce_figfile_open_impl(ce_figfile* fig,
								const ce_figfile_proto* proto,
								const ce_complection* cm)
{
	fig->vertex_count = 4 * proto->vertex_count;
	fig->normal_count = 4 * proto->normal_count;
	fig->texcoord_count = proto->texcoord_count;
	fig->index_count = proto->index_count;
	fig->component_count = proto->component_count;

	fig->vertices = ce_alloc(sizeof(ce_vec3) * fig->vertex_count);
	fig->normals = ce_alloc(sizeof(ce_vec3) * fig->normal_count);
	fig->texcoords = ce_alloc(sizeof(ce_vec2) * fig->texcoord_count);
	fig->indices = ce_alloc(sizeof(short) * fig->index_count);
	fig->components = ce_alloc(sizeof(ce_figfile_component) *
								fig->component_count);

	if (NULL == fig->vertices || NULL == fig->normals ||
			NULL == fig->texcoords || NULL == fig->indices ||
			NULL == fig->components) {
		ce_logging_error("figfile: could not allocate memory");
		return false;
	}

	const ce_figfile_proto_value_callback value_callback =
		ce_figfile_proto_value_callbacks[proto->type];

	const size_t value_count = ce_figfile_proto_value_count[proto->type];

	for (int i = 0, n = fig->vertex_count; i < n; ++i) {
		int j = 3 * value_count * 4 * (i / 4) + i % 4;
		int k = 3 * value_count * 4 / 3;
		ce_vec3_init(fig->vertices + i,
					value_callback(proto->vertices + j + 0 * k, 4, cm),
					value_callback(proto->vertices + j + 1 * k, 4, cm),
					value_callback(proto->vertices + j + 2 * k, 4, cm));
	}

	for (int i = 0, n = fig->normal_count; i < n; ++i) {
		int j = 4 * 4 * (i / 4) + i % 4;
		float w = proto->normals[j + 12];
		ce_vec3_init(fig->normals + i,
					 proto->normals[j + 0] / w,
					 proto->normals[j + 4] / w,
					 proto->normals[j + 8] / w);
	}

	for (int i = 0, n = fig->texcoord_count; i < n; ++i) {
		ce_vec2_init_array(fig->texcoords + i, proto->texcoords + 2 * i);
	}

	for (int i = 0, n = fig->index_count; i < n; ++i) {
		fig->indices[i] = proto->indices[i];
	}

	for (int i = 0, n = fig->component_count; i < n; ++i) {
		fig->components[i].vertex_index = proto->components[3 * i + 0];
		fig->components[i].normal_index = proto->components[3 * i + 1];
		fig->components[i].texcoord_index = proto->components[3 * i + 2];
	}

	ce_vec3_init(&fig->bounding_box.center,
				value_callback(proto->center + 0, 3, cm),
				value_callback(proto->center + 1, 3, cm),
				value_callback(proto->center + 2, 3, cm));

	ce_vec3_init(&fig->bounding_box.min,
				value_callback(proto->min + 0, 3, cm),
				value_callback(proto->min + 1, 3, cm),
				value_callback(proto->min + 2, 3, cm));

	ce_vec3_init(&fig->bounding_box.max,
				value_callback(proto->max + 0, 3, cm),
				value_callback(proto->max + 1, 3, cm),
				value_callback(proto->max + 2, 3, cm));

	fig->radius = value_callback(proto->radius, 1, cm);

	return true;
}

ce_figfile* ce_figfile_open(const ce_figfile_proto* proto,
									const ce_complection* cm)
{
	ce_figfile* fig = ce_alloc_zero(sizeof(ce_figfile));
	if (NULL == fig) {
		ce_logging_error("figfile: could not allocate memory");
		return NULL;
	}

	if (!ce_figfile_open_impl(fig, proto, cm)) {
		ce_figfile_close(fig);
		return NULL;
	}

	return fig;
}

void ce_figfile_close(ce_figfile* fig)
{
	if (NULL != fig) {
		ce_free(fig->components, sizeof(ce_figfile_component) *
									fig->component_count);
		ce_free(fig->indices, sizeof(short) * fig->index_count);
		ce_free(fig->texcoords, sizeof(ce_vec2) * fig->texcoord_count);
		ce_free(fig->normals, sizeof(ce_vec3) * fig->normal_count);
		ce_free(fig->vertices, sizeof(ce_vec3) * fig->vertex_count);
		ce_free(fig, sizeof(ce_figfile));
	}
}
