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

#ifndef CE_AABB_H
#define CE_AABB_H

#include "cevec3.h"
#include "cequat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ce_vec3 origin, extents;
    float radius;
} ce_aabb;

extern ce_aabb* ce_aabb_init(ce_aabb* aabb,
                                const ce_vec3* origin,
                                const ce_vec3* extents,
                                float radius);

extern ce_aabb* ce_aabb_init_zero(ce_aabb* aabb);

extern ce_aabb* ce_aabb_copy(ce_aabb* aabb, const ce_aabb* other);

extern ce_aabb* ce_aabb_clear(ce_aabb* aabb);
extern ce_aabb* ce_aabb_update_radius(ce_aabb* aabb);

extern ce_aabb* ce_aabb_merge_aabb(ce_aabb* aabb, const ce_aabb* other);
extern ce_aabb* ce_aabb_merge_point(ce_aabb* aabb, const ce_vec3* point);
extern ce_aabb* ce_aabb_merge_point_array(ce_aabb* aabb, const float* point);

#ifdef __cplusplus
}
#endif

#endif /* CE_AABB_H */
