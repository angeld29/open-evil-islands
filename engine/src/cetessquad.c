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

#include "cetessquad.h"
#include "cevec3.h"


/// Calculation of Ferguson surface patch by 4 points and 2x4 tangent vectors for them. edgeFlags is flags for calculation points on quad`s edges
ce_vec3* ce_tess_quad(ce_vec3* points, const int lod, const ce_vec3* P, const ce_vec3* TV, const char edgeFlags)
{
	/// points and tangent vectors in P and Tu/Tw
	/// z
	/// 2	3
	/// 0	1	x

	int edgelx = ((edgeFlags & CE_TESS_QUAD_EDGE_X_LOWER)  != 0);
	int edgelz = ((edgeFlags & CE_TESS_QUAD_EDGE_Z_LOWER)  != 0);
	int edgehx = ((edgeFlags & CE_TESS_QUAD_EDGE_X_HIGHER) != 0);
	int edgehz = ((edgeFlags & CE_TESS_QUAD_EDGE_Z_HIGHER) != 0);

	float Fu[4];
	float Fw[4];

	int vertex_count_x=lod+edgelx+edgehx;
	int vertex_count_z=lod+edgelz+edgehz;

	for (int x=0; x < vertex_count_x; x++)
	{
		float u = (float)(x+(1-edgelx)) / (float)(lod+1);
		ce_tess_quad_basis_func(Fu, u);

		for (int z=0; z < vertex_count_z; z++)
		{
			float w = (float)(z+(1-edgelz)) / (float)(lod+1);
			ce_tess_quad_basis_func(Fw, w);

			ce_tess_quad_point(&points[x*vertex_count_z+z],P,TV,Fu,Fw);
		}
	}
	return points;
}

/// Calculation of Ferguson surface patch by 4 points and 2x4 tangent vectors for them
ce_vec3* ce_tess_quad_normal(ce_vec3* normals, const int lod, const ce_vec3* P, const ce_vec3* TV, const char edgeFlags)
{
	/// points and tangent vectors in P and Tu/Tw
	/// z
	/// 2	3
	/// 0	1	x

	int edgelx = ((edgeFlags & CE_TESS_QUAD_EDGE_X_LOWER)  != 0);
	int edgelz = ((edgeFlags & CE_TESS_QUAD_EDGE_Z_LOWER)  != 0);
	int edgehx = ((edgeFlags & CE_TESS_QUAD_EDGE_X_HIGHER) != 0);
	int edgehz = ((edgeFlags & CE_TESS_QUAD_EDGE_Z_HIGHER) != 0);

	float Fu[4];
	float Fw[4];

	float delta = 0.01/(lod+1);
	float Fudelta[4];
	float Fwdelta[4];

	ce_vec3 a,b, base;

	int vertex_count_x=lod+edgelx+edgehx;
	int vertex_count_z=lod+edgelz+edgehz;

	for (int x=0; x < vertex_count_x; x++)
	{
		float u = (float)(x+(1-edgelx)) / (float)(lod+1);
		ce_tess_quad_basis_func(Fu, u);
		ce_tess_quad_basis_func(Fudelta, u+delta);

		for (int z=0; z < vertex_count_z; z++)
		{
			float w = (float)(z+(1-edgelz)) / (float)(lod+1);
			ce_tess_quad_basis_func(Fw, w);
			ce_tess_quad_basis_func(Fwdelta, w+delta);

			ce_tess_quad_point(&base,P,TV,Fu,Fw);
			ce_tess_quad_point(&a,P,TV,Fudelta,Fw);
			ce_tess_quad_point(&b,P,TV,Fu,Fwdelta);
			ce_vec3_sub(&a,&a,&base);
			ce_vec3_sub(&b,&b,&base);
			ce_vec3_norm(&normals[x*vertex_count_z+z],ce_vec3_cross(&base,&a,&b));

		}
	}
	return normals;
}

/// Calculation of tangent vectors through vector projection`s perpendiculars. tangent_vectors - pointer to 2-element array
ce_vec3* ce_tess_quad_tangent_vectors(ce_vec3* tangent_vectors, const ce_vec3* normal)
{
	tangent_vectors[0].x = normal->y;
	tangent_vectors[0].y = -normal->x;
	tangent_vectors[0].z = 0;

	tangent_vectors[1].x = 0;
	tangent_vectors[1].y = -normal->z;
	tangent_vectors[1].z = normal->y;

	return tangent_vectors;
}

/// Calculation of point on Ferguson patch by 4 points, 2x4 tangent vectors, 4 twist vectors and 2 basis functions arrays for this point
ce_vec3* ce_tess_quad_point(ce_vec3* point, const ce_vec3* P, const ce_vec3* TV, const float* Fu, const float* Fw)
{
	point->x=
	Fw[0]*(Fu[0]*P[0].x  + Fu[1]*P[1].x  + Fu[2]*TV[2*0+0].x + Fu[3]*TV[2*1+0].x) +
	Fw[1]*(Fu[0]*P[2].x  + Fu[1]*P[3].x  + Fu[2]*TV[2*2+0].x + Fu[3]*TV[2*3+0].x) +
	Fw[2]*(Fu[0]*TV[2*0+1].x + Fu[1]*TV[2*1+1].x)+
	Fw[3]*(Fu[0]*TV[2*2+1].x + Fu[1]*TV[2*3+1].x);

	point->y=
	Fw[0]*(Fu[0]*P[0].y  + Fu[1]*P[1].y  + Fu[2]*TV[2*0+0].y + Fu[3]*TV[2*1+0].y) +
	Fw[1]*(Fu[0]*P[2].y  + Fu[1]*P[3].y  + Fu[2]*TV[2*2+0].y + Fu[3]*TV[2*3+0].y) +
	Fw[2]*(Fu[0]*TV[2*0+1].y + Fu[1]*TV[2*1+1].y)+
	Fw[3]*(Fu[0]*TV[2*2+1].y + Fu[1]*TV[2*3+1].y);

	point->z=
	Fw[0]*(Fu[0]*P[0].z  + Fu[1]*P[1].z  + Fu[2]*TV[2*0+0].z + Fu[3]*TV[2*1+0].z) +
	Fw[1]*(Fu[0]*P[2].z  + Fu[1]*P[3].z  + Fu[2]*TV[2*2+0].z + Fu[3]*TV[2*3+0].z) +
	Fw[2]*(Fu[0]*TV[2*0+1].z + Fu[1]*TV[2*1+1].z)+
	Fw[3]*(Fu[0]*TV[2*2+1].z + Fu[1]*TV[2*3+1].z);

	return point;
}

/// Hermite basis functions calculation
void ce_tess_quad_basis_func(float* F, const float x)
{
	float x2 = x*x;
	float x3 = x2*x;

	F[0] =  2*x3 - 3*x2 + 1;		/// basis function 1
	F[1] = -2*x3 + 3*x2;			/// basis function 2
	F[2] =    x3 - 2*x2 + x;		/// basis function 3
	F[3] =    x3 -   x2;			/// basis function 4
}
