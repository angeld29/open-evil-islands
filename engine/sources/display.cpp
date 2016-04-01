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
#include <cstdio>
#include <climits>
#include <algorithm>

#include "alloc.hpp"
#include "logging.hpp"
#include "display.hpp"

namespace cursedearth
{
    ce_display_rotation ce_display_rotation_from_degrees(int value)
    {
        unsigned int rotation = CE_DISPLAY_ROTATION_NONE;
        if (value < 90) {
            rotation |= CE_DISPLAY_ROTATION_0;
        } else if (value < 180) {
            rotation |= CE_DISPLAY_ROTATION_90;
        } else if (value < 270) {
            rotation |= CE_DISPLAY_ROTATION_180;
        } else if (value < 360) {
            rotation |= CE_DISPLAY_ROTATION_270;
        }
        return static_cast<ce_display_rotation>(rotation);
    }

    ce_display_reflection ce_display_reflection_from_bool(bool x, bool y)
    {
        unsigned int reflection = CE_DISPLAY_REFLECTION_NONE;
        if (x) reflection |= CE_DISPLAY_REFLECTION_X;
        if (y) reflection |= CE_DISPLAY_REFLECTION_Y;
        return static_cast<ce_display_reflection>(reflection);
    }

    ce_displaymode* ce_displaymode_new(int width, int height, int bpp, int rate)
    {
        ce_displaymode* mode = (ce_displaymode*)ce_alloc(sizeof(ce_displaymode));
        mode->width = width;
        mode->height = height;
        mode->bpp = bpp;
        mode->rate = rate;
        return mode;
    }

    void ce_displaymode_del(ce_displaymode* mode)
    {
        ce_free(mode, sizeof(ce_displaymode));
    }

    ce_displaymng* ce_displaymng_new(ce_displaymng_vtable vtable, size_t size, ...)
    {
        ce_displaymng* displaymng = (ce_displaymng*)ce_alloc(sizeof(ce_displaymng));
        displaymng->impl = ce_alloc(size);
        displaymng->supported_modes = ce_vector_new();
        displaymng->supported_rotation = CE_DISPLAY_ROTATION_NONE;
        displaymng->supported_reflection = CE_DISPLAY_REFLECTION_NONE;
        displaymng->vtable = vtable;
        displaymng->size = size;
        va_list args;
        va_start(args, size);
        (*vtable.ctor)(displaymng, args);
        va_end(args);
        return displaymng;
    }

    void ce_displaymng_del(ce_displaymng* displaymng)
    {
        if (NULL != displaymng) {
            (*displaymng->vtable.dtor)(displaymng);
            ce_vector_for_each(displaymng->supported_modes, (void(*)(void*))ce_displaymode_del);
            ce_vector_del(displaymng->supported_modes);
            ce_free(displaymng->impl, displaymng->size);
            ce_free(displaymng, sizeof(ce_displaymng));
        }
    }

    void ce_displaymng_dump_supported_modes_to_stdout(ce_displaymng* displaymng)
    {
        for (size_t i = 0; i < displaymng->supported_modes->count; ++i) {
            ce_displaymode* mode = (ce_displaymode*)displaymng->supported_modes->items[i];
            fprintf(stdout, "%dx%d:%d@%d\n", mode->width, mode->height, mode->bpp, mode->rate);
        }
    }

    void ce_displaymng_dump_supported_rotations_to_stdout(ce_displaymng* displaymng)
    {
        int angle[4] = { 0, 90, 180, 270 };
        for (unsigned int i = 0, j = CE_DISPLAY_ROTATION_0; i < 4; ++i, j <<= 1) {
            fprintf(stdout, "%d: %s\n", angle[i], j & displaymng->supported_rotation ? "yes" : "no");
        }
    }

    void ce_displaymng_dump_supported_reflections_to_stdout(ce_displaymng* displaymng)
    {
        fprintf(stdout, "x: %s\ny: %s\n",
            CE_DISPLAY_REFLECTION_X & displaymng->supported_reflection ? "yes" : "no",
            CE_DISPLAY_REFLECTION_Y & displaymng->supported_reflection ? "yes" : "no");
    }

