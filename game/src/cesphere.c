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

#include <math.h>

#include "cesphere.h"

ce_sphere*
ce_sphere_init(ce_sphere* sphere, const ce_vec3* origin, float radius)
{
	sphere->origin = *origin;
	sphere->radius = radius;
	return sphere;
}

ce_sphere* ce_sphere_init_zero(ce_sphere* sphere)
{
	sphere->origin = CE_VEC3_ZERO;
	sphere->radius = 0.0f;
	return sphere;
}

ce_sphere* ce_sphere_copy(ce_sphere* sphere, const ce_sphere* other)
{
	sphere->origin = other->origin;
	sphere->radius = other->radius;
	return sphere;
}
