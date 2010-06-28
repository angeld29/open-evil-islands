/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#include "cemath.h"
#include "cequat.h"
#include "cevec3.h"

const ce_vec3 CE_VEC3_ZERO = {.x = 0.0f, .y = 0.0f, .z = 0.0f};
const ce_vec3 CE_VEC3_UNIT_X = {.x = 1.0f, .y = 0.0f, .z = 0.0f};
const ce_vec3 CE_VEC3_UNIT_Y = {.x = 0.0f, .y = 1.0f, .z = 0.0f};
const ce_vec3 CE_VEC3_UNIT_Z = {.x = 0.0f, .y = 0.0f, .z = 1.0f};
const ce_vec3 CE_VEC3_UNIT_SCALE = {.x = 1.0f, .y = 1.0f, .z = 1.0f};
const ce_vec3 CE_VEC3_NEG_UNIT_X = {.x = -1.0f, .y = 0.0f, .z = 0.0f};
const ce_vec3 CE_VEC3_NEG_UNIT_Y = {.x = 0.0f, .y = -1.0f, .z = 0.0f};
const ce_vec3 CE_VEC3_NEG_UNIT_Z = {.x = 0.0f, .y = 0.0f, .z = -1.0f};
const ce_vec3 CE_VEC3_NEG_UNIT_SCALE = {.x = -1.0f, .y = -1.0f, .z = -1.0f};

ce_vec3* ce_vec3_init(ce_vec3* vec, float x, float y, float z)
{
	vec->x = x;
	vec->y = y;
	vec->z = z;
	return vec;
}

ce_vec3* ce_vec3_init_scalar(ce_vec3* vec, float s)
{
	vec->x = s;
	vec->y = s;
	vec->z = s;
	return vec;
}

ce_vec3* ce_vec3_init_array(ce_vec3* vec, const float* array)
{
	vec->x = array[0];
	vec->y = array[1];
	vec->z = array[2];
	return vec;
}

ce_vec3* ce_vec3_init_zero(ce_vec3* vec)
{
	vec->x = 0.0f;
	vec->y = 0.0f;
	vec->z = 0.0f;
	return vec;
}

ce_vec3* ce_vec3_init_unit_x(ce_vec3* vec)
{
	vec->x = 1.0f;
	vec->y = 0.0f;
	vec->z = 0.0f;
	return vec;
}

ce_vec3* ce_vec3_init_unit_y(ce_vec3* vec)
{
	vec->x = 0.0f;
	vec->y = 1.0f;
	vec->z = 0.0f;
	return vec;
}

ce_vec3* ce_vec3_init_unit_z(ce_vec3* vec)
{
	vec->x = 0.0f;
	vec->y = 0.0f;
	vec->z = 1.0f;
	return vec;
}

ce_vec3* ce_vec3_init_unit_scale(ce_vec3* vec)
{
	vec->x = 1.0f;
	vec->y = 1.0f;
	vec->z = 1.0f;
	return vec;
}

ce_vec3* ce_vec3_init_neg_unit_x(ce_vec3* vec)
{
	vec->x = -1.0f;
	vec->y = 0.0f;
	vec->z = 0.0f;
	return vec;
}

ce_vec3* ce_vec3_init_neg_unit_y(ce_vec3* vec)
{
	vec->x = 0.0f;
	vec->y = -1.0f;
	vec->z = 0.0f;
	return vec;
}

ce_vec3* ce_vec3_init_neg_unit_z(ce_vec3* vec)
{
	vec->x = 0.0f;
	vec->y = 0.0f;
	vec->z = -1.0f;
	return vec;
}

ce_vec3* ce_vec3_init_neg_unit_scale(ce_vec3* vec)
{
	vec->x = -1.0f;
	vec->y = -1.0f;
	vec->z = -1.0f;
	return vec;
}

ce_vec3* ce_vec3_copy(ce_vec3* vec, const ce_vec3* other)
{
	vec->x = other->x;
	vec->y = other->y;
	vec->z = other->z;
	return vec;
}

ce_vec3* ce_vec3_neg(ce_vec3* vec, const ce_vec3* other)
{
	vec->x = -other->x;
	vec->y = -other->y;
	vec->z = -other->z;
	return vec;
}

ce_vec3* ce_vec3_add(ce_vec3* vec, const ce_vec3* lhs, const ce_vec3* rhs)
{
	vec->x = lhs->x + rhs->x;
	vec->y = lhs->y + rhs->y;
	vec->z = lhs->z + rhs->z;
	return vec;
}

