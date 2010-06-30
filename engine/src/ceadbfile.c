/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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

#include <stddef.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cebyteorder.h"
#include "ceadbfile.h"

static const uint32_t CE_ADB_SIGNATURE = 0x00424441;

ce_adb_file* ce_adb_file_new(ce_memfile* memfile)
{
	uint32_t signature = ce_memfile_read_u32le(memfile);
	uint32_t record_count = ce_memfile_read_u32le(memfile);

	assert(CE_ADB_SIGNATURE == signature && "wrong signature");
	ce_unused(signature);

	ce_adb_file* adb_file = ce_alloc_zero(sizeof(ce_adb_file) +
										sizeof(ce_adb_record) * record_count);

	adb_file->record_count = record_count;
	ce_memfile_read(memfile, adb_file->name, 1, 24);
	adb_file->min_height = ce_memfile_read_fle(memfile);
	adb_file->average_height = ce_memfile_read_fle(memfile);
	adb_file->max_height = ce_memfile_read_fle(memfile);

	for (size_t i = 0; i < record_count; ++i) {
		ce_memfile_read(memfile, adb_file->records[i].name, 1, 16);
		adb_file->records[i].id = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown1 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown2 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown3 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown4 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown5 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown6 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown7 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown8 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown9 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown10 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown11 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown12 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown13 = ce_memfile_read_fle(memfile);
		adb_file->records[i].unknown14 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown15 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown16 = ce_memfile_read_u32le(memfile);
		adb_file->records[i].unknown17 = ce_memfile_read_u32le(memfile);
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
