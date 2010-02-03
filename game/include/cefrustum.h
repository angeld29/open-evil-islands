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

#include "cevec3.h"
#include "ceaabb.h"
#include "ceplane.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

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
	ce_plane p[CE_FRUSTUM_PLANE_COUNT];
} ce_frustum;

extern ce_frustum* ce_frustum_init(ce_frustum* f, float fov, float aspect,
									float near, float far,
									const ce_vec3* eye, const ce_vec3* forward,
									const ce_vec3* right, const ce_vec3* up);

extern bool ce_frustum_test_point(const ce_frustum* f, const ce_vec3* p);
extern bool ce_frustum_test_sphere(const ce_frustum* f, const ce_vec3* p,
																float r);
extern bool ce_frustum_test_box(const ce_frustum* f, const ce_aabb* b);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FRUSTUM_H */
