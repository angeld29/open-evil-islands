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

#include "quat.h"
#include "vec3.h"

const vec3 VEC3_ZERO = { .x = 0.0f, .y = 0.0f, .z = 0.0f };
const vec3 VEC3_UNIT_X = { .x = 1.0f, .y = 0.0f, .z = 0.0f };
const vec3 VEC3_UNIT_Y = { .x = 0.0f, .y = 1.0f, .z = 0.0f };
const vec3 VEC3_UNIT_Z = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
const vec3 VEC3_UNIT_SCALE = { .x = 1.0f, .y = 1.0f, .z = 1.0f };
const vec3 VEC3_NEG_UNIT_X = { .x = -1.0f, .y = 0.0f, .z = 0.0f };
const vec3 VEC3_NEG_UNIT_Y = { .x = 0.0f, .y = -1.0f, .z = 0.0f };
const vec3 VEC3_NEG_UNIT_Z = { .x = 0.0f, .y = 0.0f, .z = -1.0f };
const vec3 VEC3_NEG_UNIT_SCALE = { .x = -1.0f, .y = -1.0f, .z = -1.0f };

vec3* vec3_zero(vec3* r)
{
	r->x = 0.0f;
	r->y = 0.0f;
	r->z = 0.0f;
	return r;
}

vec3* vec3_unit_x(vec3* r)
{
	r->x = 1.0f;
	r->y = 0.0f;
	r->z = 0.0f;
	return r;
}

vec3* vec3_unit_y(vec3* r)
{
	r->x = 0.0f;
	r->y = 1.0f;
	r->z = 0.0f;
	return r;
}

vec3* vec3_unit_z(vec3* r)
{
	r->x = 0.0f;
	r->y = 0.0f;
	r->z = 1.0f;
	return r;
}

vec3* vec3_unit_scale(vec3* r)
{
	r->x = 1.0f;
	r->y = 1.0f;
	r->z = 1.0f;
	return r;
}

vec3* vec3_neg_unit_x(vec3* r)
{
	r->x = -1.0f;
	r->y = 0.0f;
	r->z = 0.0f;
	return r;
}

vec3* vec3_neg_unit_y(vec3* r)
{
	r->x = 0.0f;
	r->y = -1.0f;
	r->z = 0.0f;
	return r;
}

vec3* vec3_neg_unit_z(vec3* r)
{
	r->x = 0.0f;
	r->y = 0.0f;
	r->z = -1.0f;
	return r;
}

vec3* vec3_neg_unit_scale(vec3* r)
{
	r->x = -1.0f;
	r->y = -1.0f;
	r->z = -1.0f;
	return r;
}

vec3* vec3_init(float x, float y, float z, vec3* r)
{
	r->x = x;
	r->y = y;
	r->z = z;
	return r;
}

vec3* vec3_copy(const vec3* a, vec3* r)
{
	r->x = a->x;
	r->y = a->y;
	r->z = a->z;
	return r;
}

vec3* vec3_neg(const vec3* a, vec3* r)
{
	r->x = -a->x;
	r->y = -a->y;
	r->z = -a->z;
	return r;
}

vec3* vec3_add(const vec3* a, const vec3* b, vec3* r)
{
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
	return r;
}

vec3* vec3_sub(const vec3* a, const vec3* b, vec3* r)
{
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
	return r;
}

vec3* vec3_mul(const vec3* a, const vec3* b, vec3* r)
{
	r->x = a->x * b->x;
	r->y = a->y * b->y;
	r->z = a->z * b->z;
	return r;
}

vec3* vec3_div(const vec3* a, const vec3* b, vec3* r)
{
	r->x = a->x / b->x;
	r->y = a->y / b->y;
	r->z = a->z / b->z;
	return r;
}

vec3* vec3_scale(const vec3* a, float s, vec3* r)
{
	r->x = a->x * s;
	r->y = a->y * s;
	r->z = a->z * s;
	return r;
}

float vec3_abs(const vec3* a)
{
	return sqrtf(vec3_abs2(a));
}

float vec3_abs2(const vec3* a)
{
	return a->x * a->x + a->y * a->y + a->z * a->z;
}

float vec3_dist(const vec3* a, const vec3* b)
{
	vec3 c;
	return vec3_abs(vec3_sub(a, b, &c));
}

float vec3_dist2(const vec3* a, const vec3* b)
{
	vec3 c;
	return vec3_abs2(vec3_sub(a, b, &c));
}

vec3* vec3_normalise(const vec3* a, vec3* r)
{
	return vec3_scale(a, 1.0f / vec3_abs(a), r);
}

float vec3_dot(const vec3* a, const vec3* b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

float vec3_absdot(const vec3* a, const vec3* b)
{
	return fabsf(a->x * b->x) + fabsf(a->y * b->y) + fabsf(a->z * b->z);
}

vec3* vec3_cross(const vec3* a, const vec3* b, vec3* restrict r)
{
	r->x = a->y * b->z - a->z * b->y;
	r->y = a->z * b->x - a->x * b->z;
	r->z = a->x * b->y - a->y * b->x;
	return r;
}

vec3* vec3_mid(const vec3* a, const vec3* b, vec3* r)
{
	r->x = 0.5f * (a->x + b->x);
	r->y = 0.5f * (a->y + b->y);
	r->z = 0.5f * (a->z + b->z);
	return r;
}

vec3* vec3_rot(const vec3* a, const quat* b, vec3* r)
{
	vec3 qv, uv, uuv;
	vec3_init(b->x, b->y, b->z, &qv);
	vec3_cross(&qv, a, &uv);
	vec3_cross(&qv, &uv, &uuv);
	vec3_scale(&uv, 2.0f * b->w, &uv);
	vec3_scale(&uuv, 2.0f, &uuv);
	return vec3_add(vec3_add(a, &uv, r), &uuv, r);
}
