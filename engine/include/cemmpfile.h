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

#include "cememfile.h"
#include "ceresfile.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
	CE_MMPFILE_FORMAT_UNKNOWN,
	// standard EI formats
	CE_MMPFILE_FORMAT_DXT1,
	CE_MMPFILE_FORMAT_DXT3,
	CE_MMPFILE_FORMAT_PNT3,
	CE_MMPFILE_FORMAT_R5G6B5,
	CE_MMPFILE_FORMAT_A1RGB5,
	CE_MMPFILE_FORMAT_ARGB4,
	CE_MMPFILE_FORMAT_ARGB8,
	// CE extensions
	CE_MMPFILE_FORMAT_RGB5A1,
	CE_MMPFILE_FORMAT_RGBA4,
	CE_MMPFILE_FORMAT_RGBA8,
	CE_MMPFILE_FORMAT_R8G8B8A8,
	CE_MMPFILE_FORMAT_COUNT
} ce_mmpfile_format;

enum {
	CE_MMPFILE_VERSION = 1
};

typedef struct {
	// standard EI header
	int width;
	int height;
	int mipmap_count;
	ce_mmpfile_format format;
	int bit_count;
	unsigned int amask, rmask, gmask, bmask;
	int ashift, rshift, gshift, bshift;
	int acount, rcount, gcount, bcount;
	int user_data_offset;
	void* texels;
	// CE extensions
	int version;
	int user_info;
	size_t size;
	void* data;
} ce_mmpfile;

extern int ce_mmpfile_storage_size(int width, int height,
									int mipmap_count, int bit_count);

extern ce_mmpfile* ce_mmpfile_new(int width, int height,
	int mipmap_count, ce_mmpfile_format format, int user_info);
extern ce_mmpfile* ce_mmpfile_new_data(void* data, size_t size);
extern ce_mmpfile* ce_mmpfile_new_memfile(ce_memfile* memfile);
extern ce_mmpfile* ce_mmpfile_new_resfile(ce_resfile* resfile, int index);
extern void ce_mmpfile_del(ce_mmpfile* mmpfile);

extern void ce_mmpfile_save(const ce_mmpfile* mmpfile, const char* path);
extern void ce_mmpfile_convert(ce_mmpfile* mmpfile, ce_mmpfile_format format);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_MMPFILE_H */
