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

#ifndef CE_UTILITY_HPP
#define CE_UTILITY_HPP

#include "standardheaders.hpp"

namespace cursedearth
{
    extern const float g_pi;
    extern const float g_pi2;
    extern const float g_pi_div_2;
    extern const float g_pi_div_4;
    extern const float g_pi_inv;

    extern const float g_epsilon_e3;
    extern const float g_epsilon_e4;
    extern const float g_epsilon_e5;
    extern const float g_epsilon_e6;

    extern const float g_deg2rad;
    extern const float g_rad2deg;

    float relative_difference(float a, float b);

    template <typename T, typename U>
    inline T clamp(T value, U min, U max)
    {
        return value < min ? min : (value > max ? max : value);
    }

    inline float lerp(float u, float a, float b)
    {
        return a + u * (b - a);
    }

    inline bool fisequal(float a, float b, float tolerance = g_epsilon_e3)
    {
        return relative_difference(a, b) <= tolerance;
    }

    inline bool fiszero(float a, float tolerance = g_epsilon_e3)
    {
        return fisequal(a, 0.0f, tolerance);
    }

    inline float deg2rad(float angle)
    {
        return  angle * g_deg2rad;
    }

    inline float rad2deg(float angle)
    {
        return angle * g_rad2deg;
    }

    inline bool is_power_of_two(size_t value)
    {
        return 0 == (value & (value - 1));
    }

    inline uint64_t next_largest_power_of_two(uint64_t value)
    {
        value |= (value >> 1);
        value |= (value >> 2);
        value |= (value >> 4);
        value |= (value >> 8);
        value |= (value >> 16);
        value |= (value >> 32);
        return value + 1;
    }
}

#endif
