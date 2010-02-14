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

#include "cevec4.h"

ce_vec4* ce_vec4_init(ce_vec4* r, float x, float y, float z, float w)
{
	r->x = x;
	r->y = y;
	r->z = z;
	r->w = w;
	return r;
}

ce_vec4* ce_vec4_init_vector(ce_vec4* r, float* v)
{
	r->x = *v++;
	r->y = *v++;
	r->z = *v++;
	r->w = *v++;
	return r;
}

ce_vec4* ce_vec4_copy(ce_vec4* r, const ce_vec4* a)
{
	r->x = a->x;
	r->y = a->y;
	r->z = a->z;
	r->w = a->w;
	return r;
}
