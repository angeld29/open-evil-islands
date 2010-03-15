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

#include "cealloc.h"
#include "cereshlp.h"
#include "cebonfile.h"

ce_bonfile* ce_bonfile_open_memfile(int value_count, ce_memfile* memfile)
{
	ce_bonfile* bonfile = ce_alloc_zero(sizeof(ce_bonfile));
	bonfile->value_count = value_count;
	bonfile->bone = ce_alloc(sizeof(float) * 3 * bonfile->value_count);
	ce_memfile_read(memfile, bonfile->bone,
					sizeof(float) * 3 * value_count, 1);
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
