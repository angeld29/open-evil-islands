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
 *  Based on:
 *  1. GTKRadiant mathlib,
 *     https://zerowing.idsoftware.com/svn/radiant/GtkRadiant.
 *     GTKRadiant contains software developed by Id Software,
 *     Loki Software and third party contributors.
*/

#include <float.h>
#include <math.h>
#include <assert.h>

#include "cemath.h"
#include "ceaabb.h"

ce_aabb* ce_aabb_init(ce_aabb* aabb, const ce_vec3* origin,
						const ce_vec3* extents, float radius)
{
	aabb->origin = *origin;
	aabb->extents = *extents;
	aabb->radius = radius;
	return aabb;
}

ce_aabb* ce_aabb_copy(ce_aabb* aabb, const ce_aabb* other)
{
	aabb->origin = other->origin;
	aabb->extents = other->extents;
	aabb->radius = other->radius;
	return aabb;
}

ce_aabb* ce_aabb_clear(ce_aabb* aabb)
{
	aabb->origin = CE_VEC3_ZERO;
	aabb->extents.x = aabb->extents.y = aabb->extents.z = -FLT_MAX;
	aabb->radius = 0.0f;
	return aabb;
}

ce_aabb* ce_aabb_update_radius(ce_aabb* aabb)
{
	aabb->radius = ce_vec3_abs(&aabb->extents);
	return aabb;
}

static void ce_aabb_merge_aabb_pass(float* origin, float* extents,
									float other_origin, float other_extents)
{
	float displacement = other_origin - *origin;
	float difference = other_extents - *extents;
	if (*extents < 0.0f || difference >= fabsf(displacement)) {
		// 2nd contains 1st
		*extents = other_extents;
		*origin = other_origin;
	} else if (other_extents < 0.0f || -difference >= fabsf(displacement)) {
		// 1st contains 2nd, do nothing
	} else {
		// not contained
		float min, max;
		if (displacement > 0.0f) {
			min = *origin - *extents;
			max = other_origin + other_extents;
		} else {
			min = other_origin - other_extents;
			max = *origin + *extents;
		}
		*origin = 0.5f * (min + max);
		*extents = max - *origin;
	}
}

ce_aabb* ce_aabb_merge_aabb(ce_aabb* aabb, const ce_aabb* other)
{
	ce_aabb_merge_aabb_pass(&aabb->origin.x, &aabb->extents.x,
							other->origin.x, other->extents.x);
	ce_aabb_merge_aabb_pass(&aabb->origin.y, &aabb->extents.y,
							other->origin.y, other->extents.y);
	ce_aabb_merge_aabb_pass(&aabb->origin.z, &aabb->extents.z,
							other->origin.z, other->extents.z);
	return aabb;
}

static void ce_aabb_merge_point_pass(float* origin, float* extents, float point)
{
	float displacement = point - *origin;
	if (fabsf(displacement) > *extents) {
		float min, max;
		if (*extents < 0.0f) { // degenerate
			min = max = point;
		} else if (displacement > 0.0f) {
			min = *origin - *extents;
			max = *origin + displacement;
		} else {
			max = *origin + *extents;
			min = *origin + displacement;
		}
		*origin = 0.5f * (min + max);
		*extents = max - *origin;
	}
}

ce_aabb* ce_aabb_merge_point(ce_aabb* aabb, const ce_vec3* point)
{
	ce_aabb_merge_point_pass(&aabb->origin.x, &aabb->extents.x, point->x);
	ce_aabb_merge_point_pass(&aabb->origin.y, &aabb->extents.y, point->y);
	ce_aabb_merge_point_pass(&aabb->origin.z, &aabb->extents.z, point->z);
	return aabb;
}

ce_aabb* ce_aabb_merge_point_array(ce_aabb* aabb, const float* point)
{
	ce_aabb_merge_point_pass(&aabb->origin.x, &aabb->extents.x, *point++);
	ce_aabb_merge_point_pass(&aabb->origin.y, &aabb->extents.y, *point++);
	ce_aabb_merge_point_pass(&aabb->origin.z, &aabb->extents.z, *point++);
	return aabb;
}
