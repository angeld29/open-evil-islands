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
 *  DXT code based on:
 *  1. DDS WIC Codec, squish
 *     Copyright (C) 2006 Simon Brown <si@sjbrown.co.uk>
 *     http://code.google.com/p/dds-wic-codec/
*/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <assert.h>

#include "celib.h"
#include "cevec3.h"
#include "cebyteorder.h"
#include "cealloc.h"
#include "cemmphlp.h"

static void ce_mmphlp_decompress_pnt3(void* restrict dst,
										const void* restrict src, int size)
{
	assert(0 == size % sizeof(uint32_t));

	uint8_t* d = dst;

	const uint32_t* s = src;
	const uint32_t* e = s + size / sizeof(uint32_t);

	int n = 0;
	uint32_t v;

	while (s != e) {
		v = ce_le2cpu32(*s++);
		if (v > 1000000 || 0 == v) {
			++n;
		} else {
			memcpy(d, s - 1 - n, n * sizeof(uint32_t));
			d += n * sizeof(uint32_t);
			n = 0;
			memset(d, '\0', v);
			d += v;
		}
	}

	memcpy(d, s - n, n * sizeof(uint32_t));
}

void ce_mmphlp_pnt3_morph_argb8(ce_mmpfile* mmpfile)
{
	assert(CE_MMPFILE_FORMAT_PNT3 == mmpfile->format);

	int size = 4 * mmpfile->width * mmpfile->height;

	// mipmap_count == compressed size for pnt3, see doc/formats/mmpfile.txt
	if (mmpfile->mipmap_count < size) { // pnt3 compressed
		void* data = ce_alloc(size);

		ce_mmphlp_decompress_pnt3(data, mmpfile->texels, mmpfile->mipmap_count);

		ce_free(mmpfile->data, mmpfile->size);

		mmpfile->texels = data;
		mmpfile->size = size;
		mmpfile->data = data;
	}

	mmpfile->mipmap_count = 1;
	mmpfile->format = CE_MMPFILE_FORMAT_ARGB8;

	// TODO: write full argb8 header
}

static void ce_mmphlp_argb_swap_rgba(ce_mmpfile* mmpfile,
										int rgbshift, int ashift)
{
	uint16_t* dst = mmpfile->texels;
	const uint16_t* src = mmpfile->texels;

	for (int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		for (const uint16_t* end = src + width * height; src != end; ++src) {
			*dst++ = *src << rgbshift | *src >> ashift;
		}
	}

	mmpfile->format = CE_MMPFILE_FORMAT_INVALID;
}

void ce_mmphlp_a1rgb5_swap_rgb5a1(ce_mmpfile* mmpfile)
{
	assert(mmpfile->format == CE_MMPFILE_FORMAT_A1RGB5);
	ce_mmphlp_argb_swap_rgba(mmpfile, 1, 15);
}

void ce_mmphlp_argb4_swap_rgba4(ce_mmpfile* mmpfile)
{
	assert(mmpfile->format == CE_MMPFILE_FORMAT_ARGB4);
	ce_mmphlp_argb_swap_rgba(mmpfile, 4, 12);
}

void ce_mmphlp_argb8_swap_rgba8(ce_mmpfile* mmpfile)
{
	assert(mmpfile->format == CE_MMPFILE_FORMAT_ARGB8);

	uint32_t* dst = mmpfile->texels;
	const uint32_t* src = mmpfile->texels;

	for (int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		for (const uint32_t* end = src + width * height; src != end; ++src) {
			*dst++ = *src << 8 | *src >> 24;
		}
	}

	mmpfile->format = CE_MMPFILE_FORMAT_INVALID;
}

