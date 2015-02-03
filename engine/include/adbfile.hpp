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
 * doc/formats/adb.txt
 */

#ifndef CE_ADBFILE_HPP
#define CE_ADBFILE_HPP

#include <cstddef>
#include <cstdint>

#include "memfile.hpp"

typedef struct {
    char name[16];
    uint32_t id;
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t unknown3;
    uint32_t unknown4;
    uint32_t unknown5;
    uint32_t unknown6;
    uint32_t unknown7;
    uint32_t unknown8;
    uint32_t unknown9;
    uint32_t unknown10;
    uint32_t unknown11;
    uint32_t unknown12;
    float unknown13;
    uint32_t unknown14;
    uint32_t unknown15;
    uint32_t unknown16;
    uint32_t unknown17;
} ce_adb_record;

typedef struct {
    uint32_t record_count;
    char name[24];
    float min_height;
    float average_height;
    float max_height;
    ce_adb_record* records;
} ce_adb_file;

extern ce_adb_file* ce_adb_file_new(ce_mem_file* mem_file);
extern void ce_adb_file_del(ce_adb_file* adb_file);

#endif
