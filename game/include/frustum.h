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

#ifndef CE_FRUSTUM_H
#define CE_FRUSTUM_H

#include <stdbool.h>

#include "vec3.h"
#include "ceaabb.h"
#include "plane.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	FRUSTUM_PLANE_TOP,
	FRUSTUM_PLANE_BOTTOM,
	FRUSTUM_PLANE_LEFT,
	FRUSTUM_PLANE_RIGHT,
	FRUSTUM_PLANE_NEAR,
	FRUSTUM_PLANE_FAR,
	FRUSTUM_PLANE_COUNT
} frustum_plane;

typedef struct {
	plane p[FRUSTUM_PLANE_COUNT];
} frustum;

extern frustum* frustum_init(float fov, float aspect, float near, float far,
							const vec3* eye, const vec3* forward,
							const vec3* right, const vec3* up, frustum* f);

extern bool frustum_test_point(const vec3* p, const frustum* f);
extern bool frustum_test_sphere(const vec3* p, float r, const frustum* f);
extern bool frustum_test_box(const ce_aabb* b, const frustum* f);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FRUSTUM_H */
