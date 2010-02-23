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

#include "cevec2.h"
#include "cevec3.h"
#include "ceaabb.h"
#include "cecomplection.h"
#include "cememfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct ce_figfile_proto ce_figfile_proto;

typedef struct {
	short vertex_index;
	short normal_index;
	short texcoord_index;
} ce_figfile_component;

typedef struct {
	int vertex_count;
	int normal_count;
	int texcoord_count;
	int index_count;
	int component_count;
	ce_vec3* vertices;
	ce_vec3* normals;
	ce_vec2* texcoords;
	short* indices;
	ce_figfile_component* components;
	ce_aabb bounding_box;
	float radius;
} ce_figfile;

extern ce_figfile_proto* ce_figfile_proto_open_memfile(ce_memfile* mem);
extern ce_figfile_proto* ce_figfile_proto_open_file(const char* path);
extern void ce_figfile_proto_close(ce_figfile_proto* proto);

extern ce_figfile* ce_figfile_open(const ce_figfile_proto* proto,
									const ce_complection* cm);
extern void ce_figfile_close(ce_figfile* fig);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FIGFILE_H */
