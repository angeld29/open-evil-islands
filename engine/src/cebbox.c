/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <stddef.h>
#include <float.h>
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
    ce_vec3 axis_x, axis_y, axis_z;
    ce_quat_to_axes(&other->axis, &axis_x, &axis_y, &axis_z);

    ce_vec3_scale(&axis_x, other->aabb.extents.x, &axis_x);
    ce_vec3_scale(&axis_y, other->aabb.extents.y, &axis_y);
    ce_vec3_scale(&axis_z, other->aabb.extents.z, &axis_z);

    ce_aabb aabb;
    aabb.origin = other->aabb.origin;

    // calculate the aabb extents in local coord space
    // from the other extents and axes
    aabb.extents.x = fabsf(axis_x.x) + fabsf(axis_y.x) + fabsf(axis_z.x);
    aabb.extents.y = fabsf(axis_x.y) + fabsf(axis_y.y) + fabsf(axis_z.y);
    aabb.extents.z = fabsf(axis_x.z) + fabsf(axis_y.z) + fabsf(axis_z.z);

    ce_aabb_merge_aabb(&bbox->aabb, &aabb);
    ce_aabb_update_radius(&bbox->aabb);

    bbox->axis = CE_QUAT_IDENTITY;

    return bbox;
}

static void ce_bbox_compute_corners(const ce_bbox* bbox, ce_vec3 corners[8])
{
    ce_vec3 axis_x, axis_y, axis_z;
    ce_quat_to_axes(&bbox->axis, &axis_x, &axis_y, &axis_z);

    ce_vec3_scale(&axis_x, bbox->aabb.extents.x, &axis_x);
    ce_vec3_scale(&axis_y, bbox->aabb.extents.y, &axis_y);
    ce_vec3_scale(&axis_z, bbox->aabb.extents.z, &axis_z);

    for (size_t i = 0; i < 8; ++i) {
        ce_vec3_copy(&corners[i], &bbox->aabb.origin);
    }

    ce_vec3_sub(&corners[0], &corners[0], &axis_x);
    ce_vec3_sub(&corners[0], &corners[0], &axis_y);
    ce_vec3_sub(&corners[0], &corners[0], &axis_z);

    ce_vec3_add(&corners[1], &corners[1], &axis_x);
    ce_vec3_sub(&corners[1], &corners[1], &axis_y);
    ce_vec3_sub(&corners[1], &corners[1], &axis_z);

    ce_vec3_add(&corners[2], &corners[2], &axis_x);
    ce_vec3_add(&corners[2], &corners[2], &axis_y);
    ce_vec3_sub(&corners[2], &corners[2], &axis_z);

    ce_vec3_sub(&corners[3], &corners[3], &axis_x);
    ce_vec3_add(&corners[3], &corners[3], &axis_y);
    ce_vec3_sub(&corners[3], &corners[3], &axis_z);

    ce_vec3_sub(&corners[4], &corners[4], &axis_x);
    ce_vec3_sub(&corners[4], &corners[4], &axis_y);
    ce_vec3_add(&corners[4], &corners[4], &axis_z);

    ce_vec3_add(&corners[5], &corners[5], &axis_x);
    ce_vec3_sub(&corners[5], &corners[5], &axis_y);
    ce_vec3_add(&corners[5], &corners[5], &axis_z);

    ce_vec3_add(&corners[6], &corners[6], &axis_x);
    ce_vec3_add(&corners[6], &corners[6], &axis_y);
    ce_vec3_add(&corners[6], &corners[6], &axis_z);

    ce_vec3_sub(&corners[7], &corners[7], &axis_x);
    ce_vec3_add(&corners[7], &corners[7], &axis_y);
    ce_vec3_add(&corners[7], &corners[7], &axis_z);
}

static void ce_bbox_find_min_max(const ce_vec3* center, const ce_vec3* axis_x,
                                const ce_vec3* axis_y, const ce_vec3* axis_z,
                                ce_vec3 corners[8], ce_vec3* min, ce_vec3* max)
{
    float dot;

    for (size_t i = 0; i < 8; ++i) {
        ce_vec3_sub(&corners[i], &corners[i], center);

        dot = ce_vec3_dot(&corners[i], axis_x);
        if (dot > max->x) {
            max->x = dot;
        } else if (dot < min->x) {
            min->x = dot;
        }

        dot = ce_vec3_dot(&corners[i], axis_y);
        if (dot > max->y) {
            max->y = dot;
        } else if (dot < min->y) {
            min->y = dot;
        }

        dot = ce_vec3_dot(&corners[i], axis_z);
        if (dot > max->z) {
            max->z = dot;
        } else if (dot < min->z) {
            min->z = dot;
        }
    }
}

ce_bbox* ce_bbox_merge2(ce_bbox* bbox, const ce_bbox* other)
{
    if (-FLT_MAX == bbox->aabb.extents.x) {
        *bbox = *other;
        return bbox;
    }

    ce_quat axis;
    if (ce_quat_dot(&bbox->axis, &other->axis) < 0.0f) {
        ce_quat_neg(&axis, &other->axis);
    } else {
        ce_quat_copy(&axis, &other->axis);
    }

    ce_quat_add(&axis, &axis, &bbox->axis);
    ce_quat_norm(&axis, &axis);

    ce_vec3 axis_x, axis_y, axis_z;
    ce_quat_to_axes(&axis, &axis_x, &axis_y, &axis_z);

    ce_vec3 center;
    ce_vec3_mid(&center, &bbox->aabb.origin, &other->aabb.origin);

    ce_vec3 min, max;
    ce_vec3 corners[8];

    ce_vec3_init_zero(&min);
    ce_vec3_init_zero(&max);

    ce_bbox_compute_corners(bbox, corners);
    ce_quat_to_axes(&bbox->axis, &axis_x, &axis_y, &axis_z);
    ce_bbox_find_min_max(&center, &axis_x, &axis_y, &axis_z, corners, &min, &max);

    ce_bbox_compute_corners(other, corners);
    ce_quat_to_axes(&other->axis, &axis_x, &axis_y, &axis_z);
    ce_bbox_find_min_max(&center, &axis_x, &axis_y, &axis_z, corners, &min, &max);

    ce_quat_to_axes(&axis, &axis_x, &axis_y, &axis_z);

    ce_vec3_scale(&axis_x, 0.5f * (min.x + max.x), &axis_x);
    ce_vec3_scale(&axis_y, 0.5f * (min.y + max.y), &axis_y);
    ce_vec3_scale(&axis_z, 0.5f * (min.z + max.z), &axis_z);

    ce_vec3_add(&center, &center, &axis_x);
    ce_vec3_add(&center, &center, &axis_y);
    ce_vec3_add(&center, &center, &axis_z);

    ce_vec3_sub(&bbox->aabb.extents, &max, &min);
    ce_vec3_scale(&bbox->aabb.extents, 0.5f, &bbox->aabb.extents);

    bbox->aabb.origin = center;
    bbox->axis = axis;

    ce_aabb_update_radius(&bbox->aabb);

    return bbox;
}
