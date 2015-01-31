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

#ifndef CE_DISPLAY_HPP
#define CE_DISPLAY_HPP

#include <cstddef>
#include <cstdarg>

#include "vector.hpp"

namespace cursedearth
{
    typedef enum {
        CE_DISPLAY_ROTATION_NONE,
        CE_DISPLAY_ROTATION_0 = 1,
        CE_DISPLAY_ROTATION_90 = 2,
        CE_DISPLAY_ROTATION_180 = 4,
        CE_DISPLAY_ROTATION_270 = 8
    } ce_display_rotation;

    typedef enum {
        CE_DISPLAY_REFLECTION_NONE,
        CE_DISPLAY_REFLECTION_X = 1,
        CE_DISPLAY_REFLECTION_Y = 2
    } ce_display_reflection;

    extern ce_display_rotation ce_display_rotation_from_degrees(int value);
    extern ce_display_reflection ce_display_reflection_from_bool(bool x, bool y);

    typedef struct {
        int width, height, bpp, rate;
    } ce_displaymode;

    extern ce_displaymode* ce_displaymode_new(int width, int height, int bpp, int rate);
    extern void ce_displaymode_del(ce_displaymode* mode);

    typedef struct ce_displaymng ce_displaymng;

    typedef struct {
        void (*ctor)(ce_displaymng* displaymng, va_list args);
        void (*dtor)(ce_displaymng* displaymng);
        void (*enter)(ce_displaymng* displaymng, size_t index,
            ce_display_rotation rotation, ce_display_reflection reflection);
        void (*exit)(ce_displaymng* displaymng);
    } ce_displaymng_vtable;

    struct ce_displaymng {
        ce_vector* supported_modes;
        ce_display_rotation supported_rotation;
        ce_display_reflection supported_reflection;
        ce_displaymng_vtable vtable;
        size_t size;
        char impl[];
    };

    extern ce_displaymng* ce_displaymng_new(ce_displaymng_vtable vtable, size_t size, ...);
    extern void ce_displaymng_del(ce_displaymng* displaymng);

    extern void ce_displaymng_dump_supported_modes_to_stdout(ce_displaymng* displaymng);
    extern void ce_displaymng_dump_supported_rotations_to_stdout(ce_displaymng* displaymng);
    extern void ce_displaymng_dump_supported_reflections_to_stdout(ce_displaymng* displaymng);

    extern size_t ce_displaymng_enter(ce_displaymng* displaymng, int width, int height, int bpp, int rate, ce_display_rotation rotation, ce_display_reflection reflection);
    extern void ce_displaymng_exit(ce_displaymng* displaymng);
}

#endif /* CE_DISPLAY_HPP */
