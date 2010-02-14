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

static void ce_figfile_init_vector(ce_vec3* vec, float* v)
{
	for (int i = 0; i < 8; ++i, ++vec, v += 3) {
		ce_vec3_init_vector(vec, v);
	}
}

static bool ce_figfile_read_data(ce_figfile* fig, ce_memfile* mem)
{
	/*while (0 != ce_memfile_tell(mem)) {
		if (fig->vertex_count * 3 * 4 * 8 * 4 + fig->normal_count * 4 * 4 * 4 + fig->texcoord_count * 2 * 4 + fig->index_count * 2 + fig->component_count * 3 * 2 + fig->light_component_count * 2 * 2 == file_size - ce_memfile_tell(mem)) printf("\n-0-\n\n");
		if (fig->normal_count * 4 * 4 * 4 + fig->texcoord_count * 2 * 4 + fig->index_count * 2 + fig->component_count * 3 * 2 + fig->light_component_count * 2 * 2 == file_size - ce_memfile_tell(mem)) printf("\n-1-\n\n");
		if (fig->texcoord_count * 2 * 4 + fig->index_count * 2 + fig->component_count * 3 * 2 + fig->light_component_count * 2 * 2 == file_size - ce_memfile_tell(mem)) printf("\n-2-\n\n");
		if (fig->index_count * 2 + fig->component_count * 3 * 2 + fig->light_component_count * 2 * 2 == file_size - ce_memfile_tell(mem)) printf("\n-3-\n\n");
		if (fig->component_count * 3 * 2 + fig->light_component_count * 2 * 2 == file_size - ce_memfile_tell(mem)) printf("\n-4-\n\n");
		if (fig->light_component_count * 2 * 2 == file_size - ce_memfile_tell(mem)) printf("\n-5-\n\n");
		if (fig->index_count * 2 + fig->component_count * 3 * 2 + fig->light_component_count * 2 * 2 >= file_size - ce_memfile_tell(mem)) {
			char ch;
			if (1 != ce_memfile_read(mem, &ch, sizeof(char), 1)) {
				break;
			}
			printf("%hhu ", ch);
		} else {
			float f;
			if (1 != ce_memfile_read(mem, &f, sizeof(float), 1)) {
				break;
			}
			printf("%f ", f);
		}
		if (0 == ce_memfile_tell(mem) % 16) printf("\n");
	}
	printf("\n");*/

	return true;
}

static bool ce_figfile_read_bound_info(ce_figfile* fig, ce_memfile* mem)
{
	/**
	 * FIG bound info:
	 * center 3 floats x 8
	 * min    3 floats x 8
	 * max    3 floats x 8
	 * radius 1 float  x 8
	*/

	if (NULL == (fig->center = ce_alloc(sizeof(ce_vec3) * 8)) ||
			NULL == (fig->min = ce_alloc(sizeof(ce_vec3) * 8)) ||
			NULL == (fig->max = ce_alloc(sizeof(ce_vec3) * 8)) ||
			NULL == (fig->radius = ce_alloc(sizeof(float) * 8))) {
		ce_logging_error("figfile: could not allocate memory");
		return false;
	}

	float bound_info[80];
	if (80 != ce_memfile_read(mem, bound_info, sizeof(float), 80)) {
		ce_logging_error("figfile: io error occured");
		return false;
	}

	ce_figfile_init_vector(fig->center, bound_info + 0);
	ce_figfile_init_vector(fig->min, bound_info + 24);
	ce_figfile_init_vector(fig->max, bound_info + 48);
	memcpy(fig->radius, bound_info + 72, sizeof(float) * 8);

	return true;
}

static bool ce_figfile_read_header(ce_figfile* fig, ce_memfile* mem)
{
	/**
	 * FIG header:
	 *  signature
	 *  vertex count
	 *  normal count
	 *  texcoord count
	 *  index count
	 *  component count
	 *  light component count
	 *  unknown - always 0; for alignment?..
	 *  unknown1
	 *  unknown2
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
		ce_figfile_read_bound_info(fig, mem) &&
		ce_figfile_read_data(fig, mem);
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
		ce_free(fig->radius, sizeof(float) * 8);
		ce_free(fig->max, sizeof(ce_vec3) * 8);
		ce_free(fig->min, sizeof(ce_vec3) * 8);
		ce_free(fig->center, sizeof(ce_vec3) * 8);
		ce_free(fig, sizeof(ce_figfile));
	}
}
