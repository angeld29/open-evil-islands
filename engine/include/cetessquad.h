/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
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

#ifndef QUAD_XY_H_INCLUDED
#define QUAD_XY_H_INCLUDED

enum {
	CE_TESS_QUAD_EDGE_X_LOWER = 0x1,
	CE_TESS_QUAD_EDGE_Z_LOWER = 0x2,
	CE_TESS_QUAD_EDGE_X_HIGHER = 0x4,
	CE_TESS_QUAD_EDGE_Z_HIGHER = 0x8,
};

ce_vec3* ce_tess_quad(ce_vec3* points, const int lod, const ce_vec3* P, const ce_vec3* Tu, const ce_vec3* Tw, const char edgeFlags);
ce_vec3* ce_tess_quad_normal(ce_vec3* normals, const int lod, const ce_vec3* P, const ce_vec3* Tu, const ce_vec3* Tw, const char edgeFlags);
ce_vec3* ce_tess_quad_tangent_vectors(ce_vec3* tangent_vectors, const ce_vec3* normal);
ce_vec3* ce_tess_quad_point(ce_vec3* point, const ce_vec3* P, const ce_vec3* Tu, const ce_vec3* Tw, const float* Fu, const float* Fw);
void ce_tess_quad_basis_func(float* F, const float x);

#endif
