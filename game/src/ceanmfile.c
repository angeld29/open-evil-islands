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

#include "cebyteorder.h"
#include "cealloc.h"
#include "ceanmfile.h"

ce_anmfile* ce_anmfile_open(ce_resfile* resfile, int index)
{
	ce_anmfile* anmfile = ce_alloc(sizeof(ce_anmfile));
	anmfile->name = ce_string_dup(resfile->name);
	anmfile->size = ce_resfile_node_size(resfile, index);
	anmfile->data = ce_resfile_node_data(resfile, index);

	union {
		uint32_t* u32ptr;
		float* fptr;
	} data = { anmfile->data };

	anmfile->rotation_frame_count = ce_le2cpu32(*data.u32ptr++);
	anmfile->rotations = data.fptr;
	data.fptr += 4 * anmfile->rotation_frame_count;

	anmfile->translation_frame_count = ce_le2cpu32(*data.u32ptr++);
	anmfile->translations = data.fptr;
	data.fptr += 3 * anmfile->translation_frame_count;

	anmfile->morph_frame_count = ce_le2cpu32(*data.u32ptr++);
	anmfile->morph_vertex_count = ce_le2cpu32(*data.u32ptr++);
	anmfile->morphs = 0 != anmfile->morph_frame_count *
							anmfile->morph_vertex_count ? data.fptr : NULL;

	return anmfile;
}

void ce_anmfile_close(ce_anmfile* anmfile)
{
	if (NULL != anmfile) {
		ce_free(anmfile->data, anmfile->size);
		ce_string_del(anmfile->name);
		ce_free(anmfile, sizeof(ce_anmfile));
	}
}
