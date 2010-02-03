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

#include "cevec3.h"
#include "quat.h"

const quat QUAT_ZERO = { .w = 0.0f, .x = 0.0f, .y = 0.0f, .z = 0.0f };
const quat QUAT_IDENTITY = { .w = 1.0f, .x = 0.0f, .y = 0.0f, .z = 0.0f };

quat* quat_zero(quat* r)
{
	r->w = 0.0f;
	r->x = 0.0f;
	r->y = 0.0f;
	r->z = 0.0f;
	return r;
}

quat* quat_identity(quat* r)
{
	r->w = 1.0f;
	r->x = 0.0f;
	r->y = 0.0f;
	r->z = 0.0f;
	return r;
}

quat* quat_init(float w, float x, float y, float z, quat* r)
{
	r->w = w;
	r->x = x;
	r->y = y;
	r->z = z;
	return r;
}

quat* quat_init_polar(float theta, const ce_vec3* axis, quat* r)
{
	float t = 0.5f * theta;
	float s = sinf(t);
	r->w = cosf(t);
	r->x = s * axis->x;
	r->y = s * axis->y;
	r->z = s * axis->z;
	return r;
}

quat* quat_copy(const quat* a, quat* r)
{
	r->w = a->w;
	r->x = a->x;
	r->y = a->y;
	r->z = a->z;
	return r;
}

quat* quat_neg(const quat* a, quat* r)
{
	r->w = -a->w;
	r->x = -a->x;
	r->y = -a->y;
	r->z = -a->z;
	return r;
}

quat* quat_conj(const quat* a, quat* r)
{
	r->w = a->w;
	r->x = -a->x;
	r->y = -a->y;
	r->z = -a->z;
	return r;
}

quat* quat_add(const quat* a, const quat* b, quat* r)
{
	r->w = a->w + b->w;
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
	return r;
}

quat* quat_sub(const quat* a, const quat* b, quat* r)
{
	r->w = a->w - b->w;
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
	return r;
}

quat* quat_mul(const quat* a, const quat* b, quat* restrict r)
{
	r->w = a->w * b->w - a->x * b->x - a->y * b->y - a->z * b->z;
	r->x = a->w * b->x + a->x * b->w + a->y * b->z - a->z * b->y;
	r->y = a->w * b->y - a->x * b->z + a->y * b->w + a->z * b->x;
	r->z = a->w * b->z + a->x * b->y - a->y * b->x + a->z * b->w;
	return r;
}

float quat_abs(const quat* a)
{
	return sqrtf(quat_abs2(a));
}

float quat_abs2(const quat* a)
{
	return a->w * a->w + a->x * a->x + a->y * a->y + a->z * a->z;
}

float quat_arg(const quat* a)
{
	float s = quat_abs(a);
	return 0.0f == s ? 0.0f : acosf(a->w / s);
}

quat* quat_normalise(const quat* a, quat* r)
{
	float s = 1.0f / quat_abs(a);
	r->w = a->w * s;
	r->x = a->x * s;
	r->y = a->y * s;
	r->z = a->z * s;
	return r;
}

quat* quat_inverse(const quat* a, quat* r)
{
	float s = 1.0f / quat_abs2(a);
	r->w = a->w * s;
	r->x = a->x * -s;
	r->y = a->y * -s;
	r->z = a->z * -s;
	return r;
}

float quat_dot(const quat* a, const quat* b)
{
	return a->w * b->w + a->x * b->x + a->y * b->y + a->z * b->z;
}
