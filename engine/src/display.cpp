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

#include <cassert>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <iostream>
#include <algorithm>

#include "alloc.hpp"
#include "logging.hpp"
#include "display.hpp"

namespace cursedearth
{
    display_rotation_t display_rotation_from_degrees(int value)
    {
        unsigned int rotation = DISPLAY_ROTATION_NONE;
        if (value < 90) {
            rotation |= DISPLAY_ROTATION_0;
        } else if (value < 180) {
            rotation |= DISPLAY_ROTATION_90;
        } else if (value < 270) {
            rotation |= DISPLAY_ROTATION_180;
        } else if (value < 360) {
            rotation |= DISPLAY_ROTATION_270;
        }
        return static_cast<display_rotation_t>(rotation);
    }

    display_reflection_t display_reflection_from_bool(bool x, bool y)
    {
        unsigned int reflection = DISPLAY_REFLECTION_NONE;
        if (x) reflection |= DISPLAY_REFLECTION_X;
        if (y) reflection |= DISPLAY_REFLECTION_Y;
        return static_cast<display_reflection_t>(reflection);
    }

    size_t display_manager_t::enter(int width, int height, int bpp, int rate, display_rotation_t rotation, display_reflection_t reflection)
    {
        if (m_modes.empty()) {
            throw std::runtime_error("display manager: no display modes found");
        }

        int best_width = width, best_height = height;
        int best_bpp = bpp, best_rate = rate;

        for (const auto& mode: m_modes) {
            if (width <= 0) best_width = std::max(best_width, mode.width);
            if (height <= 0) best_height = std::max(best_height, mode.height);
            if (bpp <= 0) best_bpp = std::max(best_bpp, mode.bpp);
            if (rate <= 0) best_rate = std::max(best_rate, mode.rate);
        }

        width = best_width, height = best_height;
        bpp = best_bpp, rate = best_rate;

        int best_size_score = std::numeric_limits<int>::max();
        int best_bpp_score = std::numeric_limits<int>::max();
        int best_rate_score = std::numeric_limits<int>::max();

        // pass 1: find best width and height
        for (const auto& mode: m_modes) {
            int score = (width - mode.width) * (width - mode.width) + (height - mode.height) * (height - mode.height);
            if (score < best_size_score) {
                best_width = mode.width;
                best_height = mode.height;
                best_size_score = score;
            }
        }

        // pass 2: find best bpp and rate
        for (const auto& mode: m_modes) {
            if (best_width == mode.width && best_height == mode.height) {
                int score = std::abs(bpp - mode.bpp);
                if (score < best_bpp_score) {
                    best_bpp = mode.bpp;
                    best_bpp_score = score;
                }
                score = std::abs(rate - mode.rate);
                if (score < best_rate_score) {
                    best_rate = mode.rate;
                    best_rate_score = score;
                }
            }
        }

        if (width != best_width || height != best_height || bpp != best_bpp || rate != best_rate) {
            ce_logging_warning("displaymng: mode %dx%d:%d@%d not found", width, height, bpp, rate);
            ce_logging_write("displaymng: best match %dx%d:%d@%d used", best_width, best_height, best_bpp, best_rate);
        }

        // pass 3: find index
        size_t index;
        for (index = 0; index < m_modes.size(); ++index) {
            if (best_width == m_modes[index].width && best_height == m_modes[index].height &&
                    best_bpp == m_modes[index].bpp && best_rate == m_modes[index].rate) {
                break;
            }
        }

        if (m_modes.size() == index) {
            assert(false);
            index = 0;
        }

        display_rotation_t best_rotation = DISPLAY_ROTATION_NONE;
        if (rotation & DISPLAY_ROTATION_0 && m_rotation & DISPLAY_ROTATION_0) {
            best_rotation = DISPLAY_ROTATION_0;
        } else if (rotation & DISPLAY_ROTATION_90 && m_rotation & DISPLAY_ROTATION_90) {
            best_rotation = DISPLAY_ROTATION_90;
        } else if (rotation & DISPLAY_ROTATION_180 && m_rotation & DISPLAY_ROTATION_180) {
            best_rotation = DISPLAY_ROTATION_180;
        } else if (rotation & DISPLAY_ROTATION_270 && m_rotation & DISPLAY_ROTATION_270) {
            best_rotation = DISPLAY_ROTATION_270;
        }

        display_reflection_t best_reflection = display_reflection_from_bool(reflection & DISPLAY_REFLECTION_X && m_reflection & DISPLAY_REFLECTION_X,
                                                                            reflection & DISPLAY_REFLECTION_Y && m_reflection & DISPLAY_REFLECTION_Y);

        do_enter(index, best_rotation, best_reflection);

        return index;
    }

    void display_manager_t::dump_supported_modes_to_stdout() const
    {
        for (const auto& mode: m_modes) {
            std::cout << boost::format("%1%x%2%:%3%@%4%") % mode.width % mode.height % mode.bpp % mode.rate << std::endl;
        }
    }

    void display_manager_t::dump_supported_rotations_to_stdout() const
    {
        int rotations[4] = { 0, 90, 180, 270 };
        for (unsigned int i = 0, j = DISPLAY_ROTATION_0; i < 4; ++i, j <<= 1) {
            std::cout << rotations[i]  << ": " << (j & m_rotation ? "yes" : "no") << std::endl;
        }
    }

    void display_manager_t::dump_supported_reflections_to_stdout() const
    {
        std::cout << "x: " << (DISPLAY_REFLECTION_X & m_reflection ? "yes" : "no") << std::endl;
        std::cout << "y: " << (DISPLAY_REFLECTION_Y & m_reflection ? "yes" : "no") << std::endl;
    }
}
