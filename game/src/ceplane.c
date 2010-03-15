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

ce_plane* ce_plane_init(ce_plane* r, float a, float b, float c, float d)
{
	ce_vec3_init(&r->n, a, b, c);
	r->d = d;
	return r;
}

ce_plane* ce_plane_init_array(ce_plane* r, const float* v)
{
	ce_vec3_init_array(&r->n, v);
	r->d = v[3];
	return r;
}

ce_plane* ce_plane_init_tri(ce_plane* r, const ce_vec3* a,
							const ce_vec3* b, const ce_vec3* c)
{
	ce_vec3 e1, e2;
	ce_vec3_sub(&e1, b, a);
	ce_vec3_sub(&e2, c, a);
	ce_vec3_cross(&r->n, &e1, &e2);
	ce_vec3_normalise(&r->n, &r->n);
	r->d = -ce_vec3_dot(&r->n, a);
	return r;
}

ce_plane* ce_plane_normalise(ce_plane* r, const ce_plane* a)
{
	const float s = 1.0f / ce_vec3_len(&a->n);
	ce_vec3_scale(&r->n, s, &a->n);
	r->d = s * a->d;
	return r;
}

float ce_plane_dist(const ce_plane* a, const ce_vec3* b)
{
	return ce_vec3_dot(&a->n, b) + a->d;
}
