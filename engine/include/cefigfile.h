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

#ifndef CE_FIGFILE_H
#define CE_FIGFILE_H

#include <stddef.h>
#include <stdint.h>

#include "cecomplection.h"
#include "ceresfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef float (*ce_figfile_value_callback)(const float* params, size_t stride,
											const ce_complection* complection);

typedef struct {
	size_t value_count;
	ce_figfile_value_callback value_callback;
	int vertex_count;
	int normal_count;
	int texcoord_count;
	int index_count;
	int vertex_component_count;
	int morph_component_count;
	int user_data_offset;
	int material_group;
	int texture_number;
	float* center;
	float* min;
	float* max;
	float* radius;
	float* vertices;
	float* normals;
	float* texcoords;
	uint16_t* indices;
	uint16_t* vertex_components;
	uint16_t* morph_components;
	size_t size;
	void* data;
} ce_figfile;

extern ce_figfile* ce_figfile_open(ce_res_file* res_file, const char* name);
extern void ce_figfile_close(ce_figfile* figfile);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FIGFILE_H */
