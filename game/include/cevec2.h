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

#ifndef CE_VEC2_H
#define CE_VEC2_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	float x, y;
} vec2;

extern const vec2 VEC2_ZERO;
extern const vec2 VEC2_UNIT_X;
extern const vec2 VEC2_UNIT_Y;
extern const vec2 VEC2_UNIT_SCALE;
extern const vec2 VEC2_NEG_UNIT_X;
extern const vec2 VEC2_NEG_UNIT_Y;
extern const vec2 VEC2_NEG_UNIT_SCALE;

extern vec2* vec2_zero(vec2* r);
extern vec2* vec2_unit_x(vec2* r);
extern vec2* vec2_unit_y(vec2* r);
extern vec2* vec2_unit_scale(vec2* r);
extern vec2* vec2_neg_unit_x(vec2* r);
extern vec2* vec2_neg_unit_y(vec2* r);
extern vec2* vec2_neg_unit_scale(vec2* r);

extern vec2* vec2_init(float x, float y, vec2* r);
extern vec2* vec2_copy(const vec2* a, vec2* r);

extern vec2* vec2_neg(const vec2* a, vec2* r);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VEC2_H */