static void ce_mmphlp_argb_unpack_rgba(ce_mmpfile* mmpfile,
								uint16_t rmask, uint16_t gmask, uint16_t bmask,
								int rshift, int gshift, int ashift,
								int rdiv, int gdiv, int bdiv, int adiv)
{
	int size = ce_mmphlp_storage_requirements_rgba8(mmpfile->width,
													mmpfile->height,
													mmpfile->mipmap_count);
	void* data = ce_alloc(size);

	uint8_t* dst = data;
	const uint16_t* src = mmpfile->texels;

	for (int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		for (const uint16_t* end = src + width * height; src != end; ++src) {
			*dst++ = ((*src & rmask) >> rshift) * 255 / rdiv;
			*dst++ = ((*src & gmask) >> gshift) * 255 / gdiv;
			*dst++ = (*src & bmask) * 255 / bdiv;
			*dst++ = 0 != adiv ? (*src >> ashift) * 255 / adiv : 255;
		}
	}

	ce_free(mmpfile->data, mmpfile->size);

	mmpfile->format = CE_MMPFILE_FORMAT_INVALID;
	mmpfile->texels = data;
	mmpfile->size = size;
	mmpfile->data = data;
}

void ce_mmphlp_r5g6b5_unpack_rgba8(ce_mmpfile* mmpfile)
{
	assert(mmpfile->format == CE_MMPFILE_FORMAT_R5G6B5);
	ce_mmphlp_argb_unpack_rgba(mmpfile, 0xf800, 0x7e0, 0x1f,
										11, 5, 0, 31, 63, 31, 0);
}

void ce_mmphlp_a1rgb5_unpack_rgba8(ce_mmpfile* mmpfile)
{
	assert(mmpfile->format == CE_MMPFILE_FORMAT_A1RGB5);
	ce_mmphlp_argb_unpack_rgba(mmpfile, 0x7c00, 0x3e0, 0x1f,
										10, 5, 15, 31, 31, 31, 1);
}

void ce_mmphlp_argb4_unpack_rgba8(ce_mmpfile* mmpfile)
{
	assert(mmpfile->format == CE_MMPFILE_FORMAT_ARGB4);
	ce_mmphlp_argb_unpack_rgba(mmpfile, 0xf00, 0xf0, 0xf,
										8, 4, 12, 15, 15, 15, 15);
}

void ce_mmphlp_argb8_unpack_rgba8(ce_mmpfile* mmpfile)
{
	assert(mmpfile->format == CE_MMPFILE_FORMAT_ARGB8);

	uint32_t* dst = mmpfile->texels;
	const uint32_t* src = mmpfile->texels;

	union {
		uint8_t u8[4];
		uint32_t u32;
	} tmp;

	for (int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		for (const uint32_t* end = src + width * height; src != end; ++src) {
			tmp.u8[0] = (*src & 0xff0000) >> 16;
			tmp.u8[1] = (*src & 0xff00) >> 8;
			tmp.u8[2] = *src & 0xff;
			tmp.u8[3] = *src >> 24;
			*dst++ = tmp.u32;
		}
	}

	mmpfile->format = CE_MMPFILE_FORMAT_INVALID;
}

int ce_mmphlp_storage_requirements_rgba8(int width, int height,
										int mipmap_count)
{
	int size = 0;
	for (int i = 0; i < mipmap_count; ++i, width >>= 1, height >>= 1) {
		size += 4 * width * height;
	}
	return size;
}

