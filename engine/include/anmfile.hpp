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

/**
 * doc/formats/anm.txt
 */

#ifndef CE_ANMFILE_HPP
#define CE_ANMFILE_HPP

#include <cstddef>

#include "string.hpp"
#include "resfile.hpp"

namespace cursedearth
{
    typedef struct {
        ce_string* name;
        int rotation_frame_count;
        int translation_frame_count;
        int morph_frame_count;
        int morph_vertex_count;
        float* rotations;
        float* translations;
        float* morphs;
        size_t size;
        void* data;
    } ce_anmfile;

    ce_anmfile* ce_anmfile_open(ce_res_file* res_file, size_t index);
    void ce_anmfile_close(ce_anmfile* anmfile);
}

#endif
