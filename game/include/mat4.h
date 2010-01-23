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

#ifndef CE_MAT4_H
#define CE_MAT4_H

#include "mat4def.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern const mat4 MAT4_ZERO;
extern const mat4 MAT4_IDENTITY;

extern mat4* mat4_zero(mat4* r);
extern mat4* mat4_identity(mat4* r);

extern mat4* mat4_mul(const mat4* a, const mat4* b, mat4* restrict r);
extern mat4* mat4_inverse(const mat4* a, mat4* r);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MAT4_H */
