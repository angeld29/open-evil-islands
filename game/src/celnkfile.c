/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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

#include <stdbool.h>
#include <stdint.h>

#include "cebyteorder.h"
#include "celogging.h"
#include "cealloc.h"
#include "cereshlp.h"
#include "celnkfile.h"

static ce_string* ce_lnkfile_read_name(ce_memfile* memfile)
{
	uint32_t length;
	if (1 != ce_memfile_read(memfile, &length, sizeof(uint32_t), 1)) {
		ce_logging_error("lnkfile: io error occured");
		return NULL;
	}

	ce_le2cpu32s(&length);

	char name[length];
	if (length != ce_memfile_read(memfile, name, 1, length)) {
		ce_logging_error("lnkfile: io error occured");
		return NULL;
	}

	return ce_string_new_str_n(name, length);
}

static bool ce_lnkfile_open_impl(ce_lnkfile* lnkfile, ce_memfile* memfile)
{
	uint32_t relationship_count;
	if (1 != ce_memfile_read(memfile,
			&relationship_count, sizeof(uint32_t), 1)) {
		ce_logging_error("lnkfile: io error occured");
		return false;
	}

	lnkfile->relationship_count = ce_le2cpu32(relationship_count);

	if (NULL == (lnkfile->relationships =
			ce_alloc_zero(sizeof(ce_lnkfile_relationship) *
							lnkfile->relationship_count))) {
		ce_logging_error("lnkfile: could not allocate memory");
		return false;
	}

	for (int i = 0; i < lnkfile->relationship_count; ++i) {
		if (NULL == (lnkfile->relationships[i].child_name =
						ce_lnkfile_read_name(memfile)) ||
				NULL == (lnkfile->relationships[i].parent_name =
							ce_lnkfile_read_name(memfile))) {
			return false;
		}
	}

	return true;
}

ce_lnkfile* ce_lnkfile_open_memfile(ce_memfile* memfile)
{
	ce_lnkfile* lnkfile = ce_alloc_zero(sizeof(ce_lnkfile));

	if (!ce_lnkfile_open_impl(lnkfile, memfile)) {
		ce_lnkfile_close(lnkfile);
		return NULL;
	}

	return lnkfile;
}

ce_lnkfile* ce_lnkfile_open_resfile(ce_resfile* resfile, const char* name)
{
	ce_memfile* memfile = ce_reshlp_extract_memfile_by_name(resfile, name);
	ce_lnkfile* lnkfile = ce_lnkfile_open_memfile(memfile);
	return ce_memfile_close(memfile), lnkfile;
}

void ce_lnkfile_close(ce_lnkfile* lnkfile)
{
	if (NULL != lnkfile) {
		if (NULL != lnkfile->relationships) {
			for (int i = 0; i < lnkfile->relationship_count; ++i) {
				ce_string_del(lnkfile->relationships[i].parent_name);
				ce_string_del(lnkfile->relationships[i].child_name);
			}
			ce_free(lnkfile->relationships, sizeof(ce_lnkfile_relationship) *
											lnkfile->relationship_count);
		}
		ce_free(lnkfile, sizeof(ce_lnkfile));
	}
}
