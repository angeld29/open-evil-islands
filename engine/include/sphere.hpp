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

#ifndef CE_SPHERE_HPP
#define CE_SPHERE_HPP

#include "vec3.hpp"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
    ce_vec3 origin;
    float radius;
} ce_sphere;

extern ce_sphere* ce_sphere_init(ce_sphere* sphere,
                                    const ce_vec3* origin,
                                    float radius);
extern ce_sphere* ce_sphere_init_array(ce_sphere* sphere, const float* array);

extern ce_sphere* ce_sphere_init_zero(ce_sphere* sphere);

extern ce_sphere* ce_sphere_copy(ce_sphere* sphere, const ce_sphere* other);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SPHERE_HPP */
