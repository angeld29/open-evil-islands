/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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
 *  See doc/formats/mmpfile.txt for more details
*/

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

#include <squish.h>

#include "celib.h"
#include "cealloc.h"
#include "cebyteorder.h"
#include "cemmpfile.h"

static const uint32_t CE_MMPFILE_SIGNATURE = 0x504d4d;
static const uint32_t CE_MMPFILE_SIGNATURE_EXT = 0x45434d4d;

enum {
	CE_MMPFILE_HEADER_SIZE = 76,
};

static const uint32_t ce_mmpfile_format_signatures[CE_MMPFILE_FORMAT_COUNT] = {
	[CE_MMPFILE_FORMAT_UNKNOWN] = 0x0,
	[CE_MMPFILE_FORMAT_DXT1] = 0x31545844,
	[CE_MMPFILE_FORMAT_DXT3] = 0x33545844,
	[CE_MMPFILE_FORMAT_PNT3] = 0x33544e50,
	[CE_MMPFILE_FORMAT_R5G6B5] = 0x5650,
	[CE_MMPFILE_FORMAT_A1RGB5] = 0x5551,
	[CE_MMPFILE_FORMAT_ARGB4] = 0x4444,
	[CE_MMPFILE_FORMAT_ARGB8] = 0x8888,
	[CE_MMPFILE_FORMAT_RGB5A1] = 0x45431555,
	[CE_MMPFILE_FORMAT_RGBA4] = 0x45434444,
	[CE_MMPFILE_FORMAT_RGBA8] = 0x45438888,
	[CE_MMPFILE_FORMAT_R8G8B8A8] = 0x45435442,
	[CE_MMPFILE_FORMAT_YCBCR] = 0x4543554c,
};

static ce_mmpfile_format ce_mmpfile_format_find(uint32_t signature)
{
	for (int i = 0; i < CE_MMPFILE_FORMAT_COUNT; ++i) {
		if (ce_mmpfile_format_signatures[i] == signature) {
			return i;
		}
	}
	return CE_MMPFILE_FORMAT_UNKNOWN;
}

static const uint32_t ce_mmpfile_bit_counts[CE_MMPFILE_FORMAT_COUNT] = {
	[CE_MMPFILE_FORMAT_UNKNOWN] = 0,
	[CE_MMPFILE_FORMAT_DXT1] = 4,
	[CE_MMPFILE_FORMAT_DXT3] = 8,
	[CE_MMPFILE_FORMAT_PNT3] = 0,
	[CE_MMPFILE_FORMAT_R5G6B5] = 16,
	[CE_MMPFILE_FORMAT_A1RGB5] = 16,
	[CE_MMPFILE_FORMAT_ARGB4] = 16,
	[CE_MMPFILE_FORMAT_ARGB8] = 32,
	[CE_MMPFILE_FORMAT_RGB5A1] = 16,
	[CE_MMPFILE_FORMAT_RGBA4] = 16,
	[CE_MMPFILE_FORMAT_RGBA8] = 32,
	[CE_MMPFILE_FORMAT_R8G8B8A8] = 32,
	[CE_MMPFILE_FORMAT_YCBCR] = 24,
};

static void ce_mmpfile_write_header_null(ce_mmpfile* mmpfile)
{
	mmpfile->amask = 0x0; mmpfile->ashift = 0; mmpfile->acount = 0;
	mmpfile->rmask = 0x0; mmpfile->rshift = 0; mmpfile->rcount = 0;
	mmpfile->gmask = 0x0; mmpfile->gshift = 0; mmpfile->gcount = 0;
	mmpfile->bmask = 0x0; mmpfile->bshift = 0; mmpfile->bcount = 0;
}

static void ce_mmpfile_write_header_dxt1(ce_mmpfile* mmpfile)
{
	mmpfile->amask = 0x8000; mmpfile->ashift = 15; mmpfile->acount = 1;
	mmpfile->rmask = 0x7c00; mmpfile->rshift = 10; mmpfile->rcount = 5;
	mmpfile->gmask = 0x3e0;  mmpfile->gshift = 5;  mmpfile->gcount = 5;
	mmpfile->bmask = 0x1f;   mmpfile->bshift = 0;  mmpfile->bcount = 5;
}

