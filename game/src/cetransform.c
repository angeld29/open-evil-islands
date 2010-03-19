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

#include "cetransform.h"

const ce_transform CE_TRANSFORM_IDENTITY = {
	.translation = { .x = 0.0f, .y = 0.0f, .z = 0.0f },
	.rotation = { .w = 1.0f, .x = 0.0f, .y = 0.0f, .z = 0.0f },
	.scaling = { .x = 1.0f, .y = 1.0f, .z = 1.0f }
};

ce_transform*
ce_transform_init(ce_transform* transform, const ce_vec3* translation,
											const ce_quat* rotation,
											const ce_vec3* scaling)
{
	transform->translation = *translation;
	transform->rotation = *rotation;
	transform->scaling = *scaling;
	return transform;
}

ce_transform* ce_transform_init_identity(ce_transform* transform)
{
	transform->translation = CE_VEC3_ZERO;
	transform->rotation = CE_QUAT_IDENTITY;
	transform->scaling = CE_VEC3_UNIT_SCALE;
	return transform;
}

ce_transform*
ce_transform_copy(ce_transform* transform, const ce_transform* other)
{
	transform->translation = other->translation;
	transform->rotation = other->rotation;
	transform->scaling = other->scaling;
	return transform;
}
