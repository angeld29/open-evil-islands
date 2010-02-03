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
} ce_vec2;

extern const ce_vec2 CE_VEC2_ZERO;
extern const ce_vec2 CE_VEC2_UNIT_X;
extern const ce_vec2 CE_VEC2_UNIT_Y;
extern const ce_vec2 CE_VEC2_UNIT_SCALE;
extern const ce_vec2 CE_VEC2_NEG_UNIT_X;
extern const ce_vec2 CE_VEC2_NEG_UNIT_Y;
extern const ce_vec2 CE_VEC2_NEG_UNIT_SCALE;

extern ce_vec2* ce_vec2_zero(ce_vec2* r);
extern ce_vec2* ce_vec2_unit_x(ce_vec2* r);
extern ce_vec2* ce_vec2_unit_y(ce_vec2* r);
extern ce_vec2* ce_vec2_unit_scale(ce_vec2* r);
extern ce_vec2* ce_vec2_neg_unit_x(ce_vec2* r);
extern ce_vec2* ce_vec2_neg_unit_y(ce_vec2* r);
extern ce_vec2* ce_vec2_neg_unit_scale(ce_vec2* r);

extern ce_vec2* ce_vec2_init(ce_vec2* r, float x, float y);
extern ce_vec2* ce_vec2_copy(ce_vec2* r, const ce_vec2* a);

extern ce_vec2* ce_vec2_neg(ce_vec2* r, const ce_vec2* a);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VEC2_H */
