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
#include "frustum.h"

static vec3* get_box_vertex_positive(const vec3* n, const ce_aabb* b, vec3* p)
{
	p->x = n->x > 0.0f ? b->max.x : b->min.x;
	p->y = n->y > 0.0f ? b->max.y : b->min.y;
	p->z = n->z > 0.0f ? b->max.z : b->min.z;
	return p;
}

frustum* frustum_init(float fov, float aspect, float near, float far,
						const vec3* eye, const vec3* forward,
						const vec3* right, const vec3* up, frustum* f)
{
	float tang = tanf(0.5f * ce_deg2rad(fov));
	float nh = tang * near;
	float nw = nh * aspect;
	float fh = tang * far;
	float fw = fh * aspect;

	vec3 nc, fc, xw, yh;
	vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;

	vec3_add(eye, vec3_scale(forward, near, &nc), &nc);
	vec3_add(eye, vec3_scale(forward, far, &fc), &fc);

	vec3_scale(right, nw, &xw);
	vec3_scale(up, nh, &yh);

	vec3_sub(vec3_add(&nc, &yh, &ntl), &xw, &ntl);
	vec3_add(vec3_add(&nc, &yh, &ntr), &xw, &ntr);
	vec3_sub(vec3_sub(&nc, &yh, &nbl), &xw, &nbl);
	vec3_add(vec3_sub(&nc, &yh, &nbr), &xw, &nbr);

	vec3_scale(right, fw, &xw);
	vec3_scale(up, fh, &yh);

	vec3_sub(vec3_add(&fc, &yh, &ftl), &xw, &ftl);
	vec3_add(vec3_add(&fc, &yh, &ftr), &xw, &ftr);
	vec3_sub(vec3_sub(&fc, &yh, &fbl), &xw, &fbl);
	vec3_add(vec3_sub(&fc, &yh, &fbr), &xw, &fbr);

	plane_init_tri(&ntr, &ntl, &ftl, &f->p[FRUSTUM_PLANE_TOP]);
	plane_init_tri(&nbl, &nbr, &fbr, &f->p[FRUSTUM_PLANE_BOTTOM]);
	plane_init_tri(&ntl, &nbl, &fbl, &f->p[FRUSTUM_PLANE_LEFT]);
	plane_init_tri(&nbr, &ntr, &fbr, &f->p[FRUSTUM_PLANE_RIGHT]);
	plane_init_tri(&ntl, &ntr, &nbr, &f->p[FRUSTUM_PLANE_NEAR]);
	plane_init_tri(&ftr, &ftl, &fbl, &f->p[FRUSTUM_PLANE_FAR]);

	return f;
}

bool frustum_test_point(const vec3* p, const frustum* f)
{
	for (size_t i = 0; i < FRUSTUM_PLANE_COUNT; ++i) {
		if (plane_dist(&f->p[i], p) < 0.0f) {
			return false;
		}
	}
	return true;
}

bool frustum_test_sphere(const vec3* p, float r, const frustum* f)
{
	for (size_t i = 0; i < FRUSTUM_PLANE_COUNT; ++i) {
		if (plane_dist(&f->p[i], p) < -r) {
			return false;
		}
	}
	return true;
}

bool frustum_test_box(const ce_aabb* b, const frustum* f)
{
	vec3 p;
	for (size_t i = 0; i < FRUSTUM_PLANE_COUNT; ++i) {
		if (plane_dist(&f->p[i],
				get_box_vertex_positive(&f->p[i].n, b, &p)) < 0.0f) {
			return false;
		}
	}
	return true;
}
