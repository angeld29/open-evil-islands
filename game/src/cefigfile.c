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

/*
 *  See doc/formats/figfile.txt for more details.
*/

#include <assert.h>

#include "celib.h"
#include "cebyteorder.h"
#include "cealloc.h"
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
	{ 0, 1, ce_figfile_value_fig1 },
	{ 0, 6, ce_figfile_value_fig6 },
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

ce_figfile* ce_figfile_open(ce_resfile* resfile, const char* name)
{
	ce_figfile* figfile = ce_alloc(sizeof(ce_figfile));
	int index = ce_resfile_node_index(resfile, name);
	figfile->size = ce_resfile_node_size(resfile, index);
	figfile->data = ce_resfile_node_data(resfile, index);

	union {
		uint16_t* u16ptr;
		uint32_t* u32ptr;
		float* fptr;
	} data = { figfile->data };

	const ce_figfile_value_tuple* value_tuple =
		ce_figfile_value_tuple_choose(ce_le2cpu32(*data.u32ptr++));
	assert(NULL != value_tuple && "wrong signature");

	figfile->value_count = value_tuple->count;
	figfile->value_callback = value_tuple->callback;
	figfile->vertex_count = ce_le2cpu32(*data.u32ptr++);
	figfile->normal_count = ce_le2cpu32(*data.u32ptr++);
	figfile->texcoord_count = ce_le2cpu32(*data.u32ptr++);
	figfile->index_count = ce_le2cpu32(*data.u32ptr++);
	figfile->vertex_component_count = ce_le2cpu32(*data.u32ptr++);
	figfile->morph_component_count = ce_le2cpu32(*data.u32ptr++);

	uint32_t unknown = ce_le2cpu32(*data.u32ptr++);
	assert(0 == unknown); ce_unused(unknown);

	figfile->group = ce_le2cpu32(*data.u32ptr++);
	figfile->texture_number = ce_le2cpu32(*data.u32ptr++);

	figfile->center = data.fptr;
	figfile->min = data.fptr += 3 * figfile->value_count;
	figfile->max = data.fptr += 3 * figfile->value_count;
	figfile->radius = data.fptr += 3 * figfile->value_count;

	figfile->vertices = data.fptr += figfile->value_count;
	figfile->normals = data.fptr += 3 * figfile->value_count *
									4 * figfile->vertex_count;
	figfile->texcoords = data.fptr += 4 * 4 * figfile->normal_count;
	figfile->indices = (data.fptr += 2 * figfile->texcoord_count, data.u16ptr);
	figfile->vertex_components = data.u16ptr += figfile->index_count;
	figfile->morph_components = data.u16ptr += 3 * figfile->vertex_component_count;

	for (int i = 0; i < figfile->index_count; ++i) {
		ce_le2cpu16s(figfile->indices + i);
	}

	for (int i = 0, n = 3 * figfile->vertex_component_count; i < n; ++i) {
		ce_le2cpu16s(figfile->vertex_components + i);
	}

	for (int i = 0, n = 2 * figfile->morph_component_count; i < n; ++i) {
		ce_le2cpu16s(figfile->morph_components + i);
	}

	return figfile;
}

void ce_figfile_close(ce_figfile* figfile)
{
	if (NULL != figfile) {
		ce_free(figfile->data, figfile->size);
		ce_free(figfile, sizeof(ce_figfile));
	}
}
