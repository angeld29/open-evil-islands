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

#include "cetesstri.h"
#include "cevec3.h"

/// Calculation of Bezier surface patch by 3 points and 3 normal vectors for them
ce_vec3* ce_tess_tri_points(ce_vec3* points, const int lod, const ce_vec3* P, const ce_vec3* N, const char edgeFlags)
{
	/// Generated points in array ret[i][j]
	/// v
	/// 0,3
	/// 0,2   1,2
	/// 0,1   1,1   2,1
	/// 0,0   1,0   2,0   3,0	u
	/// etc
	/// 0,0-P[0] 3,0-P[1] 3,3-P[2]

	int edgeu = ((edgeFlags & CE_TESS_TRI_EDGE_U) != 0);
	int edgev = ((edgeFlags & CE_TESS_TRI_EDGE_V) != 0);
	int edgew = ((edgeFlags & CE_TESS_TRI_EDGE_W) != 0);

	ce_vec3 CP[10]; /// 9 control points for Bezier surface
	ce_tess_tri_control_points(CP, P, N);

	int vertex_count_u=lod+edgev+(edgeu&&edgew);/// (edgeu&&edgew) means that top u point must be worked
	int vertex_count_v=lod+edgeu+(edgev&&edgew);/// (edgev&&edgew) means that top v point must be worked

	for (int i=0; i < vertex_count_u; i++)
	{
		float u = (float)(i+(1-edgev)) / (float)(lod+1);

		for (int j=0; j < vertex_count_v - i; j++)
		{
			float v = (float)(j+(1-edgeu)) / (float)(lod+1);

			ce_tess_tri_point(&points[i*vertex_count_v +j], CP, u, v);
		}
	}

	return points;
}

/// Calculation array of control points for cubic Bezier triangular patch. P - 3 input point array N - normals for these points CP - output array
ce_vec3* ce_tess_tri_control_points(ce_vec3* CP, const ce_vec3* P, const ce_vec3* N)
{
	/// Control points in array
	///       0
	///     3   8
	///   4   9   7
	/// 1   5   6   2
	/// 0,1,2		- input points
	/// 3,4,5,6,7,8	- tangent vectors to 0,3,6

	CP[0] = P[0];
	CP[1] = P[1];
	CP[2] = P[2];

	ce_tess_tri_tangent_vectors(&CP[3], &P[0], &P[1], &N[0]);
	ce_tess_tri_tangent_vectors(&CP[4], &P[1], &P[0], &N[1]);
	ce_tess_tri_tangent_vectors(&CP[5], &P[1], &P[2], &N[1]);
	ce_tess_tri_tangent_vectors(&CP[6], &P[2], &P[1], &N[2]);
	ce_tess_tri_tangent_vectors(&CP[7], &P[2], &P[0], &N[2]);
	ce_tess_tri_tangent_vectors(&CP[8], &P[0], &P[2], &N[0]);

	ce_vec3 E, V, tmp;
	ce_vec3_mid_many(&E, &CP[3], 6);
	ce_vec3_mid_many(&V, P, 3);
	ce_vec3_add(&CP[9],&E, ce_vec3_scale(&tmp, 0.5, ce_vec3_sub(&tmp, &E, &V))); /// E+(E-V)/2

	return CP;
}

/// calc tangent vectors - projection of 1/2 of edge on plane defined by normal
ce_vec3* ce_tess_tri_tangent_vectors2(ce_vec3* tangent, const ce_vec3* P1, const ce_vec3* P2, const ce_vec3* N1)
{
	ce_vec3 tmp;
	return ce_vec3_scale(tangent, 0.5f, ce_vec3_sub(tangent,			/// (P1+P2  -  ((P2-P1)*(scalar)*N1)*N1)  /2
		ce_vec3_add(tangent, P1, P2), 								/// P1+P2
		ce_vec3_scale(&tmp, ce_vec3_dot(ce_vec3_sub(&tmp, P2, P1),N1), N1)));	/// ((P2-P1)*(scalar)*N1)*N1
}

/// calc tangent vectors - projection of 1/3 of edge on plane defined by normal
ce_vec3* ce_tess_tri_tangent_vectors(ce_vec3* tangent, const ce_vec3* P1, const ce_vec3* P2, const ce_vec3* N1)
{
	ce_vec3 tmp;
	return ce_vec3_scale(tangent, 1.0f/3.0f, ce_vec3_sub(tangent,			/// (2*P1+P2  -  ((P2-P1)*(scalar)*N1)*N1)  /3
		ce_vec3_add(tangent,ce_vec3_scale(tangent, 2,  P1), P2), 	/// 2*P1+P2
		ce_vec3_scale(&tmp, ce_vec3_dot(ce_vec3_sub(&tmp, P2, P1),N1), N1)));	/// ((P2-P1)*(scalar)*N1)*N1
}

