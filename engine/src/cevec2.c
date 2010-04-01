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

#include "cevec2.h"

const ce_vec2 CE_VEC2_ZERO = { .x = 0.0f, .y = 0.0f };
const ce_vec2 CE_VEC2_UNIT_X = { .x = 1.0f, .y = 0.0f };
const ce_vec2 CE_VEC2_UNIT_Y = { .x = 0.0f, .y = 1.0f };
const ce_vec2 CE_VEC2_UNIT_SCALE = { .x = 1.0f, .y = 1.0f };
const ce_vec2 CE_VEC2_NEG_UNIT_X = { .x = -1.0f, .y = 0.0f };
const ce_vec2 CE_VEC2_NEG_UNIT_Y = { .x = 0.0f, .y = -1.0f };
const ce_vec2 CE_VEC2_NEG_UNIT_SCALE = { .x = -1.0f, .y = -1.0f };

ce_vec2* ce_vec2_init(ce_vec2* vec, float x, float y)
{
	vec->x = x;
	vec->y = y;
	return vec;
}

ce_vec2* ce_vec2_init_array(ce_vec2* vec, const float* array)
{
	vec->x = *array++;
	vec->y = *array;
	return vec;
}

ce_vec2* ce_vec2_init_zero(ce_vec2* vec)
{
	vec->x = 0.0f;
	vec->y = 0.0f;
	return vec;
}

ce_vec2* ce_vec2_init_unit_x(ce_vec2* vec)
{
	vec->x = 1.0f;
	vec->y = 0.0f;
	return vec;
}

ce_vec2* ce_vec2_init_unit_y(ce_vec2* vec)
{
	vec->x = 0.0f;
	vec->y = 1.0f;
	return vec;
}

ce_vec2* ce_vec2_init_unit_scale(ce_vec2* vec)
{
	vec->x = 1.0f;
	vec->y = 1.0f;
	return vec;
}

ce_vec2* ce_vec2_init_neg_unit_x(ce_vec2* vec)
{
	vec->x = -1.0f;
	vec->y = 0.0f;
	return vec;
}

ce_vec2* ce_vec2_init_neg_unit_y(ce_vec2* vec)
{
	vec->x = 0.0f;
	vec->y = -1.0f;
	return vec;
}

ce_vec2* ce_vec2_init_neg_unit_scale(ce_vec2* vec)
{
	vec->x = -1.0f;
	vec->y = -1.0f;
	return vec;
}

ce_vec2* ce_vec2_copy(ce_vec2* vec, const ce_vec2* other)
{
	vec->x = other->x;
	vec->y = other->y;
	return vec;
}

ce_vec2* ce_vec2_neg(ce_vec2* vec, const ce_vec2* other)
{
	vec->x = -other->x;
	vec->y = -other->y;
	return vec;
}