static void ce_mmpfile_write_header_dxt3(ce_mmpfile* mmpfile)
{
	mmpfile->amask = 0xf000; mmpfile->ashift = 12; mmpfile->acount = 4;
	mmpfile->rmask = 0xf00;  mmpfile->rshift = 8;  mmpfile->rcount = 4;
	mmpfile->gmask = 0xf0;   mmpfile->gshift = 4;  mmpfile->gcount = 4;
	mmpfile->bmask = 0xf;    mmpfile->bshift = 0;  mmpfile->bcount = 4;
}

static void ce_mmpfile_write_header_r5g6b5(ce_mmpfile* mmpfile)
{
	mmpfile->amask = 0x0;    mmpfile->ashift = 0;  mmpfile->acount = 0;
	mmpfile->rmask = 0xf800; mmpfile->rshift = 11; mmpfile->rcount = 5;
	mmpfile->gmask = 0x7e0;  mmpfile->gshift = 5;  mmpfile->gcount = 6;
	mmpfile->bmask = 0x1f;   mmpfile->bshift = 0;  mmpfile->bcount = 5;
}

static void ce_mmpfile_write_header_a1rgb5(ce_mmpfile* mmpfile)
{
	mmpfile->amask = 0x8000; mmpfile->ashift = 15; mmpfile->acount = 1;
	mmpfile->rmask = 0x7c00; mmpfile->rshift = 10; mmpfile->rcount = 5;
	mmpfile->gmask = 0x3e0;  mmpfile->gshift = 5;  mmpfile->gcount = 5;
	mmpfile->bmask = 0x1f;   mmpfile->bshift = 0;  mmpfile->bcount = 5;
}

static void ce_mmpfile_write_header_argb4(ce_mmpfile* mmpfile)
{
	mmpfile->amask = 0xf000; mmpfile->ashift = 12; mmpfile->acount = 4;
	mmpfile->rmask = 0xf00;  mmpfile->rshift = 8;  mmpfile->rcount = 4;
	mmpfile->gmask = 0xf0;   mmpfile->gshift = 4;  mmpfile->gcount = 4;
	mmpfile->bmask = 0xf;    mmpfile->bshift = 0;  mmpfile->bcount = 4;
}

static void ce_mmpfile_write_header_argb8(ce_mmpfile* mmpfile)
{
	mmpfile->amask = 0xff000000; mmpfile->ashift = 24; mmpfile->acount = 8;
	mmpfile->rmask = 0xff0000;   mmpfile->rshift = 16; mmpfile->rcount = 8;
	mmpfile->gmask = 0xff00;     mmpfile->gshift = 8;  mmpfile->gcount = 8;
	mmpfile->bmask = 0xff;       mmpfile->bshift = 0;  mmpfile->bcount = 8;
}

static void ce_mmpfile_write_header_rgb5a1(ce_mmpfile* mmpfile)
{
	mmpfile->amask = 0x1f;   mmpfile->ashift = 0;  mmpfile->acount = 5;
	mmpfile->rmask = 0x8000; mmpfile->rshift = 15; mmpfile->rcount = 1;
	mmpfile->gmask = 0x7c00; mmpfile->gshift = 10; mmpfile->gcount = 5;
	mmpfile->bmask = 0x3e0;  mmpfile->bshift = 5;  mmpfile->bcount = 5;
}

static void ce_mmpfile_write_header_rgba4(ce_mmpfile* mmpfile)
{
	mmpfile->amask = 0xf;    mmpfile->ashift = 0;  mmpfile->acount = 4;
	mmpfile->rmask = 0xf000; mmpfile->rshift = 12; mmpfile->rcount = 4;
	mmpfile->gmask = 0xf00;  mmpfile->gshift = 8;  mmpfile->gcount = 4;
	mmpfile->bmask = 0xf0;   mmpfile->bshift = 4;  mmpfile->bcount = 4;
}

