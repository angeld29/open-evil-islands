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
 *
 *  DXT code based on:
 *  1. DDS GIMP plugin (compression)
 *     Copyright (C) 2004-2008 Shawn Kirst <skirst@insightbb.com>,
 *     with parts (C) 2003 Arne Reuter <homepage@arnereuter.de>.
 *  2. DDS WIC Codec, squish (decompression)
 *     Copyright (C) 2006 Simon Brown <si@sjbrown.co.uk>
 *     http://code.google.com/p/dds-wic-codec/.
 *
 *  DDS GIMP plugin - fast DXT1 / DXT3 compression
 *  DDS WIC Codec - correct DXT1-alpha / DXT3 decompression
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

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

static void ce_mmpfile_write_header_unknown(ce_mmpfile* mmpfile)
{
	ce_unused(mmpfile);
	assert(false && "not implemented");
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
	mmpfile->amask = 0x0;    mmpfile->ashift = 0;  mmpfile->acount = 0;
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
	[CE_MMPFILE_FORMAT_UNKNOWN] = ce_mmpfile_write_header_unknown,
	[CE_MMPFILE_FORMAT_DXT1] = ce_mmpfile_write_header_dxt1,
	[CE_MMPFILE_FORMAT_DXT3] = ce_mmpfile_write_header_dxt3,
	[CE_MMPFILE_FORMAT_PNT3] = ce_mmpfile_write_header_unknown,
	[CE_MMPFILE_FORMAT_R5G6B5] = ce_mmpfile_write_header_r5g6b5,
	[CE_MMPFILE_FORMAT_A1RGB5] = ce_mmpfile_write_header_a1rgb5,
	[CE_MMPFILE_FORMAT_ARGB4] = ce_mmpfile_write_header_argb4,
	[CE_MMPFILE_FORMAT_ARGB8] = ce_mmpfile_write_header_argb8,
	[CE_MMPFILE_FORMAT_RGB5A1] = ce_mmpfile_write_header_rgb5a1,
	[CE_MMPFILE_FORMAT_RGBA4] = ce_mmpfile_write_header_rgba4,
	[CE_MMPFILE_FORMAT_RGBA8] = ce_mmpfile_write_header_rgba8,
	[CE_MMPFILE_FORMAT_R8G8B8A8] = ce_mmpfile_write_header_r8g8b8a8,
};

size_t ce_mmpfile_storage_size(unsigned int width, unsigned int height,
							unsigned int mipmap_count, unsigned int bit_count)
{
	size_t size = 0;
	for (unsigned int i = 0; i < mipmap_count; ++i, width >>= 1, height >>= 1) {
		size_t sz = bit_count * width * height / 8;
		if (bit_count < 16) {
			// special case for compressed mmp files
			// that have a block of fixed size, for example, 8 for dxt1
			sz = ce_smax(sz, 2 * bit_count);
		}
		size += sz;
	}
	return size;
}

ce_mmpfile* ce_mmpfile_new(unsigned int width, unsigned int height,
	unsigned int mipmap_count, ce_mmpfile_format format, unsigned int user_info)
{
	ce_mmpfile* mmpfile = ce_alloc(sizeof(ce_mmpfile));
	(*ce_mmpfile_write_header_procs[format])(mmpfile);
	mmpfile->width = width;
	mmpfile->height = height;
	mmpfile->mipmap_count = mipmap_count;
	mmpfile->format = format;
	mmpfile->size = ce_mmpfile_storage_size(width, height,
											mipmap_count, mmpfile->bit_count);
	mmpfile->user_data_offset = CE_MMPFILE_HEADER_SIZE + mmpfile->size;
	mmpfile->data = ce_alloc(mmpfile->size);
	mmpfile->texels = mmpfile->data;
	mmpfile->version = CE_MMPFILE_VERSION;
	mmpfile->user_info = user_info;
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
			mmpfile->height, mmpfile->mipmap_count, mmpfile->bit_count), file);
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

static void ce_mmpfile_decompress_dxt(const ce_mmpfile* mmpfile, ce_mmpfile* other);
static void ce_mmpfile_compress_dxt(const ce_mmpfile* mmpfile, ce_mmpfile* other);

static void ce_mmpfile_convert_unknown(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	ce_unused(mmpfile), ce_unused(other);
	assert(false && "not implemented");
}

static void ce_mmpfile_convert_dxt(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	assert(CE_MMPFILE_FORMAT_R8G8B8A8 == other->format && "not implemented");
	ce_mmpfile_decompress_dxt(mmpfile, other);
}

