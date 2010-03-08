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

#include "cemath.h"
#include "cequat.h"
#include "cevec3.h"

const ce_vec3 CE_VEC3_ZERO = { .x = 0.0f, .y = 0.0f, .z = 0.0f };
const ce_vec3 CE_VEC3_UNIT_X = { .x = 1.0f, .y = 0.0f, .z = 0.0f };
const ce_vec3 CE_VEC3_UNIT_Y = { .x = 0.0f, .y = 1.0f, .z = 0.0f };
const ce_vec3 CE_VEC3_UNIT_Z = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
const ce_vec3 CE_VEC3_UNIT_SCALE = { .x = 1.0f, .y = 1.0f, .z = 1.0f };
const ce_vec3 CE_VEC3_NEG_UNIT_X = { .x = -1.0f, .y = 0.0f, .z = 0.0f };
const ce_vec3 CE_VEC3_NEG_UNIT_Y = { .x = 0.0f, .y = -1.0f, .z = 0.0f };
const ce_vec3 CE_VEC3_NEG_UNIT_Z = { .x = 0.0f, .y = 0.0f, .z = -1.0f };
const ce_vec3 CE_VEC3_NEG_UNIT_SCALE = { .x = -1.0f, .y = -1.0f, .z = -1.0f };

ce_vec3* ce_vec3_zero(ce_vec3* r)
{
	r->x = 0.0f;
	r->y = 0.0f;
	r->z = 0.0f;
	return r;
}

ce_vec3* ce_vec3_unit_x(ce_vec3* r)
{
	r->x = 1.0f;
	r->y = 0.0f;
	r->z = 0.0f;
	return r;
}

ce_vec3* ce_vec3_unit_y(ce_vec3* r)
{
	r->x = 0.0f;
	r->y = 1.0f;
	r->z = 0.0f;
	return r;
}

ce_vec3* ce_vec3_unit_z(ce_vec3* r)
{
	r->x = 0.0f;
	r->y = 0.0f;
	r->z = 1.0f;
	return r;
}

ce_vec3* ce_vec3_unit_scale(ce_vec3* r)
{
	r->x = 1.0f;
	r->y = 1.0f;
	r->z = 1.0f;
	return r;
}

ce_vec3* ce_vec3_neg_unit_x(ce_vec3* r)
{
	r->x = -1.0f;
	r->y = 0.0f;
	r->z = 0.0f;
	return r;
}

ce_vec3* ce_vec3_neg_unit_y(ce_vec3* r)
{
	r->x = 0.0f;
	r->y = -1.0f;
	r->z = 0.0f;
	return r;
}

ce_vec3* ce_vec3_neg_unit_z(ce_vec3* r)
{
	r->x = 0.0f;
	r->y = 0.0f;
	r->z = -1.0f;
	return r;
}

ce_vec3* ce_vec3_neg_unit_scale(ce_vec3* r)
{
	r->x = -1.0f;
	r->y = -1.0f;
	r->z = -1.0f;
	return r;
}

ce_vec3* ce_vec3_init(ce_vec3* r, float x, float y, float z)
{
	r->x = x;
	r->y = y;
	r->z = z;
	return r;
}

ce_vec3* ce_vec3_init_array(ce_vec3* r, const float* v)
{
	r->x = *v++;
	r->y = *v++;
	r->z = *v++;
	return r;
}

ce_vec3* ce_vec3_copy(ce_vec3* r, const ce_vec3* a)
{
	r->x = a->x;
	r->y = a->y;
	r->z = a->z;
	return r;
}

ce_vec3* ce_vec3_neg(ce_vec3* r, const ce_vec3* a)
{
	r->x = -a->x;
	r->y = -a->y;
	r->z = -a->z;
	return r;
}

ce_vec3* ce_vec3_add(ce_vec3* r, const ce_vec3* a, const ce_vec3* b)
{
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
	return r;
}

ce_vec3* ce_vec3_sub(ce_vec3* r, const ce_vec3* a, const ce_vec3* b)
{
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
	return r;
}

