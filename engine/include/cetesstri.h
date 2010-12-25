/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Anton Kurkin
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

#include "cevec3.h"

#ifndef CE_TESS_TRI_H_INCLUDED
#define CE_TESS_TRI_H_INCLUDED

enum {
	CE_TESS_TRI_EDGE_U = 0x1,	/// edge with v == 0
	CE_TESS_TRI_EDGE_V = 0x2,	/// edge with u == 0
	CE_TESS_TRI_EDGE_W = 0x4,	/// edge with u+v == 1
};

ce_vec3* ce_tess_tri_points(ce_vec3* points, const int lod, const ce_vec3* P, const ce_vec3* N, const char edgeFlags);
ce_vec3* ce_tess_tri_control_points(ce_vec3* CP, const ce_vec3* P, const ce_vec3* N);
ce_vec3* ce_tess_tri_tangent_vectors(ce_vec3* tangent, const ce_vec3* P1, const ce_vec3* P2, const ce_vec3* N1);
ce_vec3* ce_tess_tri_tangent_vectors3(ce_vec3* tangent, const ce_vec3* P1, const ce_vec3* P2, const ce_vec3* N1);
ce_vec3* ce_tess_tri_point(ce_vec3* point, const ce_vec3* CP, const float u, const float v);

ce_vec3* ce_tess_tri_normals(ce_vec3* normals, const int lod, const ce_vec3* P, const ce_vec3* N, const char edgeFlags);
ce_vec3* ce_tess_tri_control_normals(ce_vec3* CP, const ce_vec3* P, const ce_vec3* N);
ce_vec3* ce_tess_tri_midedge(ce_vec3* midedge, const ce_vec3* P1, const ce_vec3* P2, const ce_vec3* N1, const ce_vec3* N2);
ce_vec3* ce_tess_tri_normal(ce_vec3* normal, const ce_vec3* CP, const float u, const float v);

#endif
