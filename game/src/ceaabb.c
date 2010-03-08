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

#include "ceaabb.h"

ce_aabb* ce_aabb_init(ce_aabb* aabb, const ce_vec3* min, const ce_vec3* max)
{
	ce_vec3_copy(&aabb->min, min);
	ce_vec3_copy(&aabb->max, max);
	ce_vec3_mid(&aabb->center, min, max);
	return aabb;
}

ce_aabb* ce_aabb_init_zero(ce_aabb* aabb)
{
	ce_vec3_zero(&aabb->min);
	ce_vec3_zero(&aabb->max);
	ce_vec3_zero(&aabb->center);
	return aabb;
}

ce_aabb* ce_aabb_copy(ce_aabb* aabb, const ce_aabb* other)
{
	ce_vec3_copy(&aabb->min, &other->min);
	ce_vec3_copy(&aabb->max, &other->max);
	ce_vec3_copy(&aabb->center, &other->center);
	return aabb;
}

ce_aabb* ce_aabb_merge(ce_aabb* aabb, const ce_aabb* lhs, const ce_aabb* rhs)
{
	ce_vec3 min, max;
	ce_aabb_init(aabb, ce_vec3_floor(&min, &lhs->min, &rhs->min),
						ce_vec3_ceil(&max, &lhs->max, &rhs->max));
	return aabb;
}

ce_aabb* ce_aabb_transform(ce_aabb* aabb,
							const ce_aabb* other,
							const ce_vec3* translation,
							const ce_quat* rotation)
{
	ce_vec3_rot(&aabb->min, &other->min, rotation);
	ce_vec3_add(&aabb->min, &aabb->min, translation);
	ce_vec3_rot(&aabb->max, &other->max, rotation);
	ce_vec3_add(&aabb->max, &aabb->max, translation);
	ce_vec3_mid(&aabb->center, &aabb->min, &aabb->max);
	return aabb;
}