static void ce_mmpfile_decompress_pnt3(uint8_t* restrict dst,
								const uint32_t* restrict src, size_t size)
{
	// RLE (Run-Length Encoding)
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
	if (CE_MMPFILE_FORMAT_DXT1 == other->format ||
			CE_MMPFILE_FORMAT_DXT3 == other->format) {
		ce_mmpfile_compress_dxt(mmpfile, other);
	} else {
		assert(false && "not implemented");
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

// TODO: move DXT code to library

static int Unpack565( uint8_t const* packed, uint8_t* colour )
{
	// build the packed value
	int value = ( int )packed[0] | ( ( int )packed[1] << 8 );

	// get the components in the stored range
	uint8_t red = ( uint8_t )( ( value >> 11 ) & 0x1f );
	uint8_t green = ( uint8_t )( ( value >> 5 ) & 0x3f );
	uint8_t blue = ( uint8_t )( value & 0x1f );

	// scale up to 8 bits
	colour[0] = ( red << 3 ) | ( red >> 2 );
	colour[1] = ( green << 2 ) | ( green >> 4 );
	colour[2] = ( blue << 3 ) | ( blue >> 2 );
	colour[3] = 255;

	return value;
}

static void DecompressColour( uint8_t* rgba, void const* block,
									ce_mmpfile_format format )
{
	// get the block bytes
	uint8_t const* bytes = block;

	// unpack the endpoints
	uint8_t codes[16];
	int a = Unpack565( bytes, codes );
	int b = Unpack565( bytes + 2, codes + 4 );

	// generate the midpoints
	for( int i = 0; i < 3; ++i )
	{
		int c = codes[i];
		int d = codes[4 + i];

		if( CE_MMPFILE_FORMAT_DXT1 == format && a <= b )
		{
			codes[8 + i] = ( uint8_t )( ( c + d )/2 );
			codes[12 + i] = 0;
		}
		else
		{
			codes[8 + i] = ( uint8_t )( ( 2*c + d )/3 );
			codes[12 + i] = ( uint8_t )( ( c + 2*d )/3 );
		}
	}

	// fill in alpha for the intermediate values
	codes[8 + 3] = 255;
	codes[12 + 3] = ( CE_MMPFILE_FORMAT_DXT1 == format && a <= b ) ? 0 : 255;

	// unpack the indices
	uint8_t indices[16];
	for( int i = 0; i < 4; ++i )
	{
		uint8_t* ind = indices + 4*i;
		uint8_t packed = bytes[4 + i];

		ind[0] = packed & 0x3;
		ind[1] = ( packed >> 2 ) & 0x3;
		ind[2] = ( packed >> 4 ) & 0x3;
		ind[3] = ( packed >> 6 ) & 0x3;
	}

	// store out the colours
	for( int i = 0; i < 16; ++i )
	{
		uint8_t offset = 4*indices[i];
		for( int j = 0; j < 4; ++j )
			rgba[4*i + j] = codes[offset + j];
	}
}

static void DecompressAlphaDxt3( uint8_t* rgba, void const* block )
{
	uint8_t const* bytes = block;

	// unpack the alpha values pairwise
	for( int i = 0; i < 8; ++i )
	{
		// quantise down to 4 bits
		uint8_t quant = bytes[i];

		// unpack the values
		uint8_t lo = quant & 0x0f;
		uint8_t hi = quant & 0xf0;

		// convert back up to bytes
		rgba[8*i + 3] = lo | ( lo << 4 );
		rgba[8*i + 7] = hi | ( hi >> 4 );
	}
}

static void Decompress( uint8_t* rgba, void const* block, ce_mmpfile_format format)
{
	void const* colourBlock = block;
	void const* alphaBock = block;

	if (CE_MMPFILE_FORMAT_DXT3 == format) {
		colourBlock = (uint8_t const*)block + 8;
	}

	DecompressColour(rgba, colourBlock, format);

	if (CE_MMPFILE_FORMAT_DXT3 == format) {
		DecompressAlphaDxt3(rgba, alphaBock);
	}
}

// TODO: move DXT code to library

static void ce_mmpfile_decompress_dxt(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	uint8_t* dst = other->texels;
	const uint8_t* src = mmpfile->texels;

	for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		for (unsigned int y = 0; y < height; y += 4) {
			for (unsigned int x = 0; x < width; x += 4) {
				uint8_t targetRgba[4*16];
				Decompress( targetRgba, src, mmpfile->format );
				// write the decompressed pixels to the correct image locations
				uint8_t const* sourcePixel = targetRgba;
				for (int py = 0; py < 4; ++py) {
					for (int px = 0; px < 4; ++px) {
						// get the target location
						unsigned int sx = x + px;
						unsigned int sy = y + py;
						if (sx < width && sy < height) {
							uint8_t* targetPixel = dst + 4*( width*sy + sx );
							// copy the rgba value
							for (int j = 0; j < 4; ++j) {
								*targetPixel++ = *sourcePixel++;
							}
						} else {
							// skip this pixel as its outside the image
							sourcePixel += 4;
						}
					}
				}
				src += 2 * mmpfile->bit_count;
			}
		}
		dst += 4 * width * height;
	}
}

static const unsigned char omatch5[256][2] =
{
   {0x00, 0x00}, {0x00, 0x00}, {0x00, 0x01}, {0x00, 0x01},
   {0x01, 0x00}, {0x01, 0x00}, {0x01, 0x00}, {0x01, 0x01},
   {0x01, 0x01}, {0x01, 0x01}, {0x02, 0x00}, {0x02, 0x00},
   {0x02, 0x00}, {0x02, 0x01}, {0x00, 0x05}, {0x03, 0x00},
   {0x03, 0x00}, {0x03, 0x00}, {0x03, 0x01}, {0x03, 0x01},
   {0x03, 0x01}, {0x03, 0x02}, {0x04, 0x00}, {0x04, 0x00},
   {0x03, 0x03}, {0x04, 0x01}, {0x05, 0x00}, {0x05, 0x00},
   {0x05, 0x00}, {0x05, 0x01}, {0x05, 0x01}, {0x05, 0x01},
   {0x03, 0x06}, {0x06, 0x00}, {0x06, 0x00}, {0x06, 0x01},
   {0x04, 0x05}, {0x07, 0x00}, {0x07, 0x00}, {0x07, 0x00},
   {0x07, 0x01}, {0x07, 0x01}, {0x07, 0x01}, {0x07, 0x02},
   {0x08, 0x00}, {0x08, 0x00}, {0x07, 0x03}, {0x08, 0x01},
   {0x09, 0x00}, {0x09, 0x00}, {0x09, 0x00}, {0x09, 0x01},
   {0x09, 0x01}, {0x09, 0x01}, {0x07, 0x06}, {0x0a, 0x00},
   {0x0a, 0x00}, {0x0a, 0x01}, {0x08, 0x05}, {0x0b, 0x00},
   {0x0b, 0x00}, {0x0b, 0x00}, {0x0b, 0x01}, {0x0b, 0x01},
   {0x0b, 0x01}, {0x0b, 0x02}, {0x0c, 0x00}, {0x0c, 0x00},
   {0x0b, 0x03}, {0x0c, 0x01}, {0x0d, 0x00}, {0x0d, 0x00},
   {0x0d, 0x00}, {0x0d, 0x01}, {0x0d, 0x01}, {0x0d, 0x01},
   {0x0b, 0x06}, {0x0e, 0x00}, {0x0e, 0x00}, {0x0e, 0x01},
   {0x0c, 0x05}, {0x0f, 0x00}, {0x0f, 0x00}, {0x0f, 0x00},
   {0x0f, 0x01}, {0x0f, 0x01}, {0x0f, 0x01}, {0x0f, 0x02},
   {0x10, 0x00}, {0x10, 0x00}, {0x0f, 0x03}, {0x10, 0x01},
   {0x11, 0x00}, {0x11, 0x00}, {0x11, 0x00}, {0x11, 0x01},
   {0x11, 0x01}, {0x11, 0x01}, {0x0f, 0x06}, {0x12, 0x00},
   {0x12, 0x00}, {0x12, 0x01}, {0x10, 0x05}, {0x13, 0x00},
   {0x13, 0x00}, {0x13, 0x00}, {0x13, 0x01}, {0x13, 0x01},
   {0x13, 0x01}, {0x13, 0x02}, {0x14, 0x00}, {0x14, 0x00},
   {0x13, 0x03}, {0x14, 0x01}, {0x15, 0x00}, {0x15, 0x00},
   {0x15, 0x00}, {0x15, 0x01}, {0x15, 0x01}, {0x15, 0x01},
   {0x13, 0x06}, {0x16, 0x00}, {0x16, 0x00}, {0x16, 0x01},
   {0x14, 0x05}, {0x17, 0x00}, {0x17, 0x00}, {0x17, 0x00},
   {0x17, 0x01}, {0x17, 0x01}, {0x17, 0x01}, {0x17, 0x02},
   {0x18, 0x00}, {0x18, 0x00}, {0x17, 0x03}, {0x18, 0x01},
   {0x19, 0x00}, {0x19, 0x00}, {0x19, 0x00}, {0x19, 0x01},
   {0x19, 0x01}, {0x19, 0x01}, {0x17, 0x06}, {0x1a, 0x00},
   {0x1a, 0x00}, {0x1a, 0x01}, {0x18, 0x05}, {0x1b, 0x00},
   {0x1b, 0x00}, {0x1b, 0x00}, {0x1b, 0x01}, {0x1b, 0x01},
   {0x1b, 0x01}, {0x1b, 0x02}, {0x1c, 0x00}, {0x1c, 0x00},
   {0x1b, 0x03}, {0x1c, 0x01}, {0x1d, 0x00}, {0x1d, 0x00},
   {0x1d, 0x00}, {0x1d, 0x01}, {0x1d, 0x01}, {0x1d, 0x01},
   {0x1b, 0x06}, {0x1e, 0x00}, {0x1e, 0x00}, {0x1e, 0x01},
   {0x1c, 0x05}, {0x1f, 0x00}, {0x1f, 0x00}, {0x1f, 0x00},
   {0x1f, 0x01}, {0x1f, 0x01}, {0x1f, 0x01}, {0x1f, 0x02},
   {0x1e, 0x04}, {0x1f, 0x03}, {0x1f, 0x03}, {0x1c, 0x09},
   {0x1f, 0x04}, {0x1f, 0x04}, {0x1f, 0x04}, {0x1f, 0x05},
   {0x1f, 0x05}, {0x1f, 0x05}, {0x1f, 0x06}, {0x1e, 0x08},
   {0x1f, 0x07}, {0x1f, 0x07}, {0x1c, 0x0d}, {0x1f, 0x08},
   {0x1f, 0x08}, {0x1f, 0x08}, {0x1f, 0x09}, {0x1f, 0x09},
   {0x1f, 0x09}, {0x1f, 0x0a}, {0x1e, 0x0c}, {0x1f, 0x0b},
   {0x1f, 0x0b}, {0x1c, 0x11}, {0x1f, 0x0c}, {0x1f, 0x0c},
   {0x1f, 0x0c}, {0x1f, 0x0d}, {0x1f, 0x0d}, {0x1f, 0x0d},
   {0x1f, 0x0e}, {0x1e, 0x10}, {0x1f, 0x0f}, {0x1f, 0x0f},
   {0x1c, 0x15}, {0x1f, 0x10}, {0x1f, 0x10}, {0x1f, 0x10},
   {0x1f, 0x11}, {0x1f, 0x11}, {0x1f, 0x11}, {0x1f, 0x12},
   {0x1e, 0x14}, {0x1f, 0x13}, {0x1f, 0x13}, {0x1c, 0x19},
   {0x1f, 0x14}, {0x1f, 0x14}, {0x1f, 0x14}, {0x1f, 0x15},
   {0x1f, 0x15}, {0x1f, 0x15}, {0x1f, 0x16}, {0x1e, 0x18},
   {0x1f, 0x17}, {0x1f, 0x17}, {0x1c, 0x1d}, {0x1f, 0x18},
   {0x1f, 0x18}, {0x1f, 0x18}, {0x1f, 0x19}, {0x1f, 0x19},
   {0x1f, 0x19}, {0x1f, 0x1a}, {0x1e, 0x1c}, {0x1f, 0x1b},
   {0x1f, 0x1b}, {0x1f, 0x1b}, {0x1f, 0x1c}, {0x1f, 0x1c},
   {0x1f, 0x1c}, {0x1f, 0x1d}, {0x1f, 0x1d}, {0x1f, 0x1d},
   {0x1f, 0x1e}, {0x1f, 0x1e}, {0x1f, 0x1f}, {0x1f, 0x1f},
};

static const unsigned char omatch6[256][2] =
{
   {0x00, 0x00}, {0x00, 0x01}, {0x01, 0x00}, {0x01, 0x00},
   {0x01, 0x01}, {0x02, 0x00}, {0x02, 0x00}, {0x02, 0x01},
   {0x03, 0x00}, {0x03, 0x01}, {0x04, 0x00}, {0x04, 0x00},
   {0x04, 0x01}, {0x05, 0x00}, {0x05, 0x00}, {0x05, 0x01},
   {0x06, 0x00}, {0x06, 0x01}, {0x07, 0x00}, {0x07, 0x00},
   {0x07, 0x01}, {0x08, 0x00}, {0x00, 0x10}, {0x08, 0x01},
   {0x09, 0x00}, {0x09, 0x01}, {0x01, 0x11}, {0x0a, 0x00},
   {0x0a, 0x01}, {0x0b, 0x00}, {0x03, 0x10}, {0x0b, 0x01},
   {0x0c, 0x00}, {0x0c, 0x01}, {0x04, 0x11}, {0x0d, 0x00},
   {0x0d, 0x01}, {0x0e, 0x00}, {0x06, 0x10}, {0x0e, 0x01},
   {0x0f, 0x00}, {0x0f, 0x01}, {0x07, 0x11}, {0x10, 0x00},
   {0x0f, 0x03}, {0x10, 0x01}, {0x11, 0x00}, {0x11, 0x01},
   {0x0f, 0x06}, {0x12, 0x00}, {0x12, 0x01}, {0x13, 0x00},
   {0x0f, 0x09}, {0x13, 0x01}, {0x14, 0x00}, {0x14, 0x01},
   {0x0f, 0x0c}, {0x15, 0x00}, {0x15, 0x01}, {0x16, 0x00},
   {0x0f, 0x0f}, {0x16, 0x01}, {0x17, 0x00}, {0x17, 0x01},
   {0x0f, 0x12}, {0x18, 0x00}, {0x18, 0x01}, {0x19, 0x00},
   {0x11, 0x10}, {0x19, 0x01}, {0x1a, 0x00}, {0x1a, 0x01},
   {0x12, 0x11}, {0x1b, 0x00}, {0x1b, 0x01}, {0x1c, 0x00},
   {0x14, 0x10}, {0x1c, 0x01}, {0x1d, 0x00}, {0x1d, 0x01},
   {0x15, 0x11}, {0x1e, 0x00}, {0x1e, 0x01}, {0x1f, 0x00},
   {0x17, 0x10}, {0x1f, 0x01}, {0x1f, 0x02}, {0x20, 0x00},
   {0x20, 0x01}, {0x21, 0x00}, {0x1f, 0x05}, {0x21, 0x01},
   {0x22, 0x00}, {0x22, 0x01}, {0x1f, 0x08}, {0x23, 0x00},
   {0x23, 0x01}, {0x24, 0x00}, {0x1f, 0x0b}, {0x24, 0x01},
   {0x25, 0x00}, {0x25, 0x01}, {0x1f, 0x0e}, {0x26, 0x00},
   {0x26, 0x01}, {0x27, 0x00}, {0x1f, 0x11}, {0x27, 0x01},
   {0x28, 0x00}, {0x28, 0x01}, {0x20, 0x11}, {0x29, 0x00},
   {0x29, 0x01}, {0x2a, 0x00}, {0x22, 0x10}, {0x2a, 0x01},
   {0x2b, 0x00}, {0x2b, 0x01}, {0x23, 0x11}, {0x2c, 0x00},
   {0x2c, 0x01}, {0x2d, 0x00}, {0x25, 0x10}, {0x2d, 0x01},
   {0x2e, 0x00}, {0x2e, 0x01}, {0x26, 0x11}, {0x2f, 0x00},
   {0x2f, 0x01}, {0x2f, 0x02}, {0x30, 0x00}, {0x30, 0x01},
   {0x2f, 0x04}, {0x31, 0x00}, {0x31, 0x01}, {0x32, 0x00},
   {0x2f, 0x07}, {0x32, 0x01}, {0x33, 0x00}, {0x33, 0x01},
   {0x2f, 0x0a}, {0x34, 0x00}, {0x34, 0x01}, {0x35, 0x00},
   {0x2f, 0x0d}, {0x35, 0x01}, {0x36, 0x00}, {0x36, 0x01},
   {0x2f, 0x10}, {0x37, 0x00}, {0x37, 0x01}, {0x38, 0x00},
   {0x30, 0x10}, {0x38, 0x01}, {0x39, 0x00}, {0x39, 0x01},
   {0x31, 0x11}, {0x3a, 0x00}, {0x3a, 0x01}, {0x3b, 0x00},
   {0x33, 0x10}, {0x3b, 0x01}, {0x3c, 0x00}, {0x3c, 0x01},
   {0x34, 0x11}, {0x3d, 0x00}, {0x3d, 0x01}, {0x3e, 0x00},
   {0x36, 0x10}, {0x3e, 0x01}, {0x3f, 0x00}, {0x3f, 0x01},
   {0x37, 0x11}, {0x3f, 0x02}, {0x3f, 0x03}, {0x3f, 0x04},
   {0x39, 0x10}, {0x3f, 0x05}, {0x3f, 0x06}, {0x3f, 0x07},
   {0x3a, 0x11}, {0x3f, 0x08}, {0x3f, 0x09}, {0x3f, 0x0a},
   {0x3c, 0x10}, {0x3f, 0x0b}, {0x3f, 0x0c}, {0x3f, 0x0d},
   {0x3d, 0x11}, {0x3f, 0x0e}, {0x3f, 0x0f}, {0x36, 0x21},
   {0x3f, 0x10}, {0x3f, 0x11}, {0x3f, 0x12}, {0x38, 0x20},
   {0x3f, 0x13}, {0x3f, 0x14}, {0x3f, 0x15}, {0x39, 0x21},
   {0x3f, 0x16}, {0x3f, 0x17}, {0x3f, 0x18}, {0x3b, 0x20},
   {0x3f, 0x19}, {0x3f, 0x1a}, {0x3f, 0x1b}, {0x3c, 0x21},
   {0x3f, 0x1c}, {0x3f, 0x1d}, {0x3f, 0x1e}, {0x3e, 0x20},
   {0x3f, 0x1f}, {0x3f, 0x20}, {0x37, 0x30}, {0x3f, 0x21},
   {0x3f, 0x22}, {0x3f, 0x23}, {0x38, 0x31}, {0x3f, 0x24},
   {0x3f, 0x25}, {0x3f, 0x26}, {0x3a, 0x30}, {0x3f, 0x27},
   {0x3f, 0x28}, {0x3f, 0x29}, {0x3b, 0x31}, {0x3f, 0x2a},
   {0x3f, 0x2b}, {0x3f, 0x2c}, {0x3d, 0x30}, {0x3f, 0x2d},
   {0x3f, 0x2e}, {0x3f, 0x2f}, {0x3e, 0x31}, {0x3f, 0x30},
   {0x3f, 0x31}, {0x3f, 0x31}, {0x3f, 0x32}, {0x3f, 0x33},
   {0x3f, 0x34}, {0x3f, 0x34}, {0x3f, 0x35}, {0x3f, 0x36},
   {0x3f, 0x37}, {0x3f, 0x37}, {0x3f, 0x38}, {0x3f, 0x39},
   {0x3f, 0x3a}, {0x3f, 0x3a}, {0x3f, 0x3b}, {0x3f, 0x3c},
   {0x3f, 0x3d}, {0x3f, 0x3d}, {0x3f, 0x3e}, {0x3f, 0x3f},
};

static int color_distance(const unsigned char *c0,
                          const unsigned char *c1)
{
   return(((c0[0] - c1[0]) * (c0[0] - c1[0])) +
          ((c0[1] - c1[1]) * (c0[1] - c1[1])) +
          ((c0[2] - c1[2]) * (c0[2] - c1[2])));
}

/* pack BGR8 to RGB565 */
static unsigned short pack_rgb565(const unsigned char *c)
{
   return(((c[2] >> 3) << 11) | ((c[1] >> 2) << 5) | (c[0] >> 3));
}

/* unpack RGB565 to BGR */
static void unpack_rgb565(unsigned char *dst, unsigned short v)
{
   int r = (v >> 11) & 0x1f;
   int g = (v >>  5) & 0x3f;
   int b = (v      ) & 0x1f;

   dst[0] = (b << 3) | (b >> 2);
   dst[1] = (g << 2) | (g >> 4);
   dst[2] = (r << 3) | (r >> 2);
}

static int mul8bit(int a, int b)
{
   int t = a * b + 128;
   return((t + (t >> 8)) >> 8);
}

static int blerp(int a, int b, int x)
{
   return(a + mul8bit(b - a, x));
}

static void lerp_rgb(unsigned char *dst, unsigned char *a, unsigned char *b, int f)
{
   dst[0] = blerp(a[0], b[0], f);
   dst[1] = blerp(a[1], b[1], f);
   dst[2] = blerp(a[2], b[2], f);
}

static void eval_colors(unsigned char color[4][3],
                        unsigned short c0, unsigned short c1)
{
   unpack_rgb565(color[0], c0);
   unpack_rgb565(color[1], c1);
   if(c0 > c1)
   {
      lerp_rgb(color[2], color[0], color[1], 0x55);
      lerp_rgb(color[3], color[0], color[1], 0xaa);
   }
   else
   {
      color[2][0] = (color[0][0] + color[1][0]) >> 1;
      color[2][1] = (color[0][1] + color[1][1]) >> 1;
      color[2][2] = (color[0][2] + color[1][2]) >> 1;

      color[3][0] = color[3][1] = color[3][2] = 0;
   }
}

/* extract 4x4 BGRA block from 4x4 RGBA source */
static void extract_block(const unsigned char *src, int x, int y,
                          int w, int h, unsigned char *block)
{
   int i, j;
   int bw = ce_min(w - x, 4);
   int bh = ce_min(h - y, 4);
   int bx, by;
   const int rem[] =
   {
      0, 0, 0, 0,
      0, 1, 0, 1,
      0, 1, 2, 0,
      0, 1, 2, 3
   };

   for(i = 0; i < 4; ++i)
   {
      by = rem[(bh - 1) * 4 + i] + y;
      for(j = 0; j < 4; ++j)
      {
         bx = rem[(bw - 1) * 4 + j] + x;
         block[(i * 4 * 4) + (j * 4) + 0] =
            src[(by * (w * 4)) + (bx * 4) + 2];
         block[(i * 4 * 4) + (j * 4) + 1] =
            src[(by * (w * 4)) + (bx * 4) + 1];
         block[(i * 4 * 4) + (j * 4) + 2] =
            src[(by * (w * 4)) + (bx * 4) + 0];
         block[(i * 4 * 4) + (j * 4) + 3] =
            src[(by * (w * 4)) + (bx * 4) + 3];
      }
   }
}

/* Color matching function */
static unsigned int match_colors_block(const unsigned char *block,
                                       unsigned char color[4][3])
{
   unsigned int mask = 0;
   int dirb = color[0][0] - color[1][0];
   int dirg = color[0][1] - color[1][1];
   int dirr = color[0][2] - color[1][2];
   int dots[16], stops[4];
   int c0pt, halfpt, c3pt, dot;
   int i;

   for(i = 0; i < 16; ++i)
      dots[i] = block[4 * i] * dirb + block[4 * i + 1] * dirg + block[4 * i + 2] * dirr;

   for(i = 0; i < 4; ++i)
      stops[i] = color[i][0] * dirb + color[i][1] * dirg + color[i][2] * dirr;

   c0pt = (stops[1] + stops[3]) >> 1;
   halfpt = (stops[3] + stops[2]) >> 1;
   c3pt = (stops[2] + stops[0]) >> 1;

	/* the version without dithering is straight-forward */
	for(i = 15; i >= 0; --i)
      {
         mask <<= 2;
         dot = dots[i];

         if(dot < halfpt)
            mask |= (dot < c0pt) ? 1 : 3;
         else
            mask |= (dot < c3pt) ? 2 : 0;
      }

   return(mask);
}

/* Special case color matching for DXT1 color blocks with non-opaque
 * alpha values.  Simple distance based color matching.  This is my
 * little hack, Fabian had no need for DXT1-alpha :)
 */
static unsigned int match_colors_block_DXT1alpha(const unsigned char *block,
                                                 unsigned char color[4][3])
{
   int i, d0, d1, d2, idx;
   unsigned int mask = 0;

   for(i = 15; i >= 0; --i)
   {
      mask <<= 2;
      d0 = color_distance(&block[4 * i], color[0]);
      d1 = color_distance(&block[4 * i], color[1]);
      d2 = color_distance(&block[4 * i], color[2]);
      if(block[4 * i + 3] < 128)
         idx = 3;
      else if(d0 < d1 && d0 < d2)
         idx = 0;
      else if(d1 < d2)
         idx = 1;
      else
         idx = 2;
      mask |= idx;
   }

   return(mask);
}

/* The color optimization function. (Clever code, part 1) */
static void optimize_colors_block(const unsigned char *block,
									unsigned short *max16,
									unsigned short *min16)
{
   const int niterpow = 4;

   int mu[3], mn[3], mx[3];
   int i, c, r, g, b, dot, iter;
   int muv, mnv, mxv, mnd, mxd;
   int cov[6];
   unsigned char *bp, mnc[3], mxc[3];
   float covf[6], vfr, vfg, vfb, magn;
   float fr, fg, fb;

   /* determine color distribution */
   for(c = 0; c < 3; ++c)
   {
      bp = (unsigned char *)block + c;

      muv = mnv = mxv = bp[0];
      for(i = 4; i < 64; i += 4)
      {
         muv += bp[i];
         if(mnv > bp[i]) mnv = bp[i];
         if(mxv < bp[i]) mxv = bp[i];
      }

      mu[c] = (muv + 8) >> 4;
      mn[c] = mnv;
      mx[c] = mxv;
   }

   memset(cov, 0, sizeof(cov));

   /* determine covariance matrix */
   for(i = 0; i < 16; ++i)
   {
      b = block[4 * i + 0] - mu[0];
      g = block[4 * i + 1] - mu[1];
      r = block[4 * i + 2] - mu[2];

      cov[0] += r * r;
      cov[1] += r * g;
      cov[2] += r * b;
      cov[3] += g * g;
      cov[4] += g * b;
      cov[5] += b * b;
   }

   /* convert covariance matrix to float, find principal axis via power iter */
   for(i = 0; i < 6; ++i)
      covf[i] = cov[i] / 255.0f;

   vfb = mx[0] - mn[0];
   vfg = mx[1] - mn[1];
   vfr = mx[2] - mn[2];

   for(iter = 0; iter < niterpow; ++iter)
   {
      fr = vfr * covf[0] + vfg * covf[1] + vfb * covf[2];
      fg = vfr * covf[1] + vfg * covf[3] + vfb * covf[4];
      fb = vfr * covf[2] + vfg * covf[4] + vfb * covf[5];

      vfr = fr;
      vfg = fg;
      vfb = fb;
   }

   vfr = fabsf(vfr);
   vfg = fabsf(vfg);
   vfb = fabsf(vfb);

   magn = fmaxf(fmaxf(vfr, vfg), vfb);

   if(magn < 4.0) /* too small, default to luminance */
   {
      r = 148;
      g = 300;
      b = 58;
   }
   else
   {
      magn = 512.0f / magn;
      r = (int)(vfr * magn);
      g = (int)(vfg * magn);
      b = (int)(vfb * magn);
   }

   /* pick colors at extreme points */
   mnd =  0x7fffffff;
   mxd = -0x7fffffff;

   for(i = 0; i < 16; ++i)
   {
      dot = block[4 * i] * b + block[4 * i + 1] * g + block[4 * i + 2] * r;

      if(dot < mnd)
      {
         mnd = dot;
         memcpy(mnc, &block[4 * i], 3);
      }
      if(dot > mxd)
      {
         mxd = dot;
         memcpy(mxc, &block[4 * i], 3);
      }
   }

   /* reduce to 16-bit colors */
   *max16 = pack_rgb565(mxc);
   *min16 = pack_rgb565(mnc);
}

/* The refinement function (Clever code, part 2)
 * Tries to optimize colors to suit block contents better.
 * (By solving a least squares system via normal equations + Cramer's rule)
 */
static int refine_block(const unsigned char *block,
                        unsigned short *max16, unsigned short *min16,
                        unsigned int mask)
{
   const int w1tab[4] = {3, 0, 2, 1};
   const int prods[4] = {0x090000, 0x000900, 0x040102, 0x010402};
   /* ^ Some magic to save a lot of multiplies in the accumulating loop... */

   int akku = 0;
   int At1_r, At1_g, At1_b;
   int At2_r, At2_g, At2_b;
   unsigned int cm = mask;
   int i, step, w1, r, g, b;
   int xx, yy, xy;
   float frb, fg;
   unsigned short v, oldmin, oldmax;
   int s;

   oldmin = *min16;
   oldmax = *max16;
   if((mask ^ (mask << 2)) < 4) /* all pixels have the same index */
   {
      /* degenerate system, use optimal single-color match for average color */
      r = g = b = 8;
      for(i = 0; i < 16; ++i)
      {
         r += block[4 * i + 2];
         g += block[4 * i + 1];
         b += block[4 * i + 0];
      }

      r >>= 4;
      g >>= 4;
      b >>= 4;

      *max16 = (omatch5[r][0] << 11) | (omatch6[g][0] << 5) | omatch5[b][0];
      *min16 = (omatch5[r][1] << 11) | (omatch6[g][1] << 5) | omatch5[b][1];
      return(*min16 != oldmin || *max16 != oldmax);
   }

   At1_r = At1_g = At1_b = 0;
   At2_r = At2_g = At2_b = 0;

   for(i = 0; i < 16; ++i, cm >>= 2)
   {
      step = cm & 3;
      w1 = w1tab[step];
      r = block[4 * i + 2];
      g = block[4 * i + 1];
      b = block[4 * i + 0];

      akku  += prods[step];
      At1_r += w1 * r;
      At1_g += w1 * g;
      At1_b += w1 * b;
      At2_r += r;
      At2_g += g;
      At2_b += b;
   }

   At2_r = 3 * At2_r - At1_r;
   At2_g = 3 * At2_g - At1_g;
   At2_b = 3 * At2_b - At1_b;

   /* extract solutions and decide solvability */
   xx = akku >> 16;
   yy = (akku >> 8) & 0xff;
   xy = (akku >> 0) & 0xff;

   frb = 3.0f * 31.0f / 255.0f / (xx * yy - xy * xy);
   fg = frb * 63.0f / 31.0f;

   /* solve */
   s = (int)((At1_r * yy - At2_r * xy) * frb + 0.5f);
   if(s < 0) s = 0;
   if(s > 31) s = 31;
   v = s << 11;
   s = (int)((At1_g * yy - At2_g * xy) * fg + 0.5f);
   if(s < 0) s = 0;
   if(s > 63) s = 63;
   v |= s << 5;
   s = (int)((At1_b * yy - At2_b * xy) * frb + 0.5f);
   if(s < 0) s = 0;
   if(s > 31) s = 31;
   v |= s;
   *max16 = v;

   s = (int)((At2_r * xx - At1_r * xy) * frb + 0.5f);
   if(s < 0) s = 0;
   if(s > 31) s = 31;
   v = s << 11;
   s = (int)((At2_g * xx - At1_g * xy) * fg + 0.5f);
   if(s < 0) s = 0;
   if(s > 63) s = 63;
   v |= s << 5;
   s = (int)((At2_b * xx - At1_b * xy) * frb + 0.5f);
   if(s < 0) s = 0;
   if(s > 31) s = 31;
   v |= s;
   *min16 = v;

   return(oldmin != *min16 || oldmax != *max16);
}

#define GETL32(buf) \
   (((unsigned int)(buf)[0]      ) | \
    ((unsigned int)(buf)[1] <<  8) | \
    ((unsigned int)(buf)[2] << 16) | \
    ((unsigned int)(buf)[3] << 24))

#define PUTL16(buf, s) \
   (buf)[0] = ((s)     ) & 0xff; \
   (buf)[1] = ((s) >> 8) & 0xff;

#define PUTL32(buf, l) \
   (buf)[0] = ((l)      ) & 0xff; \
	(buf)[1] = ((l) >>  8) & 0xff; \
	(buf)[2] = ((l) >> 16) & 0xff; \
	(buf)[3] = ((l) >> 24) & 0xff;

static void encode_color_block(unsigned char *dst,
                               const unsigned char *block,
								ce_mmpfile_format format)
{
   unsigned char color[4][3];
   unsigned short min16, max16;
   unsigned int v, mn, mx, mask;
   int i, block_has_alpha = 0;

   /* find min/max colors, determine if alpha values present in block
    * (for DXT1-alpha)
    */
   mn = mx = GETL32(block);
   for(i = 0; i < 16; ++i)
   {
      block_has_alpha = block_has_alpha || (block[4 * i + 3] < 255);
      v = GETL32(&block[4 * i]);
      mx = mx > v ? mx : v;
      mn = mn < v ? mn : v;
   }

   if(mn != mx) /* block is not a solid color, continue with compression */
   {
      /* pca + map along principal axis */
      optimize_colors_block(block, &max16, &min16);
      if(max16 != min16)
      {
         eval_colors(color, max16, min16);
         mask = match_colors_block(block, color);
      }
      else
         mask = 0;

      /* refine */
      refine_block(block, &max16, &min16, mask);

      if(max16 != min16)
      {
         eval_colors(color, max16, min16);
         mask = match_colors_block(block, color);
      }
      else
         mask = 0;
   }
   else /* constant color */
   {
      mask = 0xaaaaaaaa;
      max16 = (omatch5[block[2]][0] << 11) |
              (omatch6[block[1]][0] <<  5) |
              (omatch5[block[0]][0]      );
      min16 = (omatch5[block[2]][1] << 11) |
              (omatch6[block[1]][1] <<  5) |
              (omatch5[block[0]][1]      );
   }

   /* HACK! for DXT1 blocks which have non-opaque pixels */
   if(CE_MMPFILE_FORMAT_DXT1 == format && block_has_alpha)
   {
      if(max16 > min16)
      {
         max16 ^= min16; min16 ^= max16; max16 ^= min16;
      }
      eval_colors(color, max16, min16);
      mask = match_colors_block_DXT1alpha(block, color);
   }

   if(max16 < min16 && !(CE_MMPFILE_FORMAT_DXT1 == format && block_has_alpha))
   {
      max16 ^= min16; min16 ^= max16; max16 ^= min16;
      mask ^= 0x55555555;
   }

   PUTL16(&dst[0], max16);
   PUTL16(&dst[2], min16);
   PUTL32(&dst[4], mask);
}

static void encode_alpha_block_DXT3(unsigned char *dst,
                                    const unsigned char *block)
{
   int i, a1, a2;

   block += 3;

   for(i = 0; i < 8; ++i)
   {
      a1 = block[8 * i + 0];
      a2 = block[8 * i + 4];
      *dst++ = ((a2 >> 4) << 4) | (a1 >> 4);
   }
}

// TODO: move DXT code to library

static void ce_mmpfile_compress_dxt(const ce_mmpfile* mmpfile, ce_mmpfile* other)
{
	assert(0 == (mmpfile->width & 3) && 0 == (mmpfile->height & 3)); // is mul4

	uint8_t* dst = other->texels;
	const uint8_t* src = mmpfile->texels;

	unsigned char block[64];

	for (unsigned int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		for (unsigned int y = 0; y < height; y += 4) {
			for (unsigned int x = 0; x < width; x += 4) {
				extract_block(src, x, y, width, height, block);
				if (CE_MMPFILE_FORMAT_DXT3 == other->format) {
					encode_alpha_block_DXT3(dst, block);
					dst += 8;
				}
				encode_color_block(dst, block, other->format);
				dst += 8;
			}
		}
		src += 4 * width * height;
	}
}
