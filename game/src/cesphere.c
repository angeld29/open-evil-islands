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

#include <math.h>

#include "cesphere.h"

ce_sphere*
ce_sphere_init(ce_sphere* sphere, const ce_vec3* center, float radius)
{
	ce_vec3_copy(&sphere->center, center);
	sphere->radius = radius;
	return sphere;
}

ce_sphere* ce_sphere_init_aabb(ce_sphere* sphere, const ce_aabb* aabb)
{
	ce_vec3_copy(&sphere->center, &aabb->center);
	sphere->radius = ce_vec3_dist(&aabb->min, &aabb->center);
	return sphere;
}

ce_sphere* ce_sphere_init_zero(ce_sphere* sphere)
{
	ce_vec3_zero(&sphere->center);
	sphere->radius = 0.0f;
	return sphere;
}

ce_sphere* ce_sphere_copy(ce_sphere* sphere, const ce_sphere* other)
{
	ce_vec3_copy(&sphere->center, &other->center);
	sphere->radius = other->radius;
	return sphere;
}

ce_sphere* ce_sphere_merge(ce_sphere* sphere,
							const ce_sphere* lhs,
							const ce_sphere* rhs)
{
	ce_vec3 v;
	ce_vec3_sub(&v, &rhs->center, &lhs->center);
	float l = ce_vec3_abs(&v);
	ce_vec3_scale(&v, &v, 0.5f * (l + rhs->radius - lhs->radius) / l);
	ce_vec3_add(&sphere->center, &lhs->center, &v);
	sphere->radius = 0.5f * (l + lhs->radius + rhs->radius);
	return sphere;
}

ce_sphere* ce_sphere_transform(ce_sphere* sphere,
								const ce_sphere* other,
								const ce_vec3* translation,
								const ce_quat* rotation)
{
	ce_vec3_rot(&sphere->center, &other->center, rotation);
	ce_vec3_add(&sphere->center, &sphere->center, translation);
	sphere->radius = other->radius;
	return sphere;
}
