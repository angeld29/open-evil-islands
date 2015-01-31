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

#ifndef CE_RAY_H
#define CE_RAY_H

#include "cevec3.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ce_vec3 origin;
    ce_vec3 direction;
} ce_ray;

static inline ce_ray* ce_ray_init(ce_ray* ray, const ce_vec3* origin,
                                                const ce_vec3* direction)
{
    ce_vec3_copy(&ray->origin, origin);
    ce_vec3_copy(&ray->direction, direction);
    return ray;
}

static inline ce_ray* ce_ray_init_segment(ce_ray* ray, const ce_vec3* start,
                                                        const ce_vec3* end)
{
    ce_vec3_copy(&ray->origin, start);
    ce_vec3_norm(&ray->direction, ce_vec3_sub(&ray->direction, end, start));
    return ray;
}

#ifdef __cplusplus
}
#endif

#endif /* CE_RAY_H */
