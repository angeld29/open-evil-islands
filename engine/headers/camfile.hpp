/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

/*
 * doc/formats/cam.txt
 */

#ifndef CE_CAMFILE_HPP
#define CE_CAMFILE_HPP

#include <cstddef>
#include <cstdint>

#include "memfile.hpp"

namespace cursedearth
{
    typedef struct {
        uint32_t time;
        uint32_t unknown;
        float position[3];
        float rotation[4];
    } ce_cam_record;

    typedef struct {
        size_t record_count;
        ce_cam_record* records;
    } ce_cam_file;

    ce_cam_file* ce_cam_file_new(ce_mem_file* mem_file);
    void ce_cam_file_del(ce_cam_file* cam_file);
}

#endif
