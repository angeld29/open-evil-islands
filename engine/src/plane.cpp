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

#include "math.hpp"
#include "plane.hpp"

bool ce_plane_isect_ray(const ce_plane* plane, const ce_ray* ray, ce_vec3* point)
{
    float dist = ce_plane_dist_ray(plane, ray);
    if (dist < 0.0f || ce_fiszero(dist, CE_EPS_E6)) {
        return false;
    }
    ce_vec3_scale(point, dist, &ray->direction);
    ce_vec3_add(point, &ray->origin, point);
    return true;
}
