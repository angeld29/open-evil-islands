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

#include <cmath>
#include <algorithm>

#include "utility.hpp"

namespace cursedearth
{
    const float g_pi = 3.14159265f;
    const float g_pi2 = 6.28318531f;
    const float g_pi_div_2 = 1.57079633f;
    const float g_pi_div_4 = 0.78539816f;
    const float g_pi_inv = 0.31830989f;

    const float g_epsilon_e3 = 1e-3f;
    const float g_epsilon_e4 = 1e-4f;
    const float g_epsilon_e5 = 1e-5f;
    const float g_epsilon_e6 = 1e-6f;

    const double d_epsilon_e3 = 1e-3;

    const float g_deg2rad = 0.01745329f;
    const float g_rad2deg = 57.2957795f;

    // http://www.c-faq.com/fp/fpequal.html
    float relative_difference(float a, float b)
    {
        const float value = std::max(std::abs(a), std::abs(b));
        return 0.0f == value ? 0.0f : std::abs(a - b) / value;
    }
    
    double relative_difference(double a, double b)
    {
        const double value = std::max(std::abs(a), std::abs(b));
        return 0.0 == value ? 0.0 : std::abs(a - b) / value;
    }
}
