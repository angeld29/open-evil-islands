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
#include <assert.h>

#include "cebbox.h"

ce_bbox* ce_bbox_clear(ce_bbox* bbox)
{
	ce_aabb_clear(&bbox->aabb);
	bbox->axis = CE_QUAT_IDENTITY;
	return bbox;
}

ce_bbox* ce_bbox_merge(ce_bbox* bbox, const ce_bbox* other)
{
	ce_vec3 xaxis, yaxis, zaxis;
	ce_quat_to_axes(&other->axis, &xaxis, &yaxis, &zaxis);

	ce_vec3_scale(&xaxis, &xaxis, other->aabb.extents.x);
	ce_vec3_scale(&yaxis, &yaxis, other->aabb.extents.y);
	ce_vec3_scale(&zaxis, &zaxis, other->aabb.extents.z);

	ce_aabb aabb;
	aabb.origin = other->aabb.origin;

	// calculate the aabb extents in local coord space
	// from the other extents and axes
	aabb.extents.x = fabsf(xaxis.x) + fabsf(yaxis.x) + fabsf(zaxis.x);
	aabb.extents.y = fabsf(xaxis.y) + fabsf(yaxis.y) + fabsf(zaxis.y);
	aabb.extents.z = fabsf(xaxis.z) + fabsf(yaxis.z) + fabsf(zaxis.z);

	ce_aabb_merge_aabb(&bbox->aabb, &aabb);
	ce_aabb_update_radius(&bbox->aabb);

	bbox->axis = CE_QUAT_IDENTITY;

	return bbox;
}
