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

#ifndef CE_ANMSTATE_HPP
#define CE_ANMSTATE_HPP

#include "vector.hpp"
#include "anmfile.hpp"

namespace cursedearth
{
    typedef struct {
        ce_anmfile* anmfile;
        float frame_count;
        float prev_frame;
        float next_frame;
        float frame;
        float coef;
    } ce_anmstate;

    extern ce_anmstate* ce_anmstate_new(void);
    extern void ce_anmstate_del(ce_anmstate* anmstate);

    extern void ce_anmstate_advance(ce_anmstate* anmstate, float distance);

    extern bool ce_anmstate_play_animation(ce_anmstate* anmstate, ce_vector* anmfiles, const char* name);
    extern void ce_anmstate_stop_animation(ce_anmstate* anmstate);
}

#endif
