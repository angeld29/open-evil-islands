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

#include "celogging.h"
#include "cealloc.h"
#include "cereshlp.h"
#include "cebonfile.h"

static bool ce_bonfile_open_impl(ce_bonfile* bonfile, ce_memfile* memfile)
{
	if (NULL == (bonfile->bone =
					ce_alloc(sizeof(float) * 3 * bonfile->value_count))) {
		ce_logging_error("bonfile: could not allocate memory");
		return false;
	}

	if (1 != ce_memfile_read(memfile, bonfile->bone,
								sizeof(float) * 3 * bonfile->value_count, 1)) {
		ce_logging_error("bonfile: io error occured");
		return false;
	}

	return true;
}

ce_bonfile* ce_bonfile_open_memfile(int value_count, ce_memfile* memfile)
{
	ce_bonfile* bonfile = ce_alloc_zero(sizeof(ce_bonfile));
	if (NULL == bonfile) {
		ce_logging_error("bonfile: could not allocate memory");
		return NULL;
	}

	bonfile->value_count = value_count;

	if (!ce_bonfile_open_impl(bonfile, memfile)) {
		ce_bonfile_close(bonfile);
		return NULL;
	}

	return bonfile;
}

ce_bonfile* ce_bonfile_open_resfile(int value_count,
									ce_resfile* resfile,
									const char* name)
{
	ce_memfile* memfile = ce_reshlp_extract_memfile_by_name(resfile, name);
	ce_bonfile* bonfile = ce_bonfile_open_memfile(value_count, memfile);
	return ce_memfile_close(memfile), bonfile;
}

void ce_bonfile_close(ce_bonfile* bonfile)
{
	if (NULL != bonfile) {
		ce_free(bonfile->bone, sizeof(float) * 3 * bonfile->value_count);
		ce_free(bonfile, sizeof(ce_bonfile));
	}
}
