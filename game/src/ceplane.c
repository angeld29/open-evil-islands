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

#include "ceplane.h"

ce_plane* ce_plane_init(ce_plane* plane, float a, float b, float c, float d)
{
	plane->n.x = a;
	plane->n.y = b;
	plane->n.z = c;
	plane->d = d;
	return plane;
}

ce_plane* ce_plane_init_array(ce_plane* plane, const float* array)
{
	plane->n.x = *array++;
	plane->n.y = *array++;
	plane->n.z = *array++;
	plane->d = *array;
	return plane;
}

ce_plane* ce_plane_init_tri(ce_plane* plane, const ce_vec3* a,
											const ce_vec3* b,
											const ce_vec3* c)
{
	ce_vec3 e1, e2;
	ce_vec3_sub(&e1, b, a);
	ce_vec3_sub(&e2, c, a);
	ce_vec3_cross(&plane->n, &e1, &e2);
	ce_vec3_norm(&plane->n, &plane->n);
	plane->d = -ce_vec3_dot(&plane->n, a);
	return plane;
}

ce_plane* ce_plane_normalise(ce_plane* plane, const ce_plane* other)
{
	const float s = 1.0f / ce_vec3_len(&other->n);
	ce_vec3_scale(&plane->n, s, &other->n);
	plane->d = s * other->d;
	return plane;
}

float ce_plane_dist(const ce_plane* plane, const ce_vec3* vec)
{
	return ce_vec3_dot(&plane->n, vec) + plane->d;
}
