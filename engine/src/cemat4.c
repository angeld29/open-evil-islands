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

#include "cemat4.h"

const ce_mat4 CE_MAT4_ZERO = {
	.m = {
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
	}
};

const ce_mat4 CE_MAT4_IDENTITY = {
	.m = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	}
};

ce_mat4* ce_mat4_init_zero(ce_mat4* mat)
{
	mat->m[0]  = 0.0f; mat->m[1]  = 0.0f; mat->m[2]  = 0.0f; mat->m[3]  = 0.0f;
	mat->m[4]  = 0.0f; mat->m[5]  = 0.0f; mat->m[6]  = 0.0f; mat->m[7]  = 0.0f;
	mat->m[8]  = 0.0f; mat->m[9]  = 0.0f; mat->m[10] = 0.0f; mat->m[11] = 0.0f;
	mat->m[12] = 0.0f; mat->m[13] = 0.0f; mat->m[14] = 0.0f; mat->m[15] = 0.0f;
	return mat;
}

ce_mat4* ce_mat4_init_identity(ce_mat4* mat)
{
	mat->m[0]  = 1.0f; mat->m[1]  = 0.0f; mat->m[2]  = 0.0f; mat->m[3]  = 0.0f;
	mat->m[4]  = 0.0f; mat->m[5]  = 1.0f; mat->m[6]  = 0.0f; mat->m[7]  = 0.0f;
	mat->m[8]  = 0.0f; mat->m[9]  = 0.0f; mat->m[10] = 1.0f; mat->m[11] = 0.0f;
	mat->m[12] = 0.0f; mat->m[13] = 0.0f; mat->m[14] = 0.0f; mat->m[15] = 1.0f;
	return mat;
}
