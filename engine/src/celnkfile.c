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

/*
 *  See doc/formats/lnkfile.txt for more details.
*/

#include "cebyteorder.h"
#include "cealloc.h"
#include "celnkfile.h"

ce_lnkfile* ce_lnkfile_open(ce_resfile* resfile, const char* name)
{
	int index = ce_resfile_node_index(resfile, name);
	size_t size = ce_resfile_node_size(resfile, index);
	void* data = ce_resfile_node_data(resfile, index);

	union {
		char* c;
		uint32_t* u32;
	} ptr = { data };

	ce_lnkfile* lnkfile = ce_alloc(sizeof(ce_lnkfile));
	lnkfile->link_count = ce_le2cpu32(*ptr.u32++);
	lnkfile->link_index = 0;
	lnkfile->links = ce_alloc(sizeof(ce_lnklink) * lnkfile->link_count);

	for (int i = 0, length; i < lnkfile->link_count; ++i, ptr.c += length) {
		length = ce_le2cpu32(*ptr.u32++);
		lnkfile->links[i].child_name = ce_string_new_str_n(ptr.c, length);
		ptr.c += length;
		length = ce_le2cpu32(*ptr.u32++);
		lnkfile->links[i].parent_name = ce_string_new_str_n(ptr.c, length);
	}

	ce_free(data, size);
	return lnkfile;
}

void ce_lnkfile_close(ce_lnkfile* lnkfile)
{
	if (NULL != lnkfile) {
		for (int i = 0; i < lnkfile->link_count; ++i) {
			ce_string_del(lnkfile->links[i].parent_name);
			ce_string_del(lnkfile->links[i].child_name);
		}
		ce_free(lnkfile->links, sizeof(ce_lnklink) * lnkfile->link_count);
		ce_free(lnkfile, sizeof(ce_lnkfile));
	}
}
