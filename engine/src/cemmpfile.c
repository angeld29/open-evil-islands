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
#include <string.h>
#include <assert.h>

#include "celib.h"
#include "cebyteorder.h"
#include "cealloc.h"
#include "cepixelfmt.h"
#include "cemmpfile.h"

static const unsigned int CE_MMPFILE_SIGNATURE = 0x504d4d;

static const unsigned int ce_mmpfile_format_signatures[CE_MMPFILE_FORMAT_COUNT] = {
	0x0,
	0x31545844,
	0x33545844,
	0x33544e50,
	0x5650,
	0x5551,
	0x4444,
	0x8888,
	0x45431555,
	0x45434444,
	0x45438888,
	0x45435442
};

static ce_mmpfile_format ce_mmpfile_format_find(unsigned int signature)
{
	for (int i = 0; i < CE_MMPFILE_FORMAT_COUNT; ++i) {
		if (ce_mmpfile_format_signatures[i] == signature) {
			return i;
		}
	}
	return CE_MMPFILE_FORMAT_UNKNOWN;
}

static void ce_mmpfile_write_header_unknown(ce_mmpfile* mmpfile)
{
	assert(false && "not implemented");
	ce_unused(mmpfile);
}

static void ce_mmpfile_write_header_dxt1(ce_mmpfile* mmpfile)
{
	mmpfile->bit_count = 4;
	mmpfile->amask = 0x8000; mmpfile->ashift = 15; mmpfile->acount = 1;
	mmpfile->rmask = 0x7c00; mmpfile->rshift = 10; mmpfile->rcount = 5;
	mmpfile->gmask = 0x3e0;  mmpfile->gshift = 5;  mmpfile->gcount = 5;
	mmpfile->bmask = 0x1f;   mmpfile->bshift = 0;  mmpfile->bcount = 5;
}

static void ce_mmpfile_write_header_dxt3(ce_mmpfile* mmpfile)
{
	mmpfile->bit_count = 8;
	mmpfile->amask = 0xf000; mmpfile->ashift = 12; mmpfile->acount = 4;
	mmpfile->rmask = 0xf00;  mmpfile->rshift = 8;  mmpfile->rcount = 4;
	mmpfile->gmask = 0xf0;   mmpfile->gshift = 4;  mmpfile->gcount = 4;
	mmpfile->bmask = 0xf;    mmpfile->bshift = 0;  mmpfile->bcount = 4;
}

static void ce_mmpfile_write_header_r5g6b5(ce_mmpfile* mmpfile)
{
	mmpfile->bit_count = 16;
	mmpfile->amask = 0x0000; mmpfile->ashift = 0;  mmpfile->acount = 0;
	mmpfile->rmask = 0xf800; mmpfile->rshift = 11; mmpfile->rcount = 5;
	mmpfile->gmask = 0x7e0;  mmpfile->gshift = 5;  mmpfile->gcount = 6;
	mmpfile->bmask = 0x1f;   mmpfile->bshift = 0;  mmpfile->bcount = 5;
}

static void ce_mmpfile_write_header_a1rgb5(ce_mmpfile* mmpfile)
{
	mmpfile->bit_count = 16;
	mmpfile->amask = 0x8000; mmpfile->ashift = 15; mmpfile->acount = 1;
	mmpfile->rmask = 0x7c00; mmpfile->rshift = 10; mmpfile->rcount = 5;
	mmpfile->gmask = 0x3e0;  mmpfile->gshift = 5;  mmpfile->gcount = 5;
	mmpfile->bmask = 0x1f;   mmpfile->bshift = 0;  mmpfile->bcount = 5;
}

static void ce_mmpfile_write_header_argb4(ce_mmpfile* mmpfile)
{
	mmpfile->bit_count = 16;
	mmpfile->amask = 0xf000; mmpfile->ashift = 12; mmpfile->acount = 4;
	mmpfile->rmask = 0xf00;  mmpfile->rshift = 8;  mmpfile->rcount = 4;
	mmpfile->gmask = 0xf0;   mmpfile->gshift = 4;  mmpfile->gcount = 4;
	mmpfile->bmask = 0xf;    mmpfile->bshift = 0;  mmpfile->bcount = 4;
}

static void ce_mmpfile_write_header_argb8(ce_mmpfile* mmpfile)
{
	mmpfile->bit_count = 32;
	mmpfile->amask = 0xff000000; mmpfile->ashift = 24; mmpfile->acount = 8;
	mmpfile->rmask = 0xff0000;   mmpfile->rshift = 16; mmpfile->rcount = 8;
	mmpfile->gmask = 0xff00;     mmpfile->gshift = 8;  mmpfile->gcount = 8;
	mmpfile->bmask = 0xff;       mmpfile->bshift = 0;  mmpfile->bcount = 8;
}