ce_vec3* ce_vec3_sub(ce_vec3* vec, const ce_vec3* lhs, const ce_vec3* rhs)
{
	vec->x = lhs->x - rhs->x;
	vec->y = lhs->y - rhs->y;
	vec->z = lhs->z - rhs->z;
	return vec;
}

ce_vec3* ce_vec3_mul(ce_vec3* vec, const ce_vec3* lhs, const ce_vec3* rhs)
{
	vec->x = lhs->x * rhs->x;
	vec->y = lhs->y * rhs->y;
	vec->z = lhs->z * rhs->z;
	return vec;
}

ce_vec3* ce_vec3_div(ce_vec3* vec, const ce_vec3* lhs, const ce_vec3* rhs)
{
	vec->x = lhs->x / rhs->x;
	vec->y = lhs->y / rhs->y;
	vec->z = lhs->z / rhs->z;
	return vec;
}

ce_vec3* ce_vec3_scale(ce_vec3* vec, float s, const ce_vec3* other)
{
	vec->x = s * other->x;
	vec->y = s * other->y;
	vec->z = s * other->z;
	return vec;
}

float ce_vec3_len(const ce_vec3* vec)
{
	return sqrtf(ce_vec3_len2(vec));
}

float ce_vec3_len2(const ce_vec3* vec)
{
	return vec->x * vec->x + vec->y * vec->y + vec->z * vec->z;
}

float ce_vec3_dist(const ce_vec3* lhs, const ce_vec3* rhs)
{
	return sqrtf(ce_vec3_dist2(lhs, rhs));
}

float ce_vec3_dist2(const ce_vec3* lhs, const ce_vec3* rhs)
{
	ce_vec3 tmp;
	return ce_vec3_len2(ce_vec3_sub(&tmp, rhs, lhs));
}

ce_vec3* ce_vec3_norm(ce_vec3* vec, const ce_vec3* other)
{
	return ce_vec3_scale(vec, 1.0f / ce_vec3_len(other), other);
}

float ce_vec3_dot(const ce_vec3* lhs, const ce_vec3* rhs)
{
	return lhs->x * rhs->x + lhs->y * rhs->y + lhs->z * rhs->z;
}

float ce_vec3_absdot(const ce_vec3* lhs, const ce_vec3* rhs)
{
	return fabsf(lhs->x * rhs->x) +
			fabsf(lhs->y * rhs->y) +
			fabsf(lhs->z * rhs->z);
}

ce_vec3* ce_vec3_cross(ce_vec3* restrict vec, const ce_vec3* lhs,
												const ce_vec3* rhs)
{
	vec->x = lhs->y * rhs->z - lhs->z * rhs->y;
	vec->y = lhs->z * rhs->x - lhs->x * rhs->z;
	vec->z = lhs->x * rhs->y - lhs->y * rhs->x;
	return vec;
}

ce_vec3* ce_vec3_mid(ce_vec3* vec, const ce_vec3* lhs, const ce_vec3* rhs)
{
	return ce_vec3_scale(vec, 0.5f, ce_vec3_add(vec, lhs, rhs));
}

ce_vec3* ce_vec3_rot(ce_vec3* vec, const ce_vec3* other, const ce_quat* quat)
{
	ce_vec3 qv = { quat->x, quat->y, quat->z }, uv, uuv;
	ce_vec3_cross(&uv, &qv, other);
	ce_vec3_cross(&uuv, &qv, &uv);
	ce_vec3_scale(&uv, 2.0f * quat->w, &uv);
	ce_vec3_scale(&uuv, 2.0f, &uuv);
	return ce_vec3_add(vec, ce_vec3_add(vec, other, &uv), &uuv);
}

ce_vec3* ce_vec3_lerp(ce_vec3* vec, float u, const ce_vec3* lhs,
												const ce_vec3* rhs)
{
	vec->x = ce_lerp(u, lhs->x, rhs->x);
	vec->y = ce_lerp(u, lhs->y, rhs->y);
	vec->z = ce_lerp(u, lhs->z, rhs->z);
	return vec;
}

ce_vec3* ce_vec3_floor(ce_vec3* vec, const ce_vec3* lhs, const ce_vec3* rhs)
{
	vec->x = fminf(lhs->x, rhs->x);
	vec->y = fminf(lhs->y, rhs->y);
	vec->z = fminf(lhs->z, rhs->z);
	return vec;
}

ce_vec3* ce_vec3_ceil(ce_vec3* vec, const ce_vec3* lhs, const ce_vec3* rhs)
{
	vec->x = fmaxf(lhs->x, rhs->x);
	vec->y = fmaxf(lhs->y, rhs->y);
	vec->z = fmaxf(lhs->z, rhs->z);
	return vec;
}