int ce_mmphlp_storage_requirements_dxt(int width, int height,
									int mipmap_count, int format)
{
	assert(CE_MMPFILE_FORMAT_DXT1 == format ||
			CE_MMPFILE_FORMAT_DXT3 == format);
	int size = 0;
	for (int i = 0; i < mipmap_count; ++i, width >>= 1, height >>= 1) {
		int block_size = CE_MMPFILE_FORMAT_DXT1 == format ? 8 : 16;
		int block_count = ((width + 3) >> 2) * ((height + 3) >> 2);
		size += block_size * block_count;
	}
	return size;
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

#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#define MAX(a, b)  ((a) > (b) ? (a) : (b))

static const unsigned char quantRB[256 + 16] =
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
   0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 
   0x08, 0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 
   0x10, 0x10, 0x10, 0x10, 0x10, 0x18, 0x18, 0x18, 
   0x18, 0x18, 0x18, 0x18, 0x18, 0x21, 0x21, 0x21, 
   0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x29, 0x29, 
   0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x31, 0x31, 
   0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x39, 0x39, 
   0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x42, 0x42, 
   0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x4a, 0x4a, 
   0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x52, 
   0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x5a, 
   0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x63, 
   0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x6b, 
   0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 
   0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 
   0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 
   0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 
   0x8c, 0x8c, 0x8c, 0x8c, 0x8c, 0x8c, 0x8c, 0x8c, 
   0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 
   0x94, 0x9c, 0x9c, 0x9c, 0x9c, 0x9c, 0x9c, 0x9c, 
   0x9c, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 
   0xa5, 0xad, 0xad, 0xad, 0xad, 0xad, 0xad, 0xad, 
   0xad, 0xb5, 0xb5, 0xb5, 0xb5, 0xb5, 0xb5, 0xb5, 
   0xb5, 0xb5, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 
   0xbd, 0xbd, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 
   0xc6, 0xc6, 0xce, 0xce, 0xce, 0xce, 0xce, 0xce, 
   0xce, 0xce, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 
   0xd6, 0xd6, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 
   0xde, 0xde, 0xde, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 
   0xe7, 0xe7, 0xe7, 0xef, 0xef, 0xef, 0xef, 0xef, 
   0xef, 0xef, 0xef, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 
   0xf7, 0xf7, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
};

static const unsigned char quantG[256 + 16] =
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
   0x00, 0x00, 0x00, 0x04, 0x04, 0x04, 0x04, 0x08, 
   0x08, 0x08, 0x08, 0x0c, 0x0c, 0x0c, 0x0c, 0x10, 
   0x10, 0x10, 0x10, 0x14, 0x14, 0x14, 0x14, 0x18, 
   0x18, 0x18, 0x18, 0x1c, 0x1c, 0x1c, 0x1c, 0x20, 
   0x20, 0x20, 0x20, 0x24, 0x24, 0x24, 0x24, 0x28, 
   0x28, 0x28, 0x28, 0x2c, 0x2c, 0x2c, 0x2c, 0x30, 
   0x30, 0x30, 0x30, 0x34, 0x34, 0x34, 0x34, 0x38, 
   0x38, 0x38, 0x38, 0x3c, 0x3c, 0x3c, 0x3c, 0x41, 
   0x41, 0x41, 0x41, 0x45, 0x45, 0x45, 0x45, 0x49, 
   0x49, 0x49, 0x49, 0x4d, 0x4d, 0x4d, 0x4d, 0x51, 
   0x51, 0x51, 0x51, 0x55, 0x55, 0x55, 0x55, 0x55, 
   0x59, 0x59, 0x59, 0x59, 0x5d, 0x5d, 0x5d, 0x5d, 
   0x61, 0x61, 0x61, 0x61, 0x65, 0x65, 0x65, 0x65, 
   0x69, 0x69, 0x69, 0x69, 0x6d, 0x6d, 0x6d, 0x6d, 
   0x71, 0x71, 0x71, 0x71, 0x75, 0x75, 0x75, 0x75, 
   0x79, 0x79, 0x79, 0x79, 0x7d, 0x7d, 0x7d, 0x7d, 
   0x82, 0x82, 0x82, 0x82, 0x86, 0x86, 0x86, 0x86, 
   0x8a, 0x8a, 0x8a, 0x8a, 0x8e, 0x8e, 0x8e, 0x8e, 
   0x92, 0x92, 0x92, 0x92, 0x96, 0x96, 0x96, 0x96, 
   0x9a, 0x9a, 0x9a, 0x9a, 0x9e, 0x9e, 0x9e, 0x9e, 
   0xa2, 0xa2, 0xa2, 0xa2, 0xa6, 0xa6, 0xa6, 0xa6, 
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xae, 0xae, 0xae, 
   0xae, 0xb2, 0xb2, 0xb2, 0xb2, 0xb6, 0xb6, 0xb6, 
   0xb6, 0xba, 0xba, 0xba, 0xba, 0xbe, 0xbe, 0xbe, 
   0xbe, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7, 0xc7, 0xc7, 
   0xc7, 0xcb, 0xcb, 0xcb, 0xcb, 0xcf, 0xcf, 0xcf, 
   0xcf, 0xd3, 0xd3, 0xd3, 0xd3, 0xd7, 0xd7, 0xd7, 
   0xd7, 0xdb, 0xdb, 0xdb, 0xdb, 0xdf, 0xdf, 0xdf, 
   0xdf, 0xe3, 0xe3, 0xe3, 0xe3, 0xe7, 0xe7, 0xe7, 
   0xe7, 0xeb, 0xeb, 0xeb, 0xeb, 0xef, 0xef, 0xef, 
   0xef, 0xf3, 0xf3, 0xf3, 0xf3, 0xf7, 0xf7, 0xf7, 
   0xf7, 0xfb, 0xfb, 0xfb, 0xfb, 0xff, 0xff, 0xff, 
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
};

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

