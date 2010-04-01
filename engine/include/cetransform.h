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

#ifndef CE_TRANSFORM_H
#define CE_TRANSFORM_H

#include "cevec3.h"
#include "cequat.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	ce_vec3 translation;
	ce_quat rotation;
	ce_vec3 scaling;
} ce_transform;

extern const ce_transform CE_TRANSFORM_IDENTITY;

extern ce_transform*
ce_transform_init(ce_transform* transform, const ce_vec3* translation,
											const ce_quat* rotation,
											const ce_vec3* scaling);

extern ce_transform* ce_transform_init_identity(ce_transform* transform);

extern ce_transform*
ce_transform_copy(ce_transform* transform, const ce_transform* other);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_TRANSFORM_H */
