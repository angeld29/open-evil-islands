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
#include "adb.hpp"

namespace
{
    const uint32_t CE_ADB_SIGNATURE = 0x00424441;
}

namespace cursedearth
{
    adb_ptr_t make_adb(const memory_file_ptr_t& memory_file)
    {
        adb_ptr_t adb = std::make_shared<adb_t>();

        uint32_t CE_UNUSED(signature) = read_u32le(memory_file);
        uint32_t record_count = read_u32le(memory_file);

        assert(CE_ADB_SIGNATURE == signature && "wrong signature");

        adb->record_count = record_count;
        memory_file->read(adb->name, 1, 24);
        adb->min_height = read_fle(memory_file);
        adb->average_height = read_fle(memory_file);
        adb->max_height = read_fle(memory_file);

        adb->records.resize(record_count);

        for (size_t i = 0; i < record_count; ++i) {
            memory_file->read(adb->records[i].name, 1, 16);
            adb->records[i].id = read_u32le(memory_file);
            adb->records[i].unknown1 = read_u32le(memory_file);
            adb->records[i].unknown2 = read_u32le(memory_file);
            adb->records[i].unknown3 = read_u32le(memory_file);
            adb->records[i].unknown4 = read_u32le(memory_file);
            adb->records[i].unknown5 = read_u32le(memory_file);
            adb->records[i].unknown6 = read_u32le(memory_file);
            adb->records[i].unknown7 = read_u32le(memory_file);
            adb->records[i].unknown8 = read_u32le(memory_file);
            adb->records[i].unknown9 = read_u32le(memory_file);
            adb->records[i].unknown10 = read_u32le(memory_file);
            adb->records[i].unknown11 = read_u32le(memory_file);
            adb->records[i].unknown12 = read_u32le(memory_file);
            adb->records[i].unknown13 = read_fle(memory_file);
            adb->records[i].unknown14 = read_u32le(memory_file);
            adb->records[i].unknown15 = read_u32le(memory_file);
            adb->records[i].unknown16 = read_u32le(memory_file);
            adb->records[i].unknown17 = read_u32le(memory_file);
        }

        return adb;
    }
}