    size_t ce_displaymng_enter(ce_displaymng* displaymng, int width, int height, int bpp, int rate, ce_display_rotation rotation, ce_display_reflection reflection)
    {
        if (ce_vector_empty(displaymng->supported_modes)) {
            ce_logging_warning("displaymng: no display modes found");
            return -1;
        }

        int best_width = width, best_height = height;
        int best_bpp = bpp, best_rate = rate;

        for (size_t i = 0; i < displaymng->supported_modes->count; ++i) {
            ce_displaymode* mode = (ce_displaymode*)displaymng->supported_modes->items[i];
            if (width <= 0) best_width = std::max(best_width, mode->width);
            if (height <= 0) best_height = std::max(best_height, mode->height);
            if (bpp <= 0) best_bpp = std::max(best_bpp, mode->bpp);
            if (rate <= 0) best_rate = std::max(best_rate, mode->rate);
        }

        width = best_width, height = best_height;
        bpp = best_bpp, rate = best_rate;

        int best_size_score = INT_MAX;
        int best_bpp_score = INT_MAX, best_rate_score = INT_MAX;

        // pass 1: find best width and height
        for (size_t i = 0; i < displaymng->supported_modes->count; ++i) {
            ce_displaymode* mode = (ce_displaymode*)displaymng->supported_modes->items[i];
            int score = (width - mode->width) * (width - mode->width) + (height - mode->height) * (height - mode->height);
            if (score < best_size_score) {
                best_width = mode->width;
                best_height = mode->height;
                best_size_score = score;
            }
        }

        // pass 2: find best bpp and rate
        for (size_t i = 0; i < displaymng->supported_modes->count; ++i) {
            ce_displaymode* mode = (ce_displaymode*)displaymng->supported_modes->items[i];
            if (best_width == mode->width && best_height == mode->height) {
                int score = abs(bpp - mode->bpp);
                if (score < best_bpp_score) {
                    best_bpp = mode->bpp;
                    best_bpp_score = score;
                }
                score = abs(rate - mode->rate);
                if (score < best_rate_score) {
                    best_rate = mode->rate;
                    best_rate_score = score;
                }
            }
        }

        if (width != best_width || height != best_height || bpp != best_bpp || rate != best_rate) {
            ce_logging_warning("displaymng: mode %dx%d:%d@%d not found", width, height, bpp, rate);
            ce_logging_info("displaymng: using best match %dx%d:%d@%d", best_width, best_height, best_bpp, best_rate);
        }

        // pass 3: find index
        size_t index;
        for (index = 0; index < displaymng->supported_modes->count; ++index) {
            ce_displaymode* mode = (ce_displaymode*)displaymng->supported_modes->items[index];
            if (best_width == mode->width && best_height == mode->height && best_bpp == mode->bpp && best_rate == mode->rate) {
                break;
            }
        }

        if (index == displaymng->supported_modes->count) {
            assert(false);
            index = 0;
        }

        // correct rotation and reflection
        ce_display_rotation best_rotation = CE_DISPLAY_ROTATION_NONE;
        ce_display_reflection best_reflection = CE_DISPLAY_REFLECTION_NONE;

        // exclusive mode!
        if (rotation & CE_DISPLAY_ROTATION_0 && displaymng->supported_rotation & CE_DISPLAY_ROTATION_0) {
            best_rotation = CE_DISPLAY_ROTATION_0;
        } else if (rotation & CE_DISPLAY_ROTATION_90 && displaymng->supported_rotation & CE_DISPLAY_ROTATION_90) {
            best_rotation = CE_DISPLAY_ROTATION_90;
        } else if (rotation & CE_DISPLAY_ROTATION_180 && displaymng->supported_rotation & CE_DISPLAY_ROTATION_180) {
            best_rotation = CE_DISPLAY_ROTATION_180;
        } else if (rotation & CE_DISPLAY_ROTATION_270 && displaymng->supported_rotation & CE_DISPLAY_ROTATION_270) {
            best_rotation = CE_DISPLAY_ROTATION_270;
        }

        best_reflection = ce_display_reflection_from_bool(reflection & CE_DISPLAY_REFLECTION_X && displaymng->supported_reflection & CE_DISPLAY_REFLECTION_X,
                                                          reflection & CE_DISPLAY_REFLECTION_Y && displaymng->supported_reflection & CE_DISPLAY_REFLECTION_Y);

        (*displaymng->vtable.enter)(displaymng, index, best_rotation, best_reflection);

        return index;
    }

    void ce_displaymng_exit(ce_displaymng* displaymng)
    {
        (*displaymng->vtable.exit)(displaymng);
    }
}