static void ce_mmpfile_write_header_rgba8(ce_mmpfile* mmpfile)
{
	mmpfile->amask = 0xff;       mmpfile->ashift = 0;  mmpfile->acount = 8;
	mmpfile->rmask = 0xff000000; mmpfile->rshift = 24; mmpfile->rcount = 8;
	mmpfile->gmask = 0xff0000;   mmpfile->gshift = 16; mmpfile->gcount = 8;
	mmpfile->bmask = 0xff00;     mmpfile->bshift = 8;  mmpfile->bcount = 8;
}

static void (*ce_mmpfile_write_header_procs[CE_MMPFILE_FORMAT_COUNT])(ce_mmpfile*) = {
	[CE_MMPFILE_FORMAT_UNKNOWN] = ce_mmpfile_write_header_null,
	[CE_MMPFILE_FORMAT_DXT1] = ce_mmpfile_write_header_dxt1,
	[CE_MMPFILE_FORMAT_DXT3] = ce_mmpfile_write_header_dxt3,
	[CE_MMPFILE_FORMAT_PNT3] = ce_mmpfile_write_header_null,
	[CE_MMPFILE_FORMAT_R5G6B5] = ce_mmpfile_write_header_r5g6b5,
	[CE_MMPFILE_FORMAT_A1RGB5] = ce_mmpfile_write_header_a1rgb5,
	[CE_MMPFILE_FORMAT_ARGB4] = ce_mmpfile_write_header_argb4,
	[CE_MMPFILE_FORMAT_ARGB8] = ce_mmpfile_write_header_argb8,
	[CE_MMPFILE_FORMAT_RGB5A1] = ce_mmpfile_write_header_rgb5a1,
	[CE_MMPFILE_FORMAT_RGBA4] = ce_mmpfile_write_header_rgba4,
	[CE_MMPFILE_FORMAT_RGBA8] = ce_mmpfile_write_header_rgba8,
	[CE_MMPFILE_FORMAT_R8G8B8A8] = ce_mmpfile_write_header_null,
	[CE_MMPFILE_FORMAT_YCBCR] = ce_mmpfile_write_header_null,
};

static inline size_t ce_mmpfile_storage_size_generic(unsigned int width,
												unsigned int height,
												ce_mmpfile_format format)
{
	return ce_mmpfile_bit_counts[format] * width * height / 8;
}

static size_t ce_mmpfile_storage_size_dxt(unsigned int width,
											unsigned int height,
											ce_mmpfile_format format)
{
	// special case for DXT format that have a block of fixed size:
	// 8 for DXT1, 16 for DXT3
	return ce_max(size_t, 2 * ce_mmpfile_bit_counts[format],
		ce_mmpfile_storage_size_generic(width, height, format));
}

static size_t ce_mmpfile_storage_size_ycbcr(unsigned int width,
											unsigned int height,
											ce_mmpfile_format format)
{
	ce_unused(format);
	return 3 * width * height / 2;
}

static size_t (*ce_mmpfile_storage_size_procs[CE_MMPFILE_FORMAT_COUNT])
		(unsigned int width, unsigned int height, ce_mmpfile_format format) = {
	[CE_MMPFILE_FORMAT_UNKNOWN] = ce_mmpfile_storage_size_generic,
	[CE_MMPFILE_FORMAT_DXT1] = ce_mmpfile_storage_size_dxt,
	[CE_MMPFILE_FORMAT_DXT3] = ce_mmpfile_storage_size_dxt,
	[CE_MMPFILE_FORMAT_PNT3] = ce_mmpfile_storage_size_generic,
	[CE_MMPFILE_FORMAT_R5G6B5] = ce_mmpfile_storage_size_generic,
	[CE_MMPFILE_FORMAT_A1RGB5] = ce_mmpfile_storage_size_generic,
	[CE_MMPFILE_FORMAT_ARGB4] = ce_mmpfile_storage_size_generic,
	[CE_MMPFILE_FORMAT_ARGB8] = ce_mmpfile_storage_size_generic,
	[CE_MMPFILE_FORMAT_RGB5A1] = ce_mmpfile_storage_size_generic,
	[CE_MMPFILE_FORMAT_RGBA4] = ce_mmpfile_storage_size_generic,
	[CE_MMPFILE_FORMAT_RGBA8] = ce_mmpfile_storage_size_generic,
	[CE_MMPFILE_FORMAT_R8G8B8A8] = ce_mmpfile_storage_size_generic,
	[CE_MMPFILE_FORMAT_YCBCR] = ce_mmpfile_storage_size_ycbcr,
};

