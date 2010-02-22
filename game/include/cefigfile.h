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
#include "cecomplection.h"
#include "cememfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CE_FIGFILE_TYPE_FIG8
} ce_figfile_proto_type;

typedef struct {
	float x[8][4];
	float y[8][4];
	float z[8][4];
} ce_figfile_proto_vertex;

typedef struct {
	float x[4];
	float y[4];
	float z[4];
	float w[4];
} ce_figfile_proto_normal;

typedef struct {
	short vertex_index;
	short normal_index;
	short texcoord_index;
} ce_figfile_proto_component;

typedef struct {
	short unknown1;
	short unknown2;
} ce_figfile_proto_light_component;

typedef struct {
	ce_figfile_proto_type type;
	int vertex_count;
	int normal_count;
	int texcoord_count;
	int index_count;
	int component_count;
	int light_component_count;
	int unknown1;
	int unknown2;
	ce_vec3* center;
	ce_vec3* min;
	ce_vec3* max;
	float* radius;
	ce_figfile_proto_vertex* vertices;
	ce_figfile_proto_normal* normals;
	ce_vec2* texcoords;
	short* indices;
	ce_figfile_proto_component* components;
	ce_figfile_proto_light_component* light_components;
} ce_figfile_proto;

extern ce_figfile_proto* ce_figfile_proto_open_memfile(ce_memfile* mem);
extern ce_figfile_proto* ce_figfile_proto_open_file(const char* path);
extern void ce_figfile_proto_close(ce_figfile_proto* fig);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FIGFILE_H */
