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

#include <stdio.h>
#include <stdint.h>
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
	mmpfile->mipmap_count = ce_le2cpu32(*ptr++);
	mmpfile->format = ce_le2cpu32(*ptr++);
	mmpfile->bit_count = ce_le2cpu32(*ptr++);
	mmpfile->amask = ce_le2cpu32(*ptr++);
	mmpfile->ashift = ce_le2cpu32(*ptr++);
	mmpfile->acount = ce_le2cpu32(*ptr++);
	mmpfile->rmask = ce_le2cpu32(*ptr++);
	mmpfile->rshift = ce_le2cpu32(*ptr++);
	mmpfile->rcount = ce_le2cpu32(*ptr++);
	mmpfile->gmask = ce_le2cpu32(*ptr++);
	mmpfile->gshift = ce_le2cpu32(*ptr++);
	mmpfile->gcount = ce_le2cpu32(*ptr++);
	mmpfile->bmask = ce_le2cpu32(*ptr++);
	mmpfile->bshift = ce_le2cpu32(*ptr++);
	mmpfile->bcount = ce_le2cpu32(*ptr++);
	mmpfile->user_data_offset = ce_le2cpu32(*ptr++);
	mmpfile->texels = ptr;
	mmpfile->size = size;
	mmpfile->data = data;
	return mmpfile;
}

ce_mmpfile* ce_mmpfile_open_file(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL != file) {
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);
		fseek(file, 0, SEEK_SET);
		void* data = ce_alloc(size);
		fread(data, 1, size, file);
		fclose(file);
		return ce_mmpfile_open_data(data, size);
	}
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

void ce_mmpfile_save_file(ce_mmpfile* mmpfile, const char* path)
{
	assert(CE_MMPFILE_FORMAT_INVALID != mmpfile->format);
	assert(CE_MMPFILE_FORMAT_PNT3 != mmpfile->format);

	FILE* file = fopen(path, "wb");
	if (NULL != file) {
		uint32_t header[19];
		header[0] = ce_cpu2le32(CE_MMPFILE_SIGNATURE);
		header[1] = ce_cpu2le32(mmpfile->width);
		header[2] = ce_cpu2le32(mmpfile->height);
		header[3] = ce_cpu2le32(mmpfile->mipmap_count);
		header[4] = ce_cpu2le32(mmpfile->format);
		header[5] = ce_cpu2le32(mmpfile->bit_count);
		header[6] = ce_cpu2le32(mmpfile->amask);
		header[7] = ce_cpu2le32(mmpfile->ashift);
		header[8] = ce_cpu2le32(mmpfile->acount);
		header[9] = ce_cpu2le32(mmpfile->rmask);
		header[10] = ce_cpu2le32(mmpfile->rshift);
		header[11] = ce_cpu2le32(mmpfile->rcount);
		header[12] = ce_cpu2le32(mmpfile->gmask);
		header[13] = ce_cpu2le32(mmpfile->gshift);
		header[14] = ce_cpu2le32(mmpfile->gcount);
		header[15] = ce_cpu2le32(mmpfile->bmask);
		header[16] = ce_cpu2le32(mmpfile->bshift);
		header[17] = ce_cpu2le32(mmpfile->bcount);
		header[18] = ce_cpu2le32(mmpfile->user_data_offset);

		fwrite(header, sizeof(uint32_t), 19, file);
		fwrite(mmpfile->texels,
			ce_mmpfile_storage_requirements_mmpfile(mmpfile), 1, file);
		fclose(file);
	}
}

int ce_mmpfile_storage_requirements(int width, int height, int bit_count)
{
	return bit_count * width * height / 8;
}

int ce_mmpfile_storage_requirements_mipmap(int width, int height,
											int mipmap_count, int bit_count)
{
	int size = 0;
	for (int i = 0; i < mipmap_count; ++i, width >>= 1, height >>= 1) {
		size += ce_mmpfile_storage_requirements(width, height, bit_count);
	}
	return size;
}

int ce_mmpfile_storage_requirements_mmpfile(ce_mmpfile* mmpfile)
{
	return ce_mmpfile_storage_requirements_mipmap(mmpfile->width, mmpfile->height,
										mmpfile->mipmap_count, mmpfile->bit_count);
}
