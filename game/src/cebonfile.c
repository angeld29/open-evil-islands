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
#include "cebonfile.h"

ce_bonfile* ce_bonfile_open(ce_resfile* resfile, const char* name)
{
	ce_bonfile* bonfile = ce_alloc(sizeof(ce_bonfile));
	int index = ce_resfile_node_index(resfile, name);
	bonfile->size = ce_resfile_node_size(resfile, index);
	bonfile->data = ce_resfile_node_data(resfile, index);
	bonfile->bone = bonfile->data;
	return bonfile;
}

void ce_bonfile_close(ce_bonfile* bonfile)
{
	if (NULL != bonfile) {
		ce_free(bonfile->data, bonfile->size);
		ce_free(bonfile, sizeof(ce_bonfile));
	}
}