size_t ce_mmpfile_storage_size(unsigned int width, unsigned int height,
							unsigned int mipmap_count, ce_mmpfile_format format)
{
	size_t size = 0;
	for (unsigned int i = 0; i < mipmap_count; ++i, width >>= 1, height >>= 1) {
		size += (*ce_mmpfile_storage_size_procs[format])(width, height, format);
	}
	return size;
}

ce_mmpfile* ce_mmpfile_new(unsigned int width, unsigned int height,
	unsigned int mipmap_count, ce_mmpfile_format format, unsigned int user_info)
{
	ce_mmpfile* mmpfile = ce_alloc(sizeof(ce_mmpfile));
	mmpfile->width = width;
	mmpfile->height = height;
	mmpfile->mipmap_count = mipmap_count;
	mmpfile->format = format;
	mmpfile->bit_count = ce_mmpfile_bit_counts[format];
	mmpfile->size = ce_mmpfile_storage_size(width, height, mipmap_count, format);
	mmpfile->user_data_offset = CE_MMPFILE_HEADER_SIZE + mmpfile->size;
	mmpfile->data = ce_alloc(mmpfile->size);
	mmpfile->texels = mmpfile->data;
	mmpfile->version = CE_MMPFILE_VERSION;
	mmpfile->user_info = user_info;
	(*ce_mmpfile_write_header_procs[format])(mmpfile);
	return mmpfile;
}

ce_mmpfile* ce_mmpfile_new_data(void* data, size_t size)
{
	union {
		uint8_t* u8;
		uint32_t* u32;
	} ptr = {data};

	uint32_t signature = ce_le2cpu32(*ptr.u32++);
	assert(CE_MMPFILE_SIGNATURE == signature && "wrong signature");
	ce_unused(signature);

	ce_mmpfile* mmpfile = ce_alloc(sizeof(ce_mmpfile));
	mmpfile->width = ce_le2cpu32(*ptr.u32++);
	mmpfile->height = ce_le2cpu32(*ptr.u32++);
	mmpfile->mipmap_count = ce_le2cpu32(*ptr.u32++);
	mmpfile->format = ce_mmpfile_format_find(ce_le2cpu32(*ptr.u32++));
	mmpfile->bit_count = ce_le2cpu32(*ptr.u32++);
	mmpfile->amask = ce_le2cpu32(*ptr.u32++);
	mmpfile->ashift = ce_le2cpu32(*ptr.u32++);
	mmpfile->acount = ce_le2cpu32(*ptr.u32++);
	mmpfile->rmask = ce_le2cpu32(*ptr.u32++);
	mmpfile->rshift = ce_le2cpu32(*ptr.u32++);
	mmpfile->rcount = ce_le2cpu32(*ptr.u32++);
	mmpfile->gmask = ce_le2cpu32(*ptr.u32++);
	mmpfile->gshift = ce_le2cpu32(*ptr.u32++);
	mmpfile->gcount = ce_le2cpu32(*ptr.u32++);
	mmpfile->bmask = ce_le2cpu32(*ptr.u32++);
	mmpfile->bshift = ce_le2cpu32(*ptr.u32++);
	mmpfile->bcount = ce_le2cpu32(*ptr.u32++);
	mmpfile->user_data_offset = ce_le2cpu32(*ptr.u32++);
	mmpfile->texels = ptr.u8;
	mmpfile->version = 0;
	mmpfile->user_info = 0;
	mmpfile->size = size;
	mmpfile->data = data;
	if (mmpfile->user_data_offset > CE_MMPFILE_HEADER_SIZE) {
		ptr.u8 += mmpfile->user_data_offset - CE_MMPFILE_HEADER_SIZE;
		if (CE_MMPFILE_SIGNATURE_EXT == ce_le2cpu32(*ptr.u32++)) {
			mmpfile->version = ce_le2cpu32(*ptr.u32++);
			mmpfile->user_info = ce_le2cpu32(*ptr.u32);
		}
	}
	return mmpfile;
}

