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

#include <cmath>

#include "utility.hpp"
#include "frustum.hpp"

namespace cursedearth
{
    ce_frustum* ce_frustum_init(ce_frustum* frustum, float fov, float aspect, float near, float far, const ce_vec3* position, const ce_vec3* forward, const ce_vec3* right, const ce_vec3* up)
    {
        float tang = tanf(0.5f * deg2rad(fov));
        float nh = tang * near;
        float nw = nh * aspect;
        float fh = tang * far;
        float fw = fh * aspect;

        ce_vec3 nc, fc, xw, yh;
        ce_vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;

        ce_vec3_add(&nc, position, ce_vec3_scale(&nc, near, forward));
        ce_vec3_add(&fc, position, ce_vec3_scale(&fc, far, forward));

        ce_vec3_scale(&xw, nw, right);
        ce_vec3_scale(&yh, nh, up);

        ce_vec3_sub(&ntl, ce_vec3_add(&ntl, &nc, &yh), &xw);
        ce_vec3_add(&ntr, ce_vec3_add(&ntr, &nc, &yh), &xw);
        ce_vec3_sub(&nbl, ce_vec3_sub(&nbl, &nc, &yh), &xw);
        ce_vec3_add(&nbr, ce_vec3_sub(&nbr, &nc, &yh), &xw);

        ce_vec3_scale(&xw, fw, right);
        ce_vec3_scale(&yh, fh, up);

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

    bool ce_frustum_test_sphere(const ce_frustum* frustum, const ce_sphere* sphere)
    {
        for (int i = 0; i < CE_FRUSTUM_PLANE_COUNT; ++i) {
            if (ce_plane_dist(&frustum->planes[i], &sphere->origin) < -sphere->radius) {
                return false;
            }
        }
        return true;
    }

    bool ce_frustum_test_aabb(const ce_frustum* frustum, const ce_aabb* aabb)
    {
        for (int i = 0; i < CE_FRUSTUM_PLANE_COUNT; ++i) {
            float dist = ce_plane_dist(&frustum->planes[i], &aabb->origin);

            // trivial test using bounding sphere
            if (fabsf(dist) > aabb->radius) {
                if (dist < 0.0f) {
                    return false;
                } else {
                    continue;
                }
            }

            // calculate extents distance relative to plane normal
            if (fabsf(dist) >= ce_vec3_absdot(&aabb->extents, &frustum->planes[i].n) && dist < 0.0f) {
                 return false;
             }
        }
        return true;
    }

    bool ce_frustum_test_bbox(const ce_frustum* frustum, const ce_bbox* bbox)
    {
        ce_vec3 xaxis, yaxis, zaxis, nb;
        ce_quat_to_axes(&bbox->axis, &xaxis, &yaxis, &zaxis);

        for (int i = 0; i < CE_FRUSTUM_PLANE_COUNT; ++i) {
            float dist = ce_plane_dist(&frustum->planes[i], &bbox->aabb.origin);

            // trivial test using bounding sphere
            if (fabsf(dist) > bbox->aabb.radius) {
                if (dist < 0.0f) {
                    return false;
                } else {
                    continue;
                }
            }

            // calculate extents distance relative to plane normal
            if (fabsf(dist) >= ce_vec3_absdot(&bbox->aabb.extents, ce_vec3_init(&nb,
                                ce_vec3_dot(&frustum->planes[i].n, &xaxis),
                                ce_vec3_dot(&frustum->planes[i].n, &yaxis),
                                ce_vec3_dot(&frustum->planes[i].n, &zaxis))) &&
                    dist < 0.0f) {
                 return false;
             }
        }

        return true;
    }
}
