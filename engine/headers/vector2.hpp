/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#ifndef CE_VECTOR2_HPP
#define CE_VECTOR2_HPP

namespace cursedearth
{
    struct vector2_t
    {
        float x, y;
    };

    extern const vector2_t CE_VEC2_ZERO;
    extern const vector2_t CE_VEC2_UNIT_X;
    extern const vector2_t CE_VEC2_UNIT_Y;
    extern const vector2_t CE_VEC2_UNIT_SCALE;
    extern const vector2_t CE_VEC2_NEG_UNIT_X;
    extern const vector2_t CE_VEC2_NEG_UNIT_Y;
    extern const vector2_t CE_VEC2_NEG_UNIT_SCALE;

    inline vector2_t* ce_vec2_init(vector2_t* vec, float x, float y)
    {
        vec->x = x;
        vec->y = y;
        return vec;
    }

    inline vector2_t* ce_vec2_init_scalar(vector2_t* vec, float s)
    {
        vec->x = s;
        vec->y = s;
        return vec;
    }

    inline vector2_t* ce_vec2_init_array(vector2_t* vec, const float* array)
    {
        vec->x = array[0];
        vec->y = array[1];
        return vec;
    }

    inline vector2_t* ce_vec2_init_zero(vector2_t* vec)
    {
        vec->x = 0.0f;
        vec->y = 0.0f;
        return vec;
    }

    inline vector2_t* ce_vec2_init_unit_x(vector2_t* vec)
    {
        vec->x = 1.0f;
        vec->y = 0.0f;
        return vec;
    }

    inline vector2_t* ce_vec2_init_unit_y(vector2_t* vec)
    {
        vec->x = 0.0f;
        vec->y = 1.0f;
        return vec;
    }

    inline vector2_t* ce_vec2_init_unit_scale(vector2_t* vec)
    {
        vec->x = 1.0f;
        vec->y = 1.0f;
        return vec;
    }

    inline vector2_t* ce_vec2_init_neg_unit_x(vector2_t* vec)
    {
        vec->x = -1.0f;
        vec->y = 0.0f;
        return vec;
    }

    inline vector2_t* ce_vec2_init_neg_unit_y(vector2_t* vec)
    {
        vec->x = 0.0f;
        vec->y = -1.0f;
        return vec;
    }

    inline vector2_t* ce_vec2_init_neg_unit_scale(vector2_t* vec)
    {
        vec->x = -1.0f;
        vec->y = -1.0f;
        return vec;
    }

    inline vector2_t* ce_vec2_copy(vector2_t* vec, const vector2_t* other)
    {
        vec->x = other->x;
        vec->y = other->y;
        return vec;
    }

    inline vector2_t* ce_vec2_neg(vector2_t* vec, const vector2_t* other)
    {
        vec->x = -other->x;
        vec->y = -other->y;
        return vec;
    }
}

#endif