ce_mmpfile* ce_mmpfile_new_memfile(ce_memfile* memfile)
{
	size_t size = ce_memfile_size(memfile);
	void* data = ce_alloc(size);
	ce_memfile_read(memfile, data, 1, size);
	return ce_mmpfile_new_data(data, size);
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

void ce_mmpfile_save(const ce_mmpfile* mmpfile, const char* path)
{
	// compressed PNT3 not supported
	assert(CE_MMPFILE_FORMAT_PNT3 != mmpfile->format);
	if (CE_MMPFILE_FORMAT_PNT3 == mmpfile->format) {
		return;
	}

	FILE* file = fopen(path, "wb");
	if (NULL != file) {
		uint32_t header[22];
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
		header[19] = ce_cpu2le32(CE_MMPFILE_SIGNATURE_EXT);
		header[20] = ce_cpu2le32(mmpfile->version);
		header[21] = ce_cpu2le32(mmpfile->user_info);
		fwrite(header, 4, 19, file);
		fwrite(mmpfile->texels, 1, ce_mmpfile_storage_size(mmpfile->width,
			mmpfile->height, mmpfile->mipmap_count, mmpfile->format), file);
		fwrite(header + 19, 4, 3, file);
		fclose(file);
	}
}

static void ce_mmpfile_argb_swap16_rgba(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	uint16_t* dst = other->texels;
	const uint16_t* src = mmpfile->texels;
	for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		for (const uint16_t* end = src + width * height; src != end; ++src) {
			*dst++ = *src << mmpfile->acount | *src >> mmpfile->ashift;
		}
	}
}

static void ce_mmpfile_argb_swap32_rgba(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	uint32_t* dst = other->texels;
	const uint32_t* src = mmpfile->texels;
	for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		for (const uint32_t* end = src + width * height; src != end; ++src) {
			*dst++ = *src << mmpfile->acount | *src >> mmpfile->ashift;
		}
	}
}

static void ce_mmpfile_unpack16(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	uint8_t* dst = other->texels;
	const uint16_t* src = mmpfile->texels;
	for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		for (const uint16_t* end = src + width * height; src != end; ++src) {
			*dst++ = ((*src & mmpfile->rmask) >> mmpfile->rshift) *
				255 / (mmpfile->rmask >> mmpfile->rshift);
			*dst++ = ((*src & mmpfile->gmask) >> mmpfile->gshift) *
				255 / (mmpfile->gmask >> mmpfile->gshift);
			*dst++ = ((*src & mmpfile->bmask) >> mmpfile->bshift) *
				255 / (mmpfile->bmask >> mmpfile->bshift);
			*dst++ = 0 == mmpfile->amask ? 255 :
				((*src & mmpfile->amask) >> mmpfile->ashift) *
				255 / (mmpfile->amask >> mmpfile->ashift);
		}
	}
}

static void ce_mmpfile_unpack32(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	uint8_t* dst = other->texels;
	const uint32_t* src = mmpfile->texels;
	for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		for (const uint32_t* end = src + width * height; src != end; ++src) {
			*dst++ = ((*src & mmpfile->rmask) >> mmpfile->rshift) *
				255 / (mmpfile->rmask >> mmpfile->rshift);
			*dst++ = ((*src & mmpfile->gmask) >> mmpfile->gshift) *
				255 / (mmpfile->gmask >> mmpfile->gshift);
			*dst++ = ((*src & mmpfile->bmask) >> mmpfile->bshift) *
				255 / (mmpfile->bmask >> mmpfile->bshift);
			*dst++ = 0 == mmpfile->amask ? 255 :
				((*src & mmpfile->amask) >> mmpfile->ashift) *
				255 / (mmpfile->amask >> mmpfile->ashift);
		}
	}
}

