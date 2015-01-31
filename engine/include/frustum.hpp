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

#ifndef CE_FRUSTUM_HPP
#define CE_FRUSTUM_HPP

#include <stdbool.h>

#include "vec3.hpp"
#include "quat.hpp"
#include "sphere.hpp"
#include "aabb.hpp"
#include "bbox.hpp"
#include "plane.hpp"

typedef enum {
    CE_FRUSTUM_PLANE_TOP,
    CE_FRUSTUM_PLANE_BOTTOM,
    CE_FRUSTUM_PLANE_LEFT,
    CE_FRUSTUM_PLANE_RIGHT,
    CE_FRUSTUM_PLANE_NEAR,
    CE_FRUSTUM_PLANE_FAR,
    CE_FRUSTUM_PLANE_COUNT
} ce_frustum_plane;

typedef struct {
    ce_plane planes[CE_FRUSTUM_PLANE_COUNT];
} ce_frustum;

extern ce_frustum* ce_frustum_init(ce_frustum* frustum,
                                    float fov, float aspect,
                                    float near, float far,
                                    const ce_vec3* position,
                                    const ce_vec3* forward,
                                    const ce_vec3* right,
                                    const ce_vec3* up);

extern bool ce_frustum_test_point(const ce_frustum* frustum,
                                    const ce_vec3* point);
extern bool ce_frustum_test_sphere(const ce_frustum* frustum,
                                    const ce_sphere* sphere);
extern bool ce_frustum_test_aabb(const ce_frustum* frustum,
                                    const ce_aabb* aabb);
extern bool ce_frustum_test_bbox(const ce_frustum* frustum,
                                    const ce_bbox* bbox);

#endif /* CE_FRUSTUM_HPP */
