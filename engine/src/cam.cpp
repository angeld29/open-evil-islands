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

#include "alloc.hpp"
#include "logging.hpp"
#include "cam.hpp"

namespace cursedearth
{
ce_cam_file* ce_cam_file_new(memory_file_t* mem_file)
{
    size_t record_count = ce_mem_file_size(mem_file) / 36;

    ce_cam_file* cam_file = ce_alloc_zero(sizeof(ce_cam_file) + sizeof(ce_cam_record) * record_count);
    cam_file->record_count = record_count;

    for (size_t i = 0; i < record_count; ++i) {
        cam_file->records[i].time = ce_mem_file_read_u32le(mem_file);
        cam_file->records[i].unknown = ce_mem_file_read_u32le(mem_file);
        ce_mem_file_read(mem_file, cam_file->records[i].position, sizeof(float), 3);
        ce_mem_file_read(mem_file, cam_file->records[i].rotation, sizeof(float), 4);
    }

    return cam_file;
}

void ce_cam_file_del(ce_cam_file* cam_file)
{
    if (NULL != cam_file) {
        ce_free(cam_file, sizeof(ce_cam_file) + sizeof(ce_cam_record) * cam_file->record_count);
    }
}
}