static void ce_mmpfile_convert_unknown(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	ce_unused(mmpfile), ce_unused(other);
	assert(false && "not implemented");
}

static void ce_mmpfile_convert_dxt(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	assert(CE_MMPFILE_FORMAT_R8G8B8A8 == other->format && "not implemented");

	uint8_t* rgba = other->texels;
	const uint8_t* blocks = mmpfile->texels;

	int flags = (int[]){squish_kDxt1, squish_kDxt3}[CE_MMPFILE_FORMAT_DXT3 == mmpfile->format];

	for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		squish_DecompressImage(rgba, width, height, blocks, flags);
		rgba += ce_mmpfile_storage_size(width, height, 1, other->format);
		blocks += ce_mmpfile_storage_size(width, height, 1, mmpfile->format);
	}
}

// PNT3 is just a simple RLE (Run-Length Encoding)
static void ce_mmpfile_decompress_pnt3(uint8_t* restrict dst,
								const uint32_t* restrict src, size_t size)
{
	assert(0 == size % sizeof(uint32_t));

	const uint32_t* end = src + size / sizeof(uint32_t);

	size_t n = 0;
	uint32_t v;

	while (src != end) {
		v = ce_le2cpu32(*src++);
		if (v > 1000000U || 0 == v) {
			++n;
		} else {
			memcpy(dst, src - 1 - n, n * sizeof(uint32_t));
			dst += n * sizeof(uint32_t);
			memset(dst, 0, v);
			dst += v;
			n = 0;
		}
	}

	memcpy(dst, src - n, n * sizeof(uint32_t));
}

static void ce_mmpfile_convert_pnt3(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	assert(CE_MMPFILE_FORMAT_ARGB8 == other->format && "not implemented");
	// mipmap_count == compressed size for pnt3
	if (mmpfile->mipmap_count < other->size) { // pnt3 compressed
		ce_mmpfile_decompress_pnt3(other->texels, mmpfile->texels,
													mmpfile->mipmap_count);
	} else {
		memcpy(other->texels, mmpfile->texels, other->size);
	}
}

static void ce_mmpfile_convert_r5g6b5(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	assert(CE_MMPFILE_FORMAT_R8G8B8A8 == other->format && "not implemented");
	ce_mmpfile_unpack16(mmpfile, other);
}

static void ce_mmpfile_convert_a1rgb5(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	if (CE_MMPFILE_FORMAT_RGB5A1 == other->format) {
		ce_mmpfile_argb_swap16_rgba(mmpfile, other);
	} else if (CE_MMPFILE_FORMAT_R8G8B8A8 == other->format) {
		ce_mmpfile_unpack16(mmpfile, other);
	} else {
		assert(false && "not implemented");
	}
}

static void ce_mmpfile_convert_argb4(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	if (CE_MMPFILE_FORMAT_RGBA4 == other->format) {
		ce_mmpfile_argb_swap16_rgba(mmpfile, other);
	} else if (CE_MMPFILE_FORMAT_R8G8B8A8 == other->format) {
		ce_mmpfile_unpack16(mmpfile, other);
	} else {
		assert(false && "not implemented");
	}
}

static void ce_mmpfile_convert_argb8(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	if (CE_MMPFILE_FORMAT_RGBA8 == other->format) {
		ce_mmpfile_argb_swap32_rgba(mmpfile, other);
	} else if (CE_MMPFILE_FORMAT_R8G8B8A8 == other->format) {
		ce_mmpfile_unpack32(mmpfile, other);
	} else {
		assert(false && "not implemented");
	}
}

static void ce_mmpfile_convert_r8g8b8a8(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	assert((CE_MMPFILE_FORMAT_DXT1 == other->format ||
			CE_MMPFILE_FORMAT_DXT3 == other->format) && "not implemented");

	const uint8_t* rgba = mmpfile->texels;
	uint8_t* blocks = other->texels;

	int flags = (int[]){squish_kDxt1, squish_kDxt3}[CE_MMPFILE_FORMAT_DXT3 == other->format];
	flags |= squish_kColourRangeFit;

	for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		squish_CompressImage(rgba, width, height, blocks, flags, NULL);
		rgba += ce_mmpfile_storage_size(width, height, 1, mmpfile->format);
		blocks += ce_mmpfile_storage_size(width, height, 1, other->format);
	}
}