static void ce_mmpfile_write_header_rgb5a1(ce_mmpfile* mmpfile)
{
	mmpfile->bit_count = 16;
	mmpfile->amask = 0x1f;   mmpfile->ashift = 0;  mmpfile->acount = 5;
	mmpfile->rmask = 0x8000; mmpfile->rshift = 15; mmpfile->rcount = 1;
	mmpfile->gmask = 0x7c00; mmpfile->gshift = 10; mmpfile->gcount = 5;
	mmpfile->bmask = 0x3e0;  mmpfile->bshift = 5;  mmpfile->bcount = 5;
}

static void ce_mmpfile_write_header_rgba4(ce_mmpfile* mmpfile)
{
	mmpfile->bit_count = 16;
	mmpfile->amask = 0xf;    mmpfile->ashift = 0;  mmpfile->acount = 4;
	mmpfile->rmask = 0xf000; mmpfile->rshift = 12; mmpfile->rcount = 4;
	mmpfile->gmask = 0xf00;  mmpfile->gshift = 8;  mmpfile->gcount = 4;
	mmpfile->bmask = 0xf0;   mmpfile->bshift = 4;  mmpfile->bcount = 4;
}

static void ce_mmpfile_write_header_rgba8(ce_mmpfile* mmpfile)
{
	mmpfile->bit_count = 32;
	mmpfile->amask = 0xff;       mmpfile->ashift = 0;  mmpfile->acount = 8;
	mmpfile->rmask = 0xff000000; mmpfile->rshift = 24; mmpfile->rcount = 8;
	mmpfile->gmask = 0xff0000;   mmpfile->gshift = 16; mmpfile->gcount = 8;
	mmpfile->bmask = 0xff00;     mmpfile->bshift = 8;  mmpfile->bcount = 8;
}

static void ce_mmpfile_write_header_r8g8b8a8(ce_mmpfile* mmpfile)
{
	mmpfile->bit_count = 32;
	mmpfile->amask = 0x0; mmpfile->ashift = 0; mmpfile->acount = 0;
	mmpfile->rmask = 0x0; mmpfile->rshift = 0; mmpfile->rcount = 0;
	mmpfile->gmask = 0x0; mmpfile->gshift = 0; mmpfile->gcount = 0;
	mmpfile->bmask = 0x0; mmpfile->bshift = 0; mmpfile->bcount = 0;
}

static void (*ce_mmpfile_write_header_procs[CE_MMPFILE_FORMAT_COUNT])(ce_mmpfile*) = {
	ce_mmpfile_write_header_unknown,
	ce_mmpfile_write_header_dxt1,
	ce_mmpfile_write_header_dxt3,
	ce_mmpfile_write_header_unknown,
	ce_mmpfile_write_header_r5g6b5,
	ce_mmpfile_write_header_a1rgb5,
	ce_mmpfile_write_header_argb4,
	ce_mmpfile_write_header_argb8,
	ce_mmpfile_write_header_rgb5a1,
	ce_mmpfile_write_header_rgba4,
	ce_mmpfile_write_header_rgba8,
	ce_mmpfile_write_header_r8g8b8a8
};

static void ce_mmpfile_write_header(ce_mmpfile* mmpfile,
									ce_mmpfile_format format)
{
	(*ce_mmpfile_write_header_procs[mmpfile->format = format])(mmpfile);
}

ce_mmpfile* ce_mmpfile_new(int width, int height,
								int mipmap_count,
								ce_mmpfile_format format)
{
	ce_mmpfile* mmpfile = ce_alloc(sizeof(ce_mmpfile));
	mmpfile->width = width;
	mmpfile->height = height;
	mmpfile->mipmap_count = mipmap_count;

	ce_mmpfile_write_header(mmpfile, format);

	mmpfile->size = ce_mmpfile_storage_size(width, height,
											mipmap_count, mmpfile->bit_count);

	mmpfile->data = ce_alloc(mmpfile->size);
	mmpfile->texels = mmpfile->data;

	return mmpfile;
}

ce_mmpfile* ce_mmpfile_new_data(void* data, size_t size)
{
	uint32_t* ptr = data;

	uint32_t signature = ce_le2cpu32(*ptr++);
	assert(CE_MMPFILE_SIGNATURE == signature && "wrong signature");
	ce_unused(signature);

	ce_mmpfile* mmpfile = ce_alloc(sizeof(ce_mmpfile));
	mmpfile->width = ce_le2cpu32(*ptr++);
	mmpfile->height = ce_le2cpu32(*ptr++);
	mmpfile->mipmap_count = ce_le2cpu32(*ptr++);
	mmpfile->format = ce_mmpfile_format_find(ce_le2cpu32(*ptr++));
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

ce_mmpfile* ce_mmpfile_new_file(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (NULL != file) {
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);
		fseek(file, 0, SEEK_SET);
		void* data = ce_alloc(size);
		fread(data, 1, size, file);
		fclose(file);
		return ce_mmpfile_new_data(data, size);
	}
	return NULL;
}