/* extract 4x4 BGRA block */
static void extract_block(const unsigned char *src, int x, int y,
                          int w, int h, unsigned char *block)
{
   int i, j;
   int bw = MIN(w - x, 4);
   int bh = MIN(h - y, 4);
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
            src[(by * (w * 4)) + (bx * 4) + 0];
         block[(i * 4 * 4) + (j * 4) + 1] =
            src[(by * (w * 4)) + (bx * 4) + 1];
         block[(i * 4 * 4) + (j * 4) + 2] =
            src[(by * (w * 4)) + (bx * 4) + 2];
         block[(i * 4 * 4) + (j * 4) + 3] =
            src[(by * (w * 4)) + (bx * 4) + 3];
      }
   }
}

/* Block dithering function.  Simply dithers a block to 565 RGB.
 * (Floyd-Steinberg)
 */
static void dither_block(unsigned char *dst, const unsigned char *block)
{
   int err[8], *ep1 = err, *ep2 = err + 4, *tmp;
   int c, y;
   unsigned char *bp, *dp;
   const unsigned char *quant;
  
   /* process channels seperately */
   for(c = 0; c < 3; ++c)
   {
      bp = (unsigned char *)block;
      dp = dst;
      quant = (c == 1) ? quantG + 8 : quantRB + 8;
      
      bp += c;
      dp += c;
      
      memset(err, 0, sizeof(err));
      
      for(y = 0; y < 4; ++y)
      {
         /* pixel 0 */
         dp[ 0] = quant[bp[ 0] + ((3 * ep2[1] + 5 * ep2[0]) >> 4)];
         ep1[0] = bp[ 0] - dp[ 0];
         
         /* pixel 1 */
         dp[ 4] = quant[bp[ 4] + ((7 * ep1[0] + 3 * ep2[2] + 5 * ep2[1] + ep2[0]) >> 4)];
         ep1[1] = bp[ 4] - dp[ 4];
         
         /* pixel 2 */
         dp[ 8] = quant[bp[ 8] + ((7 * ep1[1] + 3 * ep2[3] + 5 * ep2[2] + ep2[1]) >> 4)];
         ep1[2] = bp[ 8] - dp[ 8];
         
         /* pixel 3 */
         dp[12] = quant[bp[12] + ((7 * ep1[2] + 5 * ep2[3] + ep2[2]) >> 4)];
         ep1[3] = bp[12] - dp[12];
         
         /* advance to next line */
         tmp = ep1;
         ep1 = ep2;
         ep2 = tmp;
         
         bp += 16;
         dp += 16;
      }
   }
}