static void ce_mmpfile_convert_ycbcr(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	assert(CE_MMPFILE_FORMAT_R8G8B8A8 == other->format && "not implemented");

	const uint8_t* y_data = mmpfile->texels;
	const uint8_t* cb_data = y_data + mmpfile->width * mmpfile->height;
	const uint8_t* cr_data = cb_data + (mmpfile->width / 2) * (mmpfile->height / 2);

	uint8_t* texels = other->texels;

	for (unsigned int h = 0; h < mmpfile->height; ++h) {
		for (unsigned int w = 0; w < mmpfile->width; ++w) {
			size_t index = 4 * (h * mmpfile->width + w);

			int y = 298 * (y_data[h * mmpfile->width + w] - 16);
			int cb = cb_data[(h / 2) * (mmpfile->width / 2) + w / 2] - 128;
			int cr = cr_data[(h / 2) * (mmpfile->width / 2) + w / 2] - 128;

			texels[index + 0] = ce_clamp(int, (y + 409 * cr + 128) / 256, 0, UCHAR_MAX);
			texels[index + 1] = ce_clamp(int, (y - 100 * cb - 208 * cr + 128) / 256, 0, UCHAR_MAX);
			texels[index + 2] = ce_clamp(int, (y + 516 * cb + 128) / 256, 0, UCHAR_MAX);
			texels[index + 3] = UCHAR_MAX;
		}
	}
}

static void (*ce_mmpfile_convert_procs[CE_MMPFILE_FORMAT_COUNT])
								(const ce_mmpfile*, ce_mmpfile*) = {
	[CE_MMPFILE_FORMAT_UNKNOWN] = ce_mmpfile_convert_unknown,
	[CE_MMPFILE_FORMAT_DXT1] = ce_mmpfile_convert_dxt,
	[CE_MMPFILE_FORMAT_DXT3] = ce_mmpfile_convert_dxt,
	[CE_MMPFILE_FORMAT_PNT3] = ce_mmpfile_convert_pnt3,
	[CE_MMPFILE_FORMAT_R5G6B5] = ce_mmpfile_convert_r5g6b5,
	[CE_MMPFILE_FORMAT_A1RGB5] = ce_mmpfile_convert_a1rgb5,
	[CE_MMPFILE_FORMAT_ARGB4] = ce_mmpfile_convert_argb4,
	[CE_MMPFILE_FORMAT_ARGB8] = ce_mmpfile_convert_argb8,
	[CE_MMPFILE_FORMAT_RGB5A1] = ce_mmpfile_convert_unknown,
	[CE_MMPFILE_FORMAT_RGBA4] = ce_mmpfile_convert_unknown,
	[CE_MMPFILE_FORMAT_RGBA8] = ce_mmpfile_convert_unknown,
	[CE_MMPFILE_FORMAT_R8G8B8A8] = ce_mmpfile_convert_r8g8b8a8,
	[CE_MMPFILE_FORMAT_YCBCR] = ce_mmpfile_convert_ycbcr,
};

void ce_mmpfile_convert(ce_mmpfile* mmpfile, ce_mmpfile_format format)
{
	unsigned int mipmap_count = mmpfile->mipmap_count;

	// special case for pnt3
	if (CE_MMPFILE_FORMAT_PNT3 == mmpfile->format) {
		mipmap_count = 1;
	}

	ce_mmpfile* other = ce_mmpfile_new(mmpfile->width,
		mmpfile->height, mipmap_count, format, mmpfile->user_info);

	(*ce_mmpfile_convert_procs[mmpfile->format])(mmpfile, other);

	ce_mmpfile temp = *mmpfile;
	*mmpfile = *other;
	*other = temp;

	ce_mmpfile_del(other);
}

void ce_mmpfile_convert2(ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	(*ce_mmpfile_convert_procs[mmpfile->format])(mmpfile, other);
}