/// Calculation of point on cubic Bezier patch by 3 input control points, 2x3 tangent vectors control points, and one central control point (formed to CP array)
ce_vec3* ce_tess_tri_point(ce_vec3* point, const ce_vec3* CP, const float u, const float v)
{
	/// Control points in array
	///       0
	///     3   8
	///   4   9   7
	/// 1   5   6   2
	/// 0,1,2		- input points
	/// 3,4,5,6,7,8	- tangent vectors to 0,3,6

	float w = 1 - u - v;

	float u2 = u*u;
	float u3 = u2*u;
	float v2 = v*v;
	float v3 = v2*v;
	float w2 = w*w;
	float w3 = w2*w;

	point->x =
		CP[0].x*w3 + CP[1].x*u3 + CP[2].x*v3 +
		3*( CP[3].x*w2*u + CP[4].x*w*u2 + CP[8].x*w2*v +
			CP[5].x*u2*v + CP[7].x*w*v2 + CP[6].x*u*v2 ) +
		6*CP[9].x*w*u*v;

	point->y =
		CP[0].y*w3 + CP[1].y*u3 + CP[2].y*v3 +
		3*( CP[3].y*w2*u + CP[4].y*w*u2 + CP[8].y*w2*v +
			CP[5].y*u2*v + CP[7].y*w*v2 + CP[6].y*u*v2 ) +
		6*CP[9].y*w*u*v;

	point->z =
		CP[0].z*w3 + CP[1].z*u3 + CP[2].z*v3 +
		3*( CP[3].z*w2*u + CP[4].z*w*u2 + CP[8].z*w2*v +
			CP[5].z*u2*v + CP[7].z*w*v2 + CP[6].z*u*v2 ) +
		6*CP[9].z*w*u*v;

	return point;
}

/// Calculation of quadratic Bezier approximation of normals by 3 points and 3 normal vectors for them
ce_vec3* ce_tess_tri_normals(ce_vec3* normals, const int lod, const ce_vec3* P, const ce_vec3* N, const char edgeFlags)
{
	/// Generated points in array ret[i][j]
	/// v
	/// 0,3
	/// 0,2   1,2
	/// 0,1   1,1   2,1
	/// 0,0   1,0   2,0   3,0	u
	/// etc
	/// 0,0-P[0] 3,0-P[1] 3,3-P[2]

	int edgeu = ((edgeFlags & CE_TESS_TRI_EDGE_U) != 0);
	int edgev = ((edgeFlags & CE_TESS_TRI_EDGE_V) != 0);
	int edgew = ((edgeFlags & CE_TESS_TRI_EDGE_W) != 0);

	ce_vec3 CP[10]; /// 9 control points for Bezier surface
	ce_tess_tri_control_normals(CP, P, N);

	int vertex_count_u=lod+edgev+(edgeu&&edgew);/// (edgeu&&edgew) means that top u point must be worked
	int vertex_count_v=lod+edgeu+(edgev&&edgew);/// (edgev&&edgew) means that top v point must be worked

	for (int i=0; i < vertex_count_u; i++)
	{
		float u = (float)(i+(1-edgev)) / (float)(lod+1);

		for (int j=0; j < vertex_count_v - i; j++)
		{
			float v = (float)(j+(1-edgeu)) / (float)(lod+1);

			ce_tess_tri_point(&normals[i*vertex_count_v +j], CP, u, v);
		}
	}

	return normals;
}

/// Calculation array of control points for quadratic Bezier triangular approximation. P - 3 input point array N - normals for these points CP - output array
ce_vec3* ce_tess_tri_control_normals(ce_vec3* CP, const ce_vec3* P, const ce_vec3* N)
{
	/// Control points in array
	///       0
	///     3   5
	///   1   4   2
	/// 0,1,2	- input normals
	/// 3,4,5	- mid-edge coefficients

	CP[0] = N[0];
	CP[1] = N[1];
	CP[2] = N[2];
	ce_tess_tri_midedge(&CP[3], &P[0], &P[1], &N[0], &N[1]);
	ce_tess_tri_midedge(&CP[4], &P[1], &P[2], &N[1], &N[2]);
	ce_tess_tri_midedge(&CP[5], &P[2], &P[0], &N[2], &N[0]);

	return CP;
}

/// Calculation of quadratic mid-edge coefficients(mid-edge coefficient reflection from plane with (P2-P1) normal direction)
ce_vec3* ce_tess_tri_midedge(ce_vec3* midedge, const ce_vec3* P1, const ce_vec3* P2, const ce_vec3* N1, const ce_vec3* N2)
{
	ce_vec3 N12, P21 ,tmp;
	ce_vec3_add(&N12, N1, N2);
	ce_vec3_sub(&P21, P2, P1);

	return ce_vec3_norm(midedge,
			ce_vec3_sub(midedge, &N12, ce_vec3_scale(midedge, 			/// N1+N2 - (P2-P1)*v12
				2*ce_vec3_dot(&P21, &N12)/ce_vec3_dot(&P21,&P21),  /// v12 = 2*((P2-P1)*scalar*(N1+N2))/((P2-P1)*scalar*(P2-P1))
				&P21)));
}

/// Calculation of normal by quadratic Bezier triangular approximation by 3 input control points and 3 mid-edge coefficients (formed to CP array)
ce_vec3* ce_tess_tri_normal(ce_vec3* normal, const ce_vec3* CP, const float u, const float v)
{
	/// Control points in array
	///       0
	///     3   5
	///   1   4   2
	/// 0,1,2	- input normals
	/// 3,4,5	- mid-edge coefficients

	float w = 1 - u - v;

	float u2 = u*u;
	float v2 = v*v;
	float w2 = w*w;

	normal->x =
		CP[0].x*w2 + CP[1].x*u2 + CP[2].x*v2 +
		CP[3].x*w*u + CP[4].x*u*v + CP[5].x*w*v;

	normal->y =
		CP[0].y*w2 + CP[1].y*u2 + CP[2].y*v2 +
		CP[3].y*w*u + CP[4].y*u*v + CP[5].y*w*v;

	normal->z =
		CP[0].z*w2 + CP[1].z*u2 + CP[2].z*v2 +
		CP[3].z*w*u + CP[4].z*u*v + CP[5].z*w*v;

	return ce_vec3_norm(normal,normal);
}