/* Color matching function */
static unsigned int match_colors_block(const unsigned char *block,
                                       unsigned char color[4][3],
                                       int dither)
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
   
   if(!dither)
   {
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
   }
   else
   {
      /* with floyd-steinberg dithering (see above) */
      int err[8], *ep1 = err, *ep2 = err + 4, *tmp;
      int *dp = dots, y, lmask, step;
      
      c0pt <<= 4;
      halfpt <<= 4;
      c3pt <<= 4;
      
      memset(err, 0, sizeof(err));
      
      for(y = 0; y < 4; ++y)
      {
         /* pixel 0 */
         dot = (dp[0] << 4) + (3 * ep2[1] + 5 * ep2[0]);
         if(dot < halfpt)
            step = (dot < c0pt) ? 1 : 3;
         else
            step = (dot < c3pt) ? 2 : 0;

         ep1[0] = dp[0] - stops[step];
         lmask = step;
         
         /* pixel 1 */
         dot = (dp[1] << 4) + (7 * ep1[0] + 3 * ep2[2] + 5 * ep2[1] + ep2[0]);
         if(dot < halfpt)
            step = (dot < c0pt) ? 1 : 3;
         else
            step = (dot < c3pt) ? 2 : 0;

         ep1[1] = dp[1] - stops[step];
         lmask |= step << 2;

         /* pixel 2 */
         dot = (dp[2] << 4) + (7 * ep1[1] + 3 * ep2[3] + 5 * ep2[2] + ep2[1]);
         if(dot < halfpt)
            step = (dot < c0pt) ? 1 : 3;
         else
            step = (dot < c3pt) ? 2 : 0;

         ep1[2] = dp[2] - stops[step];
         lmask |= step << 4;
         
         /* pixel 3 */
         dot = (dp[3] << 4) + (7 * ep1[2] + 5 * ep2[3] + ep2[2]);
         if(dot < halfpt)
            step = (dot < c0pt) ? 1 : 3;
         else
            step = (dot < c3pt) ? 2 : 0;

         ep1[3] = dp[3] - stops[step];
         lmask |= step << 6;

         /* advance to next line */
         tmp = ep1;
         ep1 = ep2;
         ep2 = tmp;
         
         dp += 4;
         mask |= lmask << (y * 8);
      }
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
                                  unsigned short *max16, unsigned short *min16)
{
   static const int niterpow = 4;
   
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
   
   magn = MAX(MAX(vfr, vfg), vfb);
   
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
   static const int w1tab[4] = {3, 0, 2, 1};
   static const int prods[4] = {0x090000, 0x000900, 0x040102, 0x010402};
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

static void encode_color_block(unsigned char *dst,
                               const unsigned char *block,
                               int dither, int dxt1_alpha)
{
   unsigned char dblock[64], color[4][3];
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
      mx = MAX(mx, v);
      mn = MIN(mn, v);
   }
   
   if(mn != mx) /* block is not a solid color, continue with compression */
   {
      /* compute dithered block for PCA if desired */
      if(dither)
         dither_block(dblock, block);
      
      /* pca + map along principal axis */
      optimize_colors_block(dither ? dblock : block, &max16, &min16);
      if(max16 != min16)
      {
         eval_colors(color, max16, min16);
         mask = match_colors_block(block, color, dither != 0);
      }
      else
         mask = 0;
      
      /* refine */
      refine_block(dither ? dblock : block, &max16, &min16, mask);
      
      if(max16 != min16)
      {
         eval_colors(color, max16, min16);
         mask = match_colors_block(block, color, dither != 0);
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
   if(dxt1_alpha && block_has_alpha)
   {
      if(max16 > min16)
      {
         max16 ^= min16; min16 ^= max16; max16 ^= min16;
      }
      eval_colors(color, max16, min16);
      mask = match_colors_block_DXT1alpha(block, color);
   }
   
   if(max16 < min16 && !(dxt1_alpha && block_has_alpha))
   {
      max16 ^= min16; min16 ^= max16; max16 ^= min16;
      mask ^= 0x55555555;
   }
      
   PUTL16(&dst[0], max16);
   PUTL16(&dst[2], min16);
   PUTL32(&dst[4], mask);
}

void ce_mmphlp_rgba8_compress_dxt(ce_mmpfile* mmpfile)
{
	assert(0 == (mmpfile->width & 3) && 0 == (mmpfile->height & 3)); // is mul4

	int size = ce_mmphlp_storage_requirements_dxt(mmpfile->width,
													mmpfile->height,
													mmpfile->mipmap_count,
													CE_MMPFILE_FORMAT_DXT1);
	void* data = ce_alloc(size);

	uint8_t* dst = data;
	const uint8_t* src = mmpfile->texels;

	unsigned char block[64];

	for (int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		for(int y = 0; y < height; y += 4)
		{
			for(int x = 0; x < width; x += 4)
			{
				extract_block(src, x, y, width, height, block);
				encode_color_block(dst, block, 0, 1);
				dst += 8;
			}
		}
		src += 4 * width * height;
	}

	ce_free(mmpfile->data, mmpfile->size);

	mmpfile->format = CE_MMPFILE_FORMAT_DXT1;
	mmpfile->texels = data;
	mmpfile->size = size;
	mmpfile->data = data;
}

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

	// return the value
	return value;
}

static void DecompressColour( uint8_t* rgba, void const* block, bool isDxt1 )
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

		if( isDxt1 && a <= b )
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
	codes[12 + 3] = ( isDxt1 && a <= b ) ? 0 : 255;

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

static void Decompress( uint8_t* rgba, void const* block, int format )
{
	// get the block locations
	void const* colourBlock = block;
	void const* alphaBock = block;

	if( CE_MMPFILE_FORMAT_DXT3 == format )
		colourBlock = (uint8_t const*)block + 8;

	// decompress colour
	DecompressColour( rgba, colourBlock, CE_MMPFILE_FORMAT_DXT1 == format );

	// decompress alpha separately if necessary
	if( CE_MMPFILE_FORMAT_DXT3 == format )
		DecompressAlphaDxt3( rgba, alphaBock );
}

void ce_mmphlp_dxt_decompress_rgba8(ce_mmpfile* mmpfile)
{
	assert(CE_MMPFILE_FORMAT_DXT1 == mmpfile->format ||
			CE_MMPFILE_FORMAT_DXT3 == mmpfile->format);

	int size = ce_mmphlp_storage_requirements_rgba8(mmpfile->width,
													mmpfile->height,
													mmpfile->mipmap_count);
	void* data = ce_alloc(size);

	uint8_t* dst = data;
	const uint8_t* src = mmpfile->texels;

	int bytesPerBlock = CE_MMPFILE_FORMAT_DXT1 == mmpfile->format ? 8 : 16;

	for (int i = 0, width = mmpfile->width, height = mmpfile->height;
			i < mmpfile->mipmap_count; ++i, width >>= 1, height >>= 1) {
		// loop over blocks
		for( int y = 0; y < height; y += 4 )
		{
			for( int x = 0; x < width; x += 4 )
			{
				// decompress the block
				uint8_t targetRgba[4*16];
				Decompress( targetRgba, src, mmpfile->format );

				// write the decompressed pixels to the correct image locations
				uint8_t const* sourcePixel = targetRgba;
				for( int py = 0; py < 4; ++py )
				{
					for( int px = 0; px < 4; ++px )
					{
						// get the target location
						int sx = x + px;
						int sy = y + py;
						if( sx < width && sy < height )
						{
							uint8_t* targetPixel = dst + 4*( width*sy + sx );

							// copy the rgba value
							for( int j = 0; j < 4; ++j )
								*targetPixel++ = *sourcePixel++;
						}
						else
						{
							// skip this pixel as its outside the image
							sourcePixel += 4;
						}
					}
				}

				// advance
				src += bytesPerBlock;
			}
		}

		dst += 4 * width * height;
	}

	ce_free(mmpfile->data, mmpfile->size);

	mmpfile->format = CE_MMPFILE_FORMAT_INVALID;
	mmpfile->texels = data;
	mmpfile->size = size;
	mmpfile->data = data;
}
