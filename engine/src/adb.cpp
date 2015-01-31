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

#include "lib.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "byteorder.hpp"
#include "adbfile.hpp"

static const uint32_t CE_ADB_SIGNATURE = 0x00424441;

ce_adb_file* ce_adb_file_new(ce_mem_file* mem_file)
{
    uint32_t CE_UNUSED(signature) = ce_mem_file_read_u32le(mem_file);
    uint32_t record_count = ce_mem_file_read_u32le(mem_file);

    assert(CE_ADB_SIGNATURE == signature && "wrong signature");

    ce_adb_file* adb_file = ce_alloc_zero(sizeof(ce_adb_file) +
                                        sizeof(ce_adb_record) * record_count);

    adb_file->record_count = record_count;
    ce_mem_file_read(mem_file, adb_file->name, 1, 24);
    adb_file->min_height = ce_mem_file_read_fle(mem_file);
    adb_file->average_height = ce_mem_file_read_fle(mem_file);
    adb_file->max_height = ce_mem_file_read_fle(mem_file);

    for (size_t i = 0; i < record_count; ++i) {
        ce_mem_file_read(mem_file, adb_file->records[i].name, 1, 16);
        adb_file->records[i].id = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown1 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown2 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown3 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown4 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown5 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown6 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown7 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown8 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown9 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown10 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown11 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown12 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown13 = ce_mem_file_read_fle(mem_file);
        adb_file->records[i].unknown14 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown15 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown16 = ce_mem_file_read_u32le(mem_file);
        adb_file->records[i].unknown17 = ce_mem_file_read_u32le(mem_file);
    }

    return adb_file;
}

void ce_adb_file_del(ce_adb_file* adb_file)
{
    if (NULL != adb_file) {
        ce_free(adb_file, sizeof(ce_adb_file) +
                            sizeof(ce_adb_record) * adb_file->record_count);
    }
}