ce_mmpfile* ce_mmpfile_new_resfile(ce_resfile* resfile, int index)
{
	return ce_mmpfile_new_data(ce_resfile_node_data(resfile, index),
								ce_resfile_node_size(resfile, index));
}

void ce_mmpfile_del(ce_mmpfile* mmpfile)
{
	if (NULL != mmpfile) {
		ce_free(mmpfile->data, mmpfile->size);
		ce_free(mmpfile, sizeof(ce_mmpfile));
	}
}

void ce_mmpfile_save(ce_mmpfile* mmpfile, const char* path)
{
	assert(CE_MMPFILE_FORMAT_PNT3 != mmpfile->format);

	FILE* file = fopen(path, "wb");
	if (NULL != file) {
		uint32_t header[19];
		header[0] = ce_cpu2le32(CE_MMPFILE_SIGNATURE);
		header[1] = ce_cpu2le32(mmpfile->width);
		header[2] = ce_cpu2le32(mmpfile->height);
		header[3] = ce_cpu2le32(mmpfile->mipmap_count);
		header[4] = ce_cpu2le32(ce_mmpfile_format_signatures[mmpfile->format]);
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
		fwrite(mmpfile->texels, 1, ce_mmpfile_storage_size(mmpfile->width,
			mmpfile->height, mmpfile->mipmap_count, mmpfile->bit_count), file);
		fclose(file);
	}
}

int ce_mmpfile_storage_size(int width, int height,
							int mipmap_count, int bit_count)
{
	int size = 0;
	for (int i = 0; i < mipmap_count; ++i, width >>= 1, height >>= 1) {
		size += bit_count * width * height / 8;
	}
	return size;
}

static void ce_mmpfile_convert_unknown(ce_mmpfile* mmpfile,
										ce_mmpfile_format format)
{
	assert(false && "not implemented");
	ce_unused(mmpfile), ce_unused(format);
}

static void ce_mmpfile_convert_dxt1(ce_mmpfile* mmpfile,
									ce_mmpfile_format format)
{
	assert(CE_MMPFILE_FORMAT_RGBA8 == format && "not implemented");
}

static void ce_mmpfile_convert_dxt3(ce_mmpfile* mmpfile,
									ce_mmpfile_format format)
{
	assert(CE_MMPFILE_FORMAT_RGBA8 == format && "not implemented");
}

static void ce_mmpfile_decompress_pnt3(uint8_t* restrict dst,
								const uint32_t* restrict src, int size)
{
	assert(0 == size % sizeof(uint32_t));

	const uint32_t* end = src + size / sizeof(uint32_t);

	int n = 0;
	uint32_t v;

	while (src != end) {
		v = ce_le2cpu32(*src++);
		if (v > 1000000 || 0 == v) {
			++n;
		} else {
			memcpy(dst, src - 1 - n, n * sizeof(uint32_t));
			dst += n * sizeof(uint32_t);
			memset(dst, '\0', v);
			dst += v;
			n = 0;
		}
	}

	memcpy(dst, src - n, n * sizeof(uint32_t));
}

static void ce_mmpfile_convert_pnt3(ce_mmpfile* mmpfile,
									ce_mmpfile_format format)
{
	assert(CE_MMPFILE_FORMAT_ARGB8 == format && "not implemented");

	int size = ce_mmpfile_storage_size(mmpfile->width, mmpfile->height,
												1, mmpfile->bit_count);

	// mipmap_count == compressed size for pnt3
	if (mmpfile->mipmap_count < size) { // pnt3 compressed
		void* texels = ce_alloc(size);
		ce_mmpfile_decompress_pnt3(texels, mmpfile->texels, mmpfile->mipmap_count);
		ce_free(mmpfile->data, mmpfile->size);
		mmpfile->texels = texels;
		mmpfile->size = size;
		mmpfile->data = texels;
	}

	mmpfile->mipmap_count = 1;

	ce_mmpfile_write_header(mmpfile, format);
}

static void (*ce_mmpfile_convert_procs[CE_MMPFILE_FORMAT_COUNT])
								(ce_mmpfile*, ce_mmpfile_format) = {
	ce_mmpfile_convert_unknown,
	ce_mmpfile_convert_dxt1,
	ce_mmpfile_convert_dxt3,
	ce_mmpfile_convert_pnt3,
	ce_mmpfile_convert_unknown,
	ce_mmpfile_convert_unknown,
	ce_mmpfile_convert_unknown,
	ce_mmpfile_convert_unknown,
	ce_mmpfile_convert_unknown,
	ce_mmpfile_convert_unknown,
	ce_mmpfile_convert_unknown,
	ce_mmpfile_convert_unknown
};

void ce_mmpfile_convert(ce_mmpfile* mmpfile, ce_mmpfile_format format)
{
	(*ce_mmpfile_convert_procs[mmpfile->format])(mmpfile, format);
}
