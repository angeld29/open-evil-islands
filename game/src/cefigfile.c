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
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cebyteorder.h"
#include "celogging.h"
#include "cealloc.h"
#include "cereshlp.h"
#include "cefigfile.h"

static float ce_figfile_value_fig1(const float* params, int stride,
								const ce_complection* complection)
{
	ce_unused(stride), ce_unused(complection);
	return *params;
}

static float ce_figfile_value_fig6(const float* params, int stride,
									const ce_complection* complection)
{
	float temp = params[0 * stride] + (params[1 * stride] -
								params[0 * stride]) * complection->strength;
	float value = temp + (params[2 * stride] - temp) * complection->dexterity;
	temp = params[3 * stride] + (params[4 * stride] -
								params[3 * stride]) * complection->strength;
	temp += (params[5 * stride] - temp) * complection->dexterity;
	return value += (temp - value) * complection->height;
}

static float ce_figfile_value_fig8(const float* params, int stride,
								const ce_complection* complection)
{
	float temp1 = params[0 * stride] + (params[1 * stride] -
									params[0 * stride]) * complection->strength;
	float temp2 = params[2 * stride] + (params[3 * stride] -
									params[2 * stride]) * complection->strength;
	float value = temp1 + (temp2 - temp1) * complection->dexterity;
	temp1 = params[4 * stride] + (params[5 * stride] -
								params[4 * stride]) * complection->strength;
	temp2 = params[6 * stride] + (params[7 * stride] -
								params[6 * stride]) * complection->strength;
	temp1 += (temp2 - temp1) * complection->dexterity;
	return value += (temp1 - value) * complection->height;
}

typedef struct {
	unsigned int type;
	int count;
	ce_figfile_value_callback callback;
} ce_figfile_value_tuple;

static const ce_figfile_value_tuple ce_figfile_value_tuples[] = {
	{ 0x38474946, 8, ce_figfile_value_fig8 }
};

static const
ce_figfile_value_tuple* ce_figfile_value_tuple_choose(unsigned int type)
{
	for (int i = 0, n = sizeof(ce_figfile_value_tuples) /
						sizeof(ce_figfile_value_tuples[0]); i < n; ++i) {
		const ce_figfile_value_tuple* value_tuple = ce_figfile_value_tuples + i;
		if (value_tuple->type == type) {
			return value_tuple;
		}
	}
	return NULL;
}

