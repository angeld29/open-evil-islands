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
#include "cequat.h"

const ce_quat CE_QUAT_ZERO = { .w = 0.0f, .x = 0.0f, .y = 0.0f, .z = 0.0f };
const ce_quat CE_QUAT_IDENTITY = { .w = 1.0f, .x = 0.0f, .y = 0.0f, .z = 0.0f };

ce_quat* ce_quat_zero(ce_quat* r)
{
	r->w = 0.0f;
	r->x = 0.0f;
	r->y = 0.0f;
	r->z = 0.0f;
	return r;
}

ce_quat* ce_quat_identity(ce_quat* r)
{
	r->w = 1.0f;
	r->x = 0.0f;
	r->y = 0.0f;
	r->z = 0.0f;
	return r;
}

ce_quat* ce_quat_init(ce_quat* r, float w, float x, float y, float z)
{
	r->w = w;
	r->x = x;
	r->y = y;
	r->z = z;
	return r;
}

ce_quat* ce_quat_init_array(ce_quat* r, float* v)
{
	r->w = *v++;
	r->x = *v++;
	r->y = *v++;
	r->z = *v++;
	return r;
}

ce_quat* ce_quat_init_polar(ce_quat* r, float theta, const ce_vec3* axis)
{
	float t = 0.5f * theta;
	float s = sinf(t);
	r->w = cosf(t);
	r->x = s * axis->x;
	r->y = s * axis->y;
	r->z = s * axis->z;
	return r;
}

ce_quat* ce_quat_copy(ce_quat* r, const ce_quat* a)
{
	r->w = a->w;
	r->x = a->x;
	r->y = a->y;
	r->z = a->z;
	return r;
}

ce_quat* ce_quat_neg(ce_quat* r, const ce_quat* a)
{
	r->w = -a->w;
	r->x = -a->x;
	r->y = -a->y;
	r->z = -a->z;
	return r;
}

ce_quat* ce_quat_conj(ce_quat* r, const ce_quat* a)
{
	r->w = a->w;
	r->x = -a->x;
	r->y = -a->y;
	r->z = -a->z;
	return r;
}

ce_quat* ce_quat_add(ce_quat* r, const ce_quat* a, const ce_quat* b)
{
	r->w = a->w + b->w;
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
	return r;
}

ce_quat* ce_quat_sub(ce_quat* r, const ce_quat* a, const ce_quat* b)
{
	r->w = a->w - b->w;
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
	return r;
}

ce_quat* ce_quat_mul(ce_quat* restrict r, const ce_quat* a, const ce_quat* b)
{
	r->w = a->w * b->w - a->x * b->x - a->y * b->y - a->z * b->z;
	r->x = a->w * b->x + a->x * b->w + a->y * b->z - a->z * b->y;
	r->y = a->w * b->y - a->x * b->z + a->y * b->w + a->z * b->x;
	r->z = a->w * b->z + a->x * b->y - a->y * b->x + a->z * b->w;
	return r;
}

float ce_quat_abs(const ce_quat* a)
{
	return sqrtf(ce_quat_abs2(a));
}

float ce_quat_abs2(const ce_quat* a)
{
	return a->w * a->w + a->x * a->x + a->y * a->y + a->z * a->z;
}

float ce_quat_arg(const ce_quat* a)
{
	float s = ce_quat_abs(a);
	return 0.0f == s ? 0.0f : acosf(a->w / s);
}

ce_quat* ce_quat_normalise(ce_quat* r, const ce_quat* a)
{
	float s = 1.0f / ce_quat_abs(a);
	r->w = a->w * s;
	r->x = a->x * s;
	r->y = a->y * s;
	r->z = a->z * s;
	return r;
}

ce_quat* ce_quat_inverse(ce_quat* r, const ce_quat* a)
{
	float s = 1.0f / ce_quat_abs2(a);
	r->w = a->w * s;
	r->x = a->x * -s;
	r->y = a->y * -s;
	r->z = a->z * -s;
	return r;
}

float ce_quat_dot(const ce_quat* a, const ce_quat* b)
{
	return a->w * b->w + a->x * b->x + a->y * b->y + a->z * b->z;
}
