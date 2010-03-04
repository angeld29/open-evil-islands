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
 *  Based on http://www.lighthouse3d.com/opengl/viewfrustum/.
*/

#include <stddef.h>
#include <math.h>

#include "cemath.h"
#include "cefrustum.h"

static ce_vec3* get_box_vertex_positive(ce_vec3* point, const ce_vec3* normal,
														const ce_aabb* box)
{
	point->x = normal->x > 0.0f ? box->max.x : box->min.x;
	point->y = normal->y > 0.0f ? box->max.y : box->min.y;
	point->z = normal->z > 0.0f ? box->max.z : box->min.z;
	return point;
}

ce_frustum* ce_frustum_init(ce_frustum* frustum,
							float fov, float aspect,
							float near, float far,
							const ce_vec3* eye, const ce_vec3* forward,
							const ce_vec3* right, const ce_vec3* up)
{
	float tang = tanf(0.5f * ce_deg2rad(fov));
	float nh = tang * near;
	float nw = nh * aspect;
	float fh = tang * far;
	float fw = fh * aspect;

	ce_vec3 nc, fc, xw, yh;
	ce_vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;

	ce_vec3_add(&nc, eye, ce_vec3_scale(&nc, forward, near));
	ce_vec3_add(&fc, eye, ce_vec3_scale(&fc, forward, far));

	ce_vec3_scale(&xw, right, nw);
	ce_vec3_scale(&yh, up, nh);

	ce_vec3_sub(&ntl, ce_vec3_add(&ntl, &nc, &yh), &xw);
	ce_vec3_add(&ntr, ce_vec3_add(&ntr, &nc, &yh), &xw);
	ce_vec3_sub(&nbl, ce_vec3_sub(&nbl, &nc, &yh), &xw);
	ce_vec3_add(&nbr, ce_vec3_sub(&nbr, &nc, &yh), &xw);

	ce_vec3_scale(&xw, right, fw);
	ce_vec3_scale(&yh, up, fh);

	ce_vec3_sub(&ftl, ce_vec3_add(&ftl, &fc, &yh), &xw);
	ce_vec3_add(&ftr, ce_vec3_add(&ftr, &fc, &yh), &xw);
	ce_vec3_sub(&fbl, ce_vec3_sub(&fbl, &fc, &yh), &xw);
	ce_vec3_add(&fbr, ce_vec3_sub(&fbr, &fc, &yh), &xw);

	ce_plane_init_tri(&frustum->planes[CE_FRUSTUM_PLANE_TOP], &ntr, &ntl, &ftl);
	ce_plane_init_tri(&frustum->planes[CE_FRUSTUM_PLANE_BOTTOM], &nbl, &nbr, &fbr);
	ce_plane_init_tri(&frustum->planes[CE_FRUSTUM_PLANE_LEFT], &ntl, &nbl, &fbl);
	ce_plane_init_tri(&frustum->planes[CE_FRUSTUM_PLANE_RIGHT], &nbr, &ntr, &fbr);
	ce_plane_init_tri(&frustum->planes[CE_FRUSTUM_PLANE_NEAR], &ntl, &ntr, &nbr);
	ce_plane_init_tri(&frustum->planes[CE_FRUSTUM_PLANE_FAR], &ftr, &ftl, &fbl);

	return frustum;
}

bool ce_frustum_test_point(const ce_frustum* frustum, const ce_vec3* point)
{
	for (int i = 0; i < CE_FRUSTUM_PLANE_COUNT; ++i) {
		if (ce_plane_dist(&frustum->planes[i], point) < 0.0f) {
			return false;
		}
	}
	return true;
}

bool ce_frustum_test_box(const ce_frustum* frustum, const ce_aabb* box)
{
	ce_vec3 point;
	for (int i = 0; i < CE_FRUSTUM_PLANE_COUNT; ++i) {
		if (ce_plane_dist(&frustum->planes[i], get_box_vertex_positive(
				&point, &frustum->planes[i].n, box)) < 0.0f) {
			return false;
		}
	}
	return true;
}

bool ce_frustum_test_sphere(const ce_frustum* frustum, const ce_sphere* sphere)
{
	for (int i = 0; i < CE_FRUSTUM_PLANE_COUNT; ++i) {
		if (ce_plane_dist(&frustum->planes[i], &sphere->center) < -sphere->radius) {
			return false;
		}
	}
	return true;
}