static bool ce_figfile_read_model_data(ce_figfile* figfile, ce_memfile* mem)
{
	/**
	 *  FIG model data:
	 *   vertex_size - 4 vertices x 3 components x n variants x float size
	 *   normal_size - 4 normals x 4 components x float size
	 *   texcoord_size - 2 components x float size
	 *   index_size - short size
	 *   spec_component_size - 3 components x short size
	 *   morph_component_size - 2 components x short size
	 *   spec_component - vertex_index, normal_index, texcoord_index
	 *   morph_component - morph_index, vertex_index
	*/

	assert(4 == sizeof(float));
	assert(2 == sizeof(uint16_t));

	figfile->vertex_size = 3 * figfile->value_count * 4 * 4;
	figfile->normal_size = 4 * 4 * 4;
	figfile->texcoord_size = 2 * 4;
	figfile->index_size = 2;
	figfile->spec_component_size = 3 * 2;
	figfile->morph_component_size = 2 * 2;

	figfile->vertices = ce_alloc(figfile->vertex_size * figfile->vertex_count);
	figfile->normals = ce_alloc(figfile->normal_size * figfile->normal_count);
	figfile->texcoords = ce_alloc(figfile->texcoord_size * figfile->texcoord_count);
	figfile->indices = ce_alloc(figfile->index_size * figfile->index_count);
	figfile->spec_components = ce_alloc(figfile->spec_component_size *
										figfile->spec_component_count);
	figfile->morph_components = ce_alloc(figfile->morph_component_size *
										figfile->morph_component_count);

	if (NULL == figfile->vertices || NULL == figfile->normals ||
			NULL == figfile->texcoords || NULL == figfile->indices ||
			NULL == figfile->spec_components || NULL == figfile->morph_components) {
		ce_logging_error("figfile: could not allocate memory");
		return false;
	}

	if (figfile->vertex_count != ce_memfile_read(mem,
									figfile->vertices,
									figfile->vertex_size,
									figfile->vertex_count) ||
			figfile->normal_count != ce_memfile_read(mem,
									figfile->normals,
									figfile->normal_size,
									figfile->normal_count) ||
			figfile->texcoord_count != ce_memfile_read(mem,
									figfile->texcoords,
									figfile->texcoord_size,
									figfile->texcoord_count) ||
			figfile->index_count != ce_memfile_read(mem,
									figfile->indices,
									figfile->index_size,
									figfile->index_count) ||
			figfile->spec_component_count != ce_memfile_read(mem,
											figfile->spec_components,
											figfile->spec_component_size,
											figfile->spec_component_count) ||
			figfile->morph_component_count != ce_memfile_read(mem,
											figfile->morph_components,
											figfile->morph_component_size,
											figfile->morph_component_count)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	for (int i = 0, n = figfile->index_count; i < n; ++i) {
		ce_le2cpu16s(figfile->indices + i);
	}

	for (int i = 0, n = 3 * figfile->spec_component_count; i < n; ++i) {
		ce_le2cpu16s(figfile->spec_components + i);
	}

	for (int i = 0, n = 2 * figfile->morph_component_count; i < n; ++i) {
		ce_le2cpu16s(figfile->morph_components + i);
	}

	return true;
}

static bool ce_figfile_read_bound_data(ce_figfile* figfile, ce_memfile* mem)
{
	/**
	 *  FIG bound data:
	 *   center - 3 floats x n
	 *   min    - 3 floats x n
	 *   max    - 3 floats x n
	 *   radius - 1 float  x n
	*/

	if (NULL == (figfile->center =
				ce_alloc(sizeof(float) * figfile->value_count * 3)) ||
			NULL == (figfile->min =
				ce_alloc(sizeof(float) * figfile->value_count * 3)) ||
			NULL == (figfile->max =
				ce_alloc(sizeof(float) * figfile->value_count * 3)) ||
			NULL == (figfile->radius =
				ce_alloc(sizeof(float) * figfile->value_count))) {
		ce_logging_error("figfile: could not allocate memory");
		return false;
	}

	if (3 != ce_memfile_read(mem, figfile->center,
				sizeof(float) * figfile->value_count, 3) ||
			3 != ce_memfile_read(mem, figfile->min,
				sizeof(float) * figfile->value_count, 3) ||
			3 != ce_memfile_read(mem, figfile->max,
				sizeof(float) * figfile->value_count, 3) ||
			1 != ce_memfile_read(mem, figfile->radius,
				sizeof(float) * figfile->value_count, 1)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	return true;
}

static bool ce_figfile_read_header(ce_figfile* figfile, ce_memfile* mem)
{
	/**
	 *  FIG header:
	 *   signature
	 *   vertex count
	 *   normal count
	 *   texcoord count
	 *   index count
	 *   spec component count
	 *   morph component count
	 *   unknown - always 0; for alignment?..
	 *   unknown
	 *   texture number (primary, secondary)
	*/

	uint32_t signature;
	if (1 != ce_memfile_read(mem, &signature, sizeof(uint32_t), 1)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	const ce_figfile_value_tuple* value_tuple =
		ce_figfile_value_tuple_choose(ce_le2cpu32(signature));
	if (NULL == value_tuple) {
		ce_logging_error("figfile: wrong signature");
		return false;
	}

	uint32_t header[9];
	if (9 != ce_memfile_read(mem, header, sizeof(uint32_t), 9)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	assert(0 == ce_le2cpu32(header[6]));

	figfile->value_count = value_tuple->count;
	figfile->value_callback = value_tuple->callback;
	figfile->vertex_count = ce_le2cpu32(header[0]);
	figfile->normal_count = ce_le2cpu32(header[1]);
	figfile->texcoord_count = ce_le2cpu32(header[2]);
	figfile->index_count = ce_le2cpu32(header[3]);
	figfile->spec_component_count = ce_le2cpu32(header[4]);
	figfile->morph_component_count = ce_le2cpu32(header[5]);
	figfile->unknown = ce_le2cpu32(header[7]);
	figfile->texture_number = ce_le2cpu32(header[8]);

	return true;
}

ce_figfile* ce_figfile_open_memfile(ce_memfile* memfile)
{
	ce_figfile* figfile = ce_alloc_zero(sizeof(ce_figfile));
	if (NULL == figfile) {
		ce_logging_error("figfile: could not allocate memory");
		return NULL;
	}

	if (!ce_figfile_read_header(figfile, memfile) ||
			!ce_figfile_read_bound_data(figfile, memfile) ||
			!ce_figfile_read_model_data(figfile, memfile)) {
		ce_figfile_close(figfile);
		return NULL;
	}

	return figfile;
}

ce_figfile* ce_figfile_open_resfile(ce_resfile* resfile, const char* name)
{
	ce_memfile* memfile = ce_reshlp_extract_memfile_by_name(resfile, name);
	ce_figfile* figfile = ce_figfile_open_memfile(memfile);
	return ce_memfile_close(memfile), figfile;
}

ce_figfile* ce_figfile_open_file(const char* path)
{
	ce_memfile* memfile = ce_memfile_open_file(path);
	if (NULL == memfile) {
		return NULL;
	}
	ce_figfile* figfile = ce_figfile_open_memfile(memfile);
	return ce_memfile_close(memfile), figfile;
}

void ce_figfile_close(ce_figfile* figfile)
{
	if (NULL != figfile) {
		ce_free(figfile->morph_components, figfile->morph_component_size *
										figfile->morph_component_count);
		ce_free(figfile->spec_components, figfile->spec_component_size *
										figfile->spec_component_count);
		ce_free(figfile->indices, figfile->index_size * figfile->index_count);
		ce_free(figfile->texcoords, figfile->texcoord_size * figfile->texcoord_count);
		ce_free(figfile->normals, figfile->normal_size * figfile->normal_count);
		ce_free(figfile->vertices, figfile->vertex_size * figfile->vertex_count);
		ce_free(figfile->radius, sizeof(float) * figfile->value_count);
		ce_free(figfile->max, sizeof(float) * figfile->value_count * 3);
		ce_free(figfile->min, sizeof(float) * figfile->value_count * 3);
		ce_free(figfile->center, sizeof(float) * figfile->value_count * 3);
		ce_free(figfile, sizeof(ce_figfile));
	}
}
