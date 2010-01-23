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

#include "vec2.h"

const vec2 VEC2_ZERO = { .x = 0.0f, .y = 0.0f };
const vec2 VEC2_UNIT_X = { .x = 1.0f, .y = 0.0f };
const vec2 VEC2_UNIT_Y = { .x = 0.0f, .y = 1.0f };
const vec2 VEC2_UNIT_SCALE = { .x = 1.0f, .y = 1.0f };
const vec2 VEC2_NEG_UNIT_X = { .x = -1.0f, .y = 0.0f };
const vec2 VEC2_NEG_UNIT_Y = { .x = 0.0f, .y = -1.0f };
const vec2 VEC2_NEG_UNIT_SCALE = { .x = -1.0f, .y = -1.0f };

vec2* vec2_zero(vec2* r)
{
	r->x = 0.0f;
	r->y = 0.0f;
	return r;
}

vec2* vec2_unit_x(vec2* r)
{
	r->x = 1.0f;
	r->y = 0.0f;
	return r;
}

vec2* vec2_unit_y(vec2* r)
{
	r->x = 0.0f;
	r->y = 1.0f;
	return r;
}

vec2* vec2_unit_scale(vec2* r)
{
	r->x = 1.0f;
	r->y = 1.0f;
	return r;
}

vec2* vec2_neg_unit_x(vec2* r)
{
	r->x = -1.0f;
	r->y = 0.0f;
	return r;
}

vec2* vec2_neg_unit_y(vec2* r)
{
	r->x = 0.0f;
	r->y = -1.0f;
	return r;
}

vec2* vec2_neg_unit_scale(vec2* r)
{
	r->x = -1.0f;
	r->y = -1.0f;
	return r;
}

vec2* vec2_init(float x, float y, vec2* r)
{
	r->x = x;
	r->y = y;
	return r;
}

vec2* vec2_copy(const vec2* a, vec2* r)
{
	r->x = a->x;
	r->y = a->y;
	return r;
}

vec2* vec2_neg(const vec2* a, vec2* r)
{
	r->x = -a->x;
	r->y = -a->y;
	return r;
}
