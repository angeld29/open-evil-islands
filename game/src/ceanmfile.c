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
#include "cereshlp.h"
#include "ceanmfile.h"

ce_anmfile* ce_anmfile_open_memfile(const char* name, ce_memfile* memfile)
{
	ce_anmfile* anmfile = ce_alloc(sizeof(ce_anmfile));
	anmfile->name = ce_string_new_str(name);

	ce_memfile_read(memfile,
		&anmfile->rotation_frame_count, sizeof(uint32_t), 1);

	ce_le2cpu32s(&anmfile->rotation_frame_count);

	anmfile->rotations = ce_alloc(sizeof(float) * 4 *
									anmfile->rotation_frame_count);

	ce_memfile_read(memfile, anmfile->rotations,
					sizeof(float) * 4, anmfile->rotation_frame_count);

	ce_memfile_read(memfile, &anmfile->translation_frame_count, sizeof(uint32_t), 1);

	ce_le2cpu32s(&anmfile->translation_frame_count);

	anmfile->translations =
		ce_alloc(sizeof(float) * 3 * anmfile->translation_frame_count);

	ce_memfile_read(memfile, anmfile->translations,
		sizeof(float) * 3, anmfile->translation_frame_count);

	ce_memfile_read(memfile, &anmfile->morph_frame_count, sizeof(uint32_t), 1);
	ce_memfile_read(memfile, &anmfile->morph_vertex_count, sizeof(uint32_t), 1);

	ce_le2cpu32s(&anmfile->morph_frame_count);
	ce_le2cpu32s(&anmfile->morph_vertex_count);

	if (0 != anmfile->morph_frame_count * anmfile->morph_vertex_count) {
		anmfile->morphs = ce_alloc(sizeof(float) * 3 *
			anmfile->morph_frame_count * anmfile->morph_vertex_count);

		ce_memfile_read(memfile, anmfile->morphs, sizeof(float) * 3 *
			anmfile->morph_vertex_count, anmfile->morph_frame_count);
	}

	return anmfile;
}

ce_anmfile* ce_anmfile_open_resfile(ce_resfile* resfile, int index)
{
	ce_memfile* memfile = ce_reshlp_extract_memfile(resfile, index);
	ce_anmfile* anmfile = ce_anmfile_open_memfile(resfile->name->str, memfile);
	return ce_memfile_close(memfile), anmfile;
}

void ce_anmfile_close(ce_anmfile* anmfile)
{
	if (NULL != anmfile) {
		ce_free(anmfile->morphs, sizeof(float) * 3 *
									anmfile->morph_frame_count *
									anmfile->morph_vertex_count);
		ce_free(anmfile->translations,
				sizeof(float) * 3 * anmfile->translation_frame_count);
		ce_free(anmfile->rotations,
				sizeof(float) * 4 * anmfile->rotation_frame_count);
		ce_string_del(anmfile->name);
		ce_free(anmfile, sizeof(ce_anmfile));
	}
}
