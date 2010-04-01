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

ce_vec4* ce_vec4_init(ce_vec4* vec, float x, float y, float z, float w)
{
	vec->x = x;
	vec->y = y;
	vec->z = z;
	vec->w = w;
	return vec;
}

ce_vec4* ce_vec4_init_array(ce_vec4* vec, const float* array)
{
	vec->x = *array++;
	vec->y = *array++;
	vec->z = *array++;
	vec->w = *array;
	return vec;
}

ce_vec4* ce_vec4_copy(ce_vec4* vec, const ce_vec4* other)
{
	vec->x = other->x;
	vec->y = other->y;
	vec->z = other->z;
	vec->w = other->w;
	return vec;
}
