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

#ifndef CE_MMPFILE_H
#define CE_MMPFILE_H

#include <stddef.h>

#include "ceresfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

enum {
	CE_MMPFILE_FORMAT_GENERIC,
	CE_MMPFILE_FORMAT_DXT1 = 0x31545844,
	CE_MMPFILE_FORMAT_DXT3 = 0x33545844,
	CE_MMPFILE_FORMAT_PNT3 = 0x33544e50,
	CE_MMPFILE_FORMAT_R5G6B5 = 0x5650,
	CE_MMPFILE_FORMAT_A1RGB5 = 0x5551,
	CE_MMPFILE_FORMAT_ARGB4 = 0x4444,
	CE_MMPFILE_FORMAT_ARGB8 = 0x8888
};

typedef struct {
	int width;
	int height;
	int mipmap_count;
	int format;
	int bit_count;
	unsigned int amask, rmask, gmask, bmask;
	int ashift, rshift, gshift, bshift;
	int acount, rcount, gcount, bcount;
	int user_data_offset;
	void* texels;
	size_t size;
	void* data;
} ce_mmpfile;

extern ce_mmpfile* ce_mmpfile_open_data(void* data, size_t size);
extern ce_mmpfile* ce_mmpfile_open_file(const char* path);
extern ce_mmpfile* ce_mmpfile_open_resfile(ce_resfile* resfile, int index);
extern void ce_mmpfile_close(ce_mmpfile* mmpfile);

extern void ce_mmpfile_save_file(ce_mmpfile* mmpfile, const char* path);

extern void ce_mmpfile_replace_texels(ce_mmpfile* mmpfile,
										void* texels, size_t size);

extern int ce_mmpfile_storage_requirements(int width, int height, int bit_count);
extern int ce_mmpfile_storage_requirements_mipmap(int width, int height,
												int mipmap_count, int bit_count);
extern int ce_mmpfile_storage_requirements_mmpfile(ce_mmpfile* mmpfile);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MMPFILE_H */
