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
 *  See doc/formats/mmpfile.txt for more details.
*/

#include <assert.h>

#include "celib.h"
#include "cebyteorder.h"
#include "cealloc.h"
#include "cemmpfile.h"

static const unsigned int CE_MMPFILE_SIGNATURE = 0x504d4d;

ce_mmpfile* ce_mmpfile_open_data(void* data, size_t size)
{
	uint32_t* ptr = data;

	uint32_t signature = ce_le2cpu32(*ptr++);
	assert(CE_MMPFILE_SIGNATURE == signature && "wrong signature");
	ce_unused(signature);

	ce_mmpfile* mmpfile = ce_alloc(sizeof(ce_mmpfile));
	mmpfile->width = ce_le2cpu32(*ptr++);
	mmpfile->height = ce_le2cpu32(*ptr++);
	mmpfile->info.size = ce_le2cpu32(*ptr++);
	mmpfile->format = ce_le2cpu32(*ptr++);
	mmpfile->texels = ptr += 14;
	mmpfile->size = size;
	mmpfile->data = data;
	return mmpfile;
}

ce_mmpfile* ce_mmpfile_open_file(const char* path)
{
	return NULL;
}

ce_mmpfile* ce_mmpfile_open_resfile(ce_resfile* resfile, int index)
{
	return ce_mmpfile_open_data(ce_resfile_node_data(resfile, index),
								ce_resfile_node_size(resfile, index));
}

void ce_mmpfile_close(ce_mmpfile* mmpfile)
{
	if (NULL != mmpfile) {
		ce_free(mmpfile->data, mmpfile->size);
		ce_free(mmpfile, sizeof(ce_mmpfile));
	}
}