ce_vec3* ce_vec3_mul(ce_vec3* r, const ce_vec3* a, const ce_vec3* b)
{
	r->x = a->x * b->x;
	r->y = a->y * b->y;
	r->z = a->z * b->z;
	return r;
}

ce_vec3* ce_vec3_div(ce_vec3* r, const ce_vec3* a, const ce_vec3* b)
{
	r->x = a->x / b->x;
	r->y = a->y / b->y;
	r->z = a->z / b->z;
	return r;
}

ce_vec3* ce_vec3_scale(ce_vec3* r, const ce_vec3* a, float s)
{
	r->x = a->x * s;
	r->y = a->y * s;
	r->z = a->z * s;
	return r;
}

float ce_vec3_abs(const ce_vec3* a)
{
	return sqrtf(ce_vec3_abs2(a));
}

float ce_vec3_abs2(const ce_vec3* a)
{
	return a->x * a->x + a->y * a->y + a->z * a->z;
}

float ce_vec3_dist(const ce_vec3* a, const ce_vec3* b)
{
	ce_vec3 c;
	return ce_vec3_abs(ce_vec3_sub(&c, a, b));
}

float ce_vec3_dist2(const ce_vec3* a, const ce_vec3* b)
{
	ce_vec3 c;
	return ce_vec3_abs2(ce_vec3_sub(&c, a, b));
}

ce_vec3* ce_vec3_normalise(ce_vec3* r, const ce_vec3* a)
{
	return ce_vec3_scale(r, a, 1.0f / ce_vec3_abs(a));
}

float ce_vec3_dot(const ce_vec3* a, const ce_vec3* b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

float ce_vec3_absdot(const ce_vec3* a, const ce_vec3* b)
{
	return fabsf(a->x * b->x) + fabsf(a->y * b->y) + fabsf(a->z * b->z);
}

ce_vec3* ce_vec3_cross(ce_vec3* restrict r, const ce_vec3* a, const ce_vec3* b)
{
	r->x = a->y * b->z - a->z * b->y;
	r->y = a->z * b->x - a->x * b->z;
	r->z = a->x * b->y - a->y * b->x;
	return r;
}

ce_vec3* ce_vec3_mid(ce_vec3* r, const ce_vec3* a, const ce_vec3* b)
{
	r->x = 0.5f * (a->x + b->x);
	r->y = 0.5f * (a->y + b->y);
	r->z = 0.5f * (a->z + b->z);
	return r;
}

ce_vec3* ce_vec3_rot(ce_vec3* r, const ce_vec3* a, const ce_quat* b)
{
	ce_vec3 qv, uv, uuv;
	ce_vec3_init(&qv, b->x, b->y, b->z);
	ce_vec3_cross(&uv, &qv, a);
	ce_vec3_cross(&uuv, &qv, &uv);
	ce_vec3_scale(&uv, &uv, 2.0f * b->w);
	ce_vec3_scale(&uuv, &uuv, 2.0f);
	return ce_vec3_add(r, ce_vec3_add(r, a, &uv), &uuv);
}

ce_vec3* ce_vec3_lerp(ce_vec3* r, const ce_vec3* a, const ce_vec3* b, float u)
{
	r->x = ce_lerp(u, a->x, b->x);
	r->y = ce_lerp(u, a->y, b->y);
	r->z = ce_lerp(u, a->z, b->z);
	return r;
}

ce_vec3* ce_vec3_floor(ce_vec3* r, const ce_vec3* a, const ce_vec3* b)
{
	r->x = fminf(a->x, b->x);
	r->y = fminf(a->y, b->y);
	r->z = fminf(a->z, b->z);
	return r;
}

ce_vec3* ce_vec3_ceil(ce_vec3* r, const ce_vec3* a, const ce_vec3* b)
{
	r->x = fmaxf(a->x, b->x);
	r->y = fmaxf(a->y, b->y);
	r->z = fmaxf(a->z, b->z);
	return r;
}
