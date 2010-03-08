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

#ifndef CE_AABB_H
#define CE_AABB_H

#include "cevec3.h"
#include "cequat.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_vec3 min, max, center;
} ce_aabb;

extern ce_aabb* ce_aabb_init(ce_aabb* aabb,
								const ce_vec3* min,
								const ce_vec3* max);

extern ce_aabb* ce_aabb_init_zero(ce_aabb* aabb);

extern ce_aabb* ce_aabb_copy(ce_aabb* aabb, const ce_aabb* other);

extern ce_aabb* ce_aabb_merge(ce_aabb* aabb,
								const ce_aabb* lhs,
								const ce_aabb* rhs);

extern ce_aabb* ce_aabb_transform(ce_aabb* aabb,
									const ce_aabb* other,
									const ce_vec3* translation,
									const ce_quat* rotation);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_AABB_H */
