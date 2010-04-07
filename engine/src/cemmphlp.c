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

static int FloatTo565( const ce_vec3* color )
{
	// get the components in the correct range
	int r = ce_clamp( 31.0f*color->x + 0.5f, 0, 31 );
	int g = ce_clamp( 63.0f*color->y + 0.5f, 0, 63 );
	int b = ce_clamp( 31.0f*color->z + 0.5f, 0, 31 );
	
	// pack into a single value
	return ( r << 11 ) | ( g << 5 ) | b;
}

static void WriteColourBlock( int a, int b, uint8_t* indices, void* block )
{
	// get the block as bytes
	uint8_t* bytes = block;

	// write the endpoints
	bytes[0] = a & 0xff;
	bytes[1] = a >> 8;
	bytes[2] = b & 0xff;
	bytes[3] = b >> 8;
	
	// write the indices
	for( int i = 0; i < 4; ++i )
	{
		uint8_t const* ind = indices + 4*i;
		bytes[4 + i] = ind[0] | ( ind[1] << 2 ) | ( ind[2] << 4 ) | ( ind[3] << 6 );
	}
}

static void WriteColourBlock3( const ce_vec3* start, const ce_vec3* end,
								uint8_t const* indices, void* block )
{
	// get the packed values
	int a = FloatTo565( start );
	int b = FloatTo565( end );

	// remap the indices
	uint8_t remapped[16];
	if( a <= b )
	{
		// use the indices directly
		for( int i = 0; i < 16; ++i )
			remapped[i] = indices[i];
	}
	else
	{
		// swap a and b
		ce_swap(&a, &b);
		for( int i = 0; i < 16; ++i )
		{
			if( indices[i] == 0 )
				remapped[i] = 1;
			else if( indices[i] == 1 )
				remapped[i] = 0;
			else
				remapped[i] = indices[i];
		}
	}
	
	// write the block
	WriteColourBlock( a, b, remapped, block );
}

static void WriteColourBlock4( const ce_vec3* start, const ce_vec3* end,
						uint8_t const* indices, void* block )
{
	// get the packed values
	int a = FloatTo565( start );
	int b = FloatTo565( end );

	// remap the indices
	uint8_t remapped[16];
	if( a < b )
	{
		// swap a and b
		ce_swap( &a, &b );
		for( int i = 0; i < 16; ++i )
			remapped[i] = ( indices[i] ^ 0x1 ) & 0x3;
	}
	else if( a == b )
	{
		// use index 0
		for( int i = 0; i < 16; ++i )
			remapped[i] = 0;
	}
	else
	{
		// use the indices directly
		for( int i = 0; i < 16; ++i )
			remapped[i] = indices[i];
	}
	
	// write the block
	WriteColourBlock( a, b, remapped, block );
}

typedef struct {
	int count;
	ce_vec3 points[16];
	float weights[16];
	int remap[16];
	bool transparent;
} ce_mmpfile_colorset;

static void
ce_mmpfile_colorset_init(ce_mmpfile_colorset* cs,
						const uint8_t* rgba, int mask, int format)
{
	cs->count = 0;
	cs->transparent = false;

	// check the compression mode for dxt1
	bool isDxt1 = CE_MMPFILE_FORMAT_DXT1 == format;
	bool weightByAlpha = false; // TODO: test it

	// create the minimal set
	for( int i = 0; i < 16; ++i )
	{
		// check this pixel is enabled
		int bit = 1 << i;
		if( ( mask & bit ) == 0 )
		{
			cs->remap[i] = -1;
			continue;
		}

		// check for transparent pixels when using dxt1
		if( isDxt1 && rgba[4*i + 3] < 128 )
		{
			cs->remap[i] = -1;
			cs->transparent = true;
			continue;
		}

		// loop over previous points for a match
		for( int j = 0;; ++j )
		{
			// allocate a new point
			if( j == i )
			{
				// add the point

				// normalise coordinates to [0,1]
				cs->points[cs->count].x = rgba[4 * i + 0] / 255.0f;
				cs->points[cs->count].y = rgba[4 * i + 1] / 255.0f;
				cs->points[cs->count].z = rgba[4 * i + 2] / 255.0f;

				// ensure there is always non-zero weight even for zero alpha
				cs->weights[cs->count] = weightByAlpha ?
										((rgba[4*i + 3] + 1) / 256.0f) : 1.0f;

				cs->remap[i] = cs->count;

				// advance
				++cs->count;
				break;
			}

			// check for a match
			int oldbit = 1 << j;
			bool match = ( ( mask & oldbit ) != 0 )
				&& ( rgba[4*i] == rgba[4*j] )
				&& ( rgba[4*i + 1] == rgba[4*j + 1] )
				&& ( rgba[4*i + 2] == rgba[4*j + 2] )
				&& ( rgba[4*j + 3] >= 128 || !isDxt1 );
			if( match )
			{
				// get the index of the match
				int index = cs->remap[j];

				// ensure there is always non-zero weight even for zero alpha
				float w = ( float )( rgba[4*i + 3] + 1 ) / 256.0f;

				// map to this point and increase the weight
				cs->weights[index] += ( weightByAlpha ? w : 1.0f );
				cs->remap[i] = index;
				break;
			}
		}
	}

	// square root the weights
	for( int i = 0; i < cs->count; ++i )
		cs->weights[i] = sqrtf(cs->weights[i]);
}

static void ce_mmpfile_colorset_remap_indices( const ce_mmpfile_colorset* cs,
									uint8_t const* source, uint8_t* target )
{
	for( int i = 0; i < 16; ++i )
	{
		int j = cs->remap[i];
		if( j == -1 )
			target[i] = 3;
		else
			target[i] = source[j];
	}
}

typedef struct {
	uint8_t start;
	uint8_t end;
	uint8_t error;
} SourceBlock;

typedef struct {
	SourceBlock sources[2];
} SingleColourLookup;

extern SingleColourLookup const lookup_5_3[];
extern SingleColourLookup const lookup_6_3[];
extern SingleColourLookup const lookup_5_4[];
extern SingleColourLookup const lookup_6_4[];

typedef struct {
	const ce_mmpfile_colorset* cs;
	int format;
	uint8_t color[3];
	ce_vec3 start;
	ce_vec3 end;
	uint8_t index;
	int error;
	int besterror;
} ce_mmpfile_singlecolorfit;

static void ce_mmpfile_singlecolorfit_init(ce_mmpfile_singlecolorfit* cf,
									const ce_mmpfile_colorset* cs, int format)
{
	cf->cs = cs;
	cf->format = format;
	cf->besterror = INT_MAX;

	// grab the single colour
	// use ANSI round-to-zero behaviour to get round-to-nearest
	cf->color[0] = ce_clamp(255.0f * cs->points[0].x + 0.5f, 0, 255);
	cf->color[1] = ce_clamp(255.0f * cs->points[0].y + 0.5f, 0, 255);
	cf->color[2] = ce_clamp(255.0f * cs->points[0].z + 0.5f, 0, 255);
}

static void ce_mmpfile_singlecolorfit_compute_end_points(ce_mmpfile_singlecolorfit* cf,
										SingleColourLookup const* const* lookups)
{
	// check each index combination (endpoint or intermediate)
	cf->error = INT_MAX;
	for( int index = 0; index < 2; ++index )
	{
		// check the error for this codebook index
		SourceBlock const* sources[3];
		int error = 0;
		for( int channel = 0; channel < 3; ++channel )
		{
			// grab the lookup table and index for this channel
			SingleColourLookup const* lookup = lookups[channel];
			int target = cf->color[channel];
			
			// store a pointer to the source for this channel
			sources[channel] = lookup[target].sources + index;
			
			// accumulate the error
			int diff = sources[channel]->error;
			error += diff*diff;			
		}
		
		// keep it if the error is lower
		if( error < cf->error )
		{
			cf->start.x = sources[0]->start / 31.0f;
			cf->start.y = sources[1]->start / 63.0f;
			cf->start.z = sources[2]->start / 31.0f;

			cf->end.x = sources[0]->end / 31.0f;
			cf->end.y = sources[1]->end / 63.0f;
			cf->end.z = sources[2]->end / 31.0f;

			cf->index = ( 2*index );
			cf->error = error;
		}
	}
}

static void ce_mmpfile_singlecolorfit_compress3(ce_mmpfile_singlecolorfit* cf, void* block )
{
	// build the table of lookups
	SingleColourLookup const* const lookups[] = 
	{
		lookup_5_3, 
		lookup_6_3, 
		lookup_5_3
	};
	
	// find the best end-points and index
	ce_mmpfile_singlecolorfit_compute_end_points(cf, lookups);
	
	// build the block if we win
	if( cf->error < cf->besterror )
	{
		// remap the indices
		uint8_t indices[16];
		ce_mmpfile_colorset_remap_indices(cf->cs, &cf->index, indices);

		// save the block
		WriteColourBlock3( &cf->start, &cf->end, indices, block );

		// save the error
		cf->besterror = cf->error;
	}
}

static void ce_mmpfile_singlecolorfit_compress4( ce_mmpfile_singlecolorfit* cf, void* block )
{
	// build the table of lookups
	SingleColourLookup const* const lookups[] = 
	{
		lookup_5_4, 
		lookup_6_4, 
		lookup_5_4
	};
	
	// find the best end-points and index
	ce_mmpfile_singlecolorfit_compute_end_points(cf, lookups);
	
	// build the block if we win
	if( cf->error < cf->besterror )
	{
		// remap the indices
		uint8_t indices[16];
		ce_mmpfile_colorset_remap_indices(cf->cs, &cf->index, indices);

		// save the block
		WriteColourBlock4( &cf->start, &cf->end, indices, block );

		// save the error
		cf->besterror = cf->error;
	}
}

static void ce_mmpfile_singlecolorfit_compress(ce_mmpfile_singlecolorfit* cf, void* block)
{
	bool isDxt1 = CE_MMPFILE_FORMAT_DXT1 == cf->format;
	if( isDxt1 )
	{
		ce_mmpfile_singlecolorfit_compress3(cf, block);
		if (!cf->cs->transparent) {
			ce_mmpfile_singlecolorfit_compress4(cf, block);
		}
	}
	else {
		ce_mmpfile_singlecolorfit_compress4(cf, block);
	}
}

typedef struct {
	const ce_mmpfile_colorset* cs;
	int format;
	ce_vec3 metric;
	ce_vec3 start;
	ce_vec3 end;
	float besterror;
} ce_mmpfile_rangefit;

static void CompressAlphaDxt3( uint8_t const* rgba, int mask, void* block )
{
	uint8_t* bytes = block;

	// quantise and pack the alpha values pairwise
	for( int i = 0; i < 8; ++i )
	{
		// quantise down to 4 bits
		float alpha1 = rgba[8*i + 3] * ( 15.0f / 255.0f );
		float alpha2 = rgba[8*i + 7] * ( 15.0f / 255.0f );
		// use ANSI round-to-zero behaviour to get round-to-nearest
		int quant1 = ce_clamp(alpha1 + 0.5f, 0, 15);
		int quant2 = ce_clamp(alpha2 + 0.5f, 0, 15);

		// set alpha to zero where masked
		int bit1 = 1 << ( 2*i );
		int bit2 = 1 << ( 2*i + 1 );
		if( ( mask & bit1 ) == 0 )
			quant1 = 0;
		if( ( mask & bit2 ) == 0 )
			quant2 = 0;

		// pack into the byte
		bytes[i] = quant1 | ( quant2 << 4 );
	}
}

static void CompressMasked( uint8_t const* rgba, int mask, void* block, int format )
{
	// get the block locations
	void* colourBlock = block;
	void* alphaBock = block;

	if( CE_MMPFILE_FORMAT_DXT3 == format )
		colourBlock = (uint8_t*)block + 8;

	// create the minimal point set
	ce_mmpfile_colorset cs;
	ce_mmpfile_colorset_init(&cs, rgba, mask, format);

	// check the compression type and compress colour
	if (1 == cs.count)
	{
		// always do a single colour fit
		ce_mmpfile_singlecolorfit cf;
		ce_mmpfile_singlecolorfit_init(&cf, &cs, format);
		ce_mmpfile_singlecolorfit_compress(&cf, colourBlock);
	}
	else if( true/*0 == cs.count*/ )
	{
		// do a range fit
		//ce_mmpfile_rangefit rf;
		//ce_mmpfile_rangefit_init(&rf, &cs, format);
		//ce_mmpfile_rangefit_compress(&rf, colourBlock);
	}
	else
	{
		// default to a cluster fit (could be iterative or not)
		//ClusterFit fit( &colours, flags );
		//fit.Compress( colourBlock );
	}

	// compress alpha separately if necessary
	if( CE_MMPFILE_FORMAT_DXT3 == format )
		CompressAlphaDxt3( rgba, mask, alphaBock );
}

void ce_mmphlp_rgba8_compress_dxt(void* restrict dst, const void* restrict src,
							int width, int height, int format)
{
	assert(CE_MMPFILE_FORMAT_DXT1 == format || CE_MMPFILE_FORMAT_DXT3 == format);

	const uint8_t* s = src;

	// initialise the block output
	uint8_t* targetBlock = dst;
	int bytesPerBlock = CE_MMPFILE_FORMAT_DXT1 == format ? 8 : 16;

	// loop over blocks
	for( int y = 0; y < height; y += 4 )
	{
		for( int x = 0; x < width; x += 4 )
		{
			// build the 4x4 block of pixels
			uint8_t sourceRgba[16*4];
			uint8_t* targetPixel = sourceRgba;
			int mask = 0;
			for( int py = 0; py < 4; ++py )
			{
				for( int px = 0; px < 4; ++px )
				{
					// get the source pixel in the image
					int sx = x + px;
					int sy = y + py;

					// enable if we're in the image
					if( sx < width && sy < height )
					{
						// copy the rgba value
						uint8_t const* sourcePixel = s + 4*( width*sy + sx );
						for( int i = 0; i < 4; ++i )
							*targetPixel++ = *sourcePixel++;

						// enable this pixel
						mask |= ( 1 << ( 4*py + px ) );
					}
					else
					{
						// skip this pixel as its outside the image
						targetPixel += 4;
					}
				}
			}

			// compress it into the output
			CompressMasked( sourceRgba, mask, targetBlock, format );

			// advance
			targetBlock += bytesPerBlock;
		}
	}
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

SingleColourLookup const lookup_5_3[] =
{
	{ { { 0, 0, 0 }, { 0, 0, 0 } } },
	{ { { 0, 0, 1 }, { 0, 0, 1 } } },
	{ { { 0, 0, 2 }, { 0, 0, 2 } } },
	{ { { 0, 0, 3 }, { 0, 1, 1 } } },
	{ { { 0, 0, 4 }, { 0, 1, 0 } } },
	{ { { 1, 0, 3 }, { 0, 1, 1 } } },
	{ { { 1, 0, 2 }, { 0, 1, 2 } } },
	{ { { 1, 0, 1 }, { 0, 2, 1 } } },
	{ { { 1, 0, 0 }, { 0, 2, 0 } } },
	{ { { 1, 0, 1 }, { 0, 2, 1 } } },
	{ { { 1, 0, 2 }, { 0, 2, 2 } } },
	{ { { 1, 0, 3 }, { 0, 3, 1 } } },
	{ { { 1, 0, 4 }, { 0, 3, 0 } } },
	{ { { 2, 0, 3 }, { 0, 3, 1 } } },
	{ { { 2, 0, 2 }, { 0, 3, 2 } } },
	{ { { 2, 0, 1 }, { 0, 4, 1 } } },
	{ { { 2, 0, 0 }, { 0, 4, 0 } } },
	{ { { 2, 0, 1 }, { 0, 4, 1 } } },
	{ { { 2, 0, 2 }, { 0, 4, 2 } } },
	{ { { 2, 0, 3 }, { 0, 5, 1 } } },
	{ { { 2, 0, 4 }, { 0, 5, 0 } } },
	{ { { 3, 0, 3 }, { 0, 5, 1 } } },
	{ { { 3, 0, 2 }, { 0, 5, 2 } } },
	{ { { 3, 0, 1 }, { 0, 6, 1 } } },
	{ { { 3, 0, 0 }, { 0, 6, 0 } } },
	{ { { 3, 0, 1 }, { 0, 6, 1 } } },
	{ { { 3, 0, 2 }, { 0, 6, 2 } } },
	{ { { 3, 0, 3 }, { 0, 7, 1 } } },
	{ { { 3, 0, 4 }, { 0, 7, 0 } } },
	{ { { 4, 0, 4 }, { 0, 7, 1 } } },
	{ { { 4, 0, 3 }, { 0, 7, 2 } } },
	{ { { 4, 0, 2 }, { 1, 7, 1 } } },
	{ { { 4, 0, 1 }, { 1, 7, 0 } } },
	{ { { 4, 0, 0 }, { 0, 8, 0 } } },
	{ { { 4, 0, 1 }, { 0, 8, 1 } } },
	{ { { 4, 0, 2 }, { 2, 7, 1 } } },
	{ { { 4, 0, 3 }, { 2, 7, 0 } } },
	{ { { 4, 0, 4 }, { 0, 9, 0 } } },
	{ { { 5, 0, 3 }, { 0, 9, 1 } } },
	{ { { 5, 0, 2 }, { 3, 7, 1 } } },
	{ { { 5, 0, 1 }, { 3, 7, 0 } } },
	{ { { 5, 0, 0 }, { 0, 10, 0 } } },
	{ { { 5, 0, 1 }, { 0, 10, 1 } } },
	{ { { 5, 0, 2 }, { 0, 10, 2 } } },
	{ { { 5, 0, 3 }, { 0, 11, 1 } } },
	{ { { 5, 0, 4 }, { 0, 11, 0 } } },
	{ { { 6, 0, 3 }, { 0, 11, 1 } } },
	{ { { 6, 0, 2 }, { 0, 11, 2 } } },
	{ { { 6, 0, 1 }, { 0, 12, 1 } } },
	{ { { 6, 0, 0 }, { 0, 12, 0 } } },
	{ { { 6, 0, 1 }, { 0, 12, 1 } } },
	{ { { 6, 0, 2 }, { 0, 12, 2 } } },
	{ { { 6, 0, 3 }, { 0, 13, 1 } } },
	{ { { 6, 0, 4 }, { 0, 13, 0 } } },
	{ { { 7, 0, 3 }, { 0, 13, 1 } } },
	{ { { 7, 0, 2 }, { 0, 13, 2 } } },
	{ { { 7, 0, 1 }, { 0, 14, 1 } } },
	{ { { 7, 0, 0 }, { 0, 14, 0 } } },
	{ { { 7, 0, 1 }, { 0, 14, 1 } } },
	{ { { 7, 0, 2 }, { 0, 14, 2 } } },
	{ { { 7, 0, 3 }, { 0, 15, 1 } } },
	{ { { 7, 0, 4 }, { 0, 15, 0 } } },
	{ { { 8, 0, 4 }, { 0, 15, 1 } } },
	{ { { 8, 0, 3 }, { 0, 15, 2 } } },
	{ { { 8, 0, 2 }, { 1, 15, 1 } } },
	{ { { 8, 0, 1 }, { 1, 15, 0 } } },
	{ { { 8, 0, 0 }, { 0, 16, 0 } } },
	{ { { 8, 0, 1 }, { 0, 16, 1 } } },
	{ { { 8, 0, 2 }, { 2, 15, 1 } } },
	{ { { 8, 0, 3 }, { 2, 15, 0 } } },
	{ { { 8, 0, 4 }, { 0, 17, 0 } } },
	{ { { 9, 0, 3 }, { 0, 17, 1 } } },
	{ { { 9, 0, 2 }, { 3, 15, 1 } } },
	{ { { 9, 0, 1 }, { 3, 15, 0 } } },
	{ { { 9, 0, 0 }, { 0, 18, 0 } } },
	{ { { 9, 0, 1 }, { 0, 18, 1 } } },
	{ { { 9, 0, 2 }, { 0, 18, 2 } } },
	{ { { 9, 0, 3 }, { 0, 19, 1 } } },
	{ { { 9, 0, 4 }, { 0, 19, 0 } } },
	{ { { 10, 0, 3 }, { 0, 19, 1 } } },
	{ { { 10, 0, 2 }, { 0, 19, 2 } } },
	{ { { 10, 0, 1 }, { 0, 20, 1 } } },
	{ { { 10, 0, 0 }, { 0, 20, 0 } } },
	{ { { 10, 0, 1 }, { 0, 20, 1 } } },
	{ { { 10, 0, 2 }, { 0, 20, 2 } } },
	{ { { 10, 0, 3 }, { 0, 21, 1 } } },
	{ { { 10, 0, 4 }, { 0, 21, 0 } } },
	{ { { 11, 0, 3 }, { 0, 21, 1 } } },
	{ { { 11, 0, 2 }, { 0, 21, 2 } } },
	{ { { 11, 0, 1 }, { 0, 22, 1 } } },
	{ { { 11, 0, 0 }, { 0, 22, 0 } } },
	{ { { 11, 0, 1 }, { 0, 22, 1 } } },
	{ { { 11, 0, 2 }, { 0, 22, 2 } } },
	{ { { 11, 0, 3 }, { 0, 23, 1 } } },
	{ { { 11, 0, 4 }, { 0, 23, 0 } } },
	{ { { 12, 0, 4 }, { 0, 23, 1 } } },
	{ { { 12, 0, 3 }, { 0, 23, 2 } } },
	{ { { 12, 0, 2 }, { 1, 23, 1 } } },
	{ { { 12, 0, 1 }, { 1, 23, 0 } } },
	{ { { 12, 0, 0 }, { 0, 24, 0 } } },
	{ { { 12, 0, 1 }, { 0, 24, 1 } } },
	{ { { 12, 0, 2 }, { 2, 23, 1 } } },
	{ { { 12, 0, 3 }, { 2, 23, 0 } } },
	{ { { 12, 0, 4 }, { 0, 25, 0 } } },
	{ { { 13, 0, 3 }, { 0, 25, 1 } } },
	{ { { 13, 0, 2 }, { 3, 23, 1 } } },
	{ { { 13, 0, 1 }, { 3, 23, 0 } } },
	{ { { 13, 0, 0 }, { 0, 26, 0 } } },
	{ { { 13, 0, 1 }, { 0, 26, 1 } } },
	{ { { 13, 0, 2 }, { 0, 26, 2 } } },
	{ { { 13, 0, 3 }, { 0, 27, 1 } } },
	{ { { 13, 0, 4 }, { 0, 27, 0 } } },
	{ { { 14, 0, 3 }, { 0, 27, 1 } } },
	{ { { 14, 0, 2 }, { 0, 27, 2 } } },
	{ { { 14, 0, 1 }, { 0, 28, 1 } } },
	{ { { 14, 0, 0 }, { 0, 28, 0 } } },
	{ { { 14, 0, 1 }, { 0, 28, 1 } } },
	{ { { 14, 0, 2 }, { 0, 28, 2 } } },
	{ { { 14, 0, 3 }, { 0, 29, 1 } } },
	{ { { 14, 0, 4 }, { 0, 29, 0 } } },
	{ { { 15, 0, 3 }, { 0, 29, 1 } } },
	{ { { 15, 0, 2 }, { 0, 29, 2 } } },
	{ { { 15, 0, 1 }, { 0, 30, 1 } } },
	{ { { 15, 0, 0 }, { 0, 30, 0 } } },
	{ { { 15, 0, 1 }, { 0, 30, 1 } } },
	{ { { 15, 0, 2 }, { 0, 30, 2 } } },
	{ { { 15, 0, 3 }, { 0, 31, 1 } } },
	{ { { 15, 0, 4 }, { 0, 31, 0 } } },
	{ { { 16, 0, 4 }, { 0, 31, 1 } } },
	{ { { 16, 0, 3 }, { 0, 31, 2 } } },
	{ { { 16, 0, 2 }, { 1, 31, 1 } } },
	{ { { 16, 0, 1 }, { 1, 31, 0 } } },
	{ { { 16, 0, 0 }, { 4, 28, 0 } } },
	{ { { 16, 0, 1 }, { 4, 28, 1 } } },
	{ { { 16, 0, 2 }, { 2, 31, 1 } } },
	{ { { 16, 0, 3 }, { 2, 31, 0 } } },
	{ { { 16, 0, 4 }, { 4, 29, 0 } } },
	{ { { 17, 0, 3 }, { 4, 29, 1 } } },
	{ { { 17, 0, 2 }, { 3, 31, 1 } } },
	{ { { 17, 0, 1 }, { 3, 31, 0 } } },
	{ { { 17, 0, 0 }, { 4, 30, 0 } } },
	{ { { 17, 0, 1 }, { 4, 30, 1 } } },
	{ { { 17, 0, 2 }, { 4, 30, 2 } } },
	{ { { 17, 0, 3 }, { 4, 31, 1 } } },
	{ { { 17, 0, 4 }, { 4, 31, 0 } } },
	{ { { 18, 0, 3 }, { 4, 31, 1 } } },
	{ { { 18, 0, 2 }, { 4, 31, 2 } } },
	{ { { 18, 0, 1 }, { 5, 31, 1 } } },
	{ { { 18, 0, 0 }, { 5, 31, 0 } } },
	{ { { 18, 0, 1 }, { 5, 31, 1 } } },
	{ { { 18, 0, 2 }, { 5, 31, 2 } } },
	{ { { 18, 0, 3 }, { 6, 31, 1 } } },
	{ { { 18, 0, 4 }, { 6, 31, 0 } } },
	{ { { 19, 0, 3 }, { 6, 31, 1 } } },
	{ { { 19, 0, 2 }, { 6, 31, 2 } } },
	{ { { 19, 0, 1 }, { 7, 31, 1 } } },
	{ { { 19, 0, 0 }, { 7, 31, 0 } } },
	{ { { 19, 0, 1 }, { 7, 31, 1 } } },
	{ { { 19, 0, 2 }, { 7, 31, 2 } } },
	{ { { 19, 0, 3 }, { 8, 31, 1 } } },
	{ { { 19, 0, 4 }, { 8, 31, 0 } } },
	{ { { 20, 0, 4 }, { 8, 31, 1 } } },
	{ { { 20, 0, 3 }, { 8, 31, 2 } } },
	{ { { 20, 0, 2 }, { 9, 31, 1 } } },
	{ { { 20, 0, 1 }, { 9, 31, 0 } } },
	{ { { 20, 0, 0 }, { 12, 28, 0 } } },
	{ { { 20, 0, 1 }, { 12, 28, 1 } } },
	{ { { 20, 0, 2 }, { 10, 31, 1 } } },
	{ { { 20, 0, 3 }, { 10, 31, 0 } } },
	{ { { 20, 0, 4 }, { 12, 29, 0 } } },
	{ { { 21, 0, 3 }, { 12, 29, 1 } } },
	{ { { 21, 0, 2 }, { 11, 31, 1 } } },
	{ { { 21, 0, 1 }, { 11, 31, 0 } } },
	{ { { 21, 0, 0 }, { 12, 30, 0 } } },
	{ { { 21, 0, 1 }, { 12, 30, 1 } } },
	{ { { 21, 0, 2 }, { 12, 30, 2 } } },
	{ { { 21, 0, 3 }, { 12, 31, 1 } } },
	{ { { 21, 0, 4 }, { 12, 31, 0 } } },
	{ { { 22, 0, 3 }, { 12, 31, 1 } } },
	{ { { 22, 0, 2 }, { 12, 31, 2 } } },
	{ { { 22, 0, 1 }, { 13, 31, 1 } } },
	{ { { 22, 0, 0 }, { 13, 31, 0 } } },
	{ { { 22, 0, 1 }, { 13, 31, 1 } } },
	{ { { 22, 0, 2 }, { 13, 31, 2 } } },
	{ { { 22, 0, 3 }, { 14, 31, 1 } } },
	{ { { 22, 0, 4 }, { 14, 31, 0 } } },
	{ { { 23, 0, 3 }, { 14, 31, 1 } } },
	{ { { 23, 0, 2 }, { 14, 31, 2 } } },
	{ { { 23, 0, 1 }, { 15, 31, 1 } } },
	{ { { 23, 0, 0 }, { 15, 31, 0 } } },
	{ { { 23, 0, 1 }, { 15, 31, 1 } } },
	{ { { 23, 0, 2 }, { 15, 31, 2 } } },
	{ { { 23, 0, 3 }, { 16, 31, 1 } } },
	{ { { 23, 0, 4 }, { 16, 31, 0 } } },
	{ { { 24, 0, 4 }, { 16, 31, 1 } } },
	{ { { 24, 0, 3 }, { 16, 31, 2 } } },
	{ { { 24, 0, 2 }, { 17, 31, 1 } } },
	{ { { 24, 0, 1 }, { 17, 31, 0 } } },
	{ { { 24, 0, 0 }, { 20, 28, 0 } } },
	{ { { 24, 0, 1 }, { 20, 28, 1 } } },
	{ { { 24, 0, 2 }, { 18, 31, 1 } } },
	{ { { 24, 0, 3 }, { 18, 31, 0 } } },
	{ { { 24, 0, 4 }, { 20, 29, 0 } } },
	{ { { 25, 0, 3 }, { 20, 29, 1 } } },
	{ { { 25, 0, 2 }, { 19, 31, 1 } } },
	{ { { 25, 0, 1 }, { 19, 31, 0 } } },
	{ { { 25, 0, 0 }, { 20, 30, 0 } } },
	{ { { 25, 0, 1 }, { 20, 30, 1 } } },
	{ { { 25, 0, 2 }, { 20, 30, 2 } } },
	{ { { 25, 0, 3 }, { 20, 31, 1 } } },
	{ { { 25, 0, 4 }, { 20, 31, 0 } } },
	{ { { 26, 0, 3 }, { 20, 31, 1 } } },
	{ { { 26, 0, 2 }, { 20, 31, 2 } } },
	{ { { 26, 0, 1 }, { 21, 31, 1 } } },
	{ { { 26, 0, 0 }, { 21, 31, 0 } } },
	{ { { 26, 0, 1 }, { 21, 31, 1 } } },
	{ { { 26, 0, 2 }, { 21, 31, 2 } } },
	{ { { 26, 0, 3 }, { 22, 31, 1 } } },
	{ { { 26, 0, 4 }, { 22, 31, 0 } } },
	{ { { 27, 0, 3 }, { 22, 31, 1 } } },
	{ { { 27, 0, 2 }, { 22, 31, 2 } } },
	{ { { 27, 0, 1 }, { 23, 31, 1 } } },
	{ { { 27, 0, 0 }, { 23, 31, 0 } } },
	{ { { 27, 0, 1 }, { 23, 31, 1 } } },
	{ { { 27, 0, 2 }, { 23, 31, 2 } } },
	{ { { 27, 0, 3 }, { 24, 31, 1 } } },
	{ { { 27, 0, 4 }, { 24, 31, 0 } } },
	{ { { 28, 0, 4 }, { 24, 31, 1 } } },
	{ { { 28, 0, 3 }, { 24, 31, 2 } } },
	{ { { 28, 0, 2 }, { 25, 31, 1 } } },
	{ { { 28, 0, 1 }, { 25, 31, 0 } } },
	{ { { 28, 0, 0 }, { 28, 28, 0 } } },
	{ { { 28, 0, 1 }, { 28, 28, 1 } } },
	{ { { 28, 0, 2 }, { 26, 31, 1 } } },
	{ { { 28, 0, 3 }, { 26, 31, 0 } } },
	{ { { 28, 0, 4 }, { 28, 29, 0 } } },
	{ { { 29, 0, 3 }, { 28, 29, 1 } } },
	{ { { 29, 0, 2 }, { 27, 31, 1 } } },
	{ { { 29, 0, 1 }, { 27, 31, 0 } } },
	{ { { 29, 0, 0 }, { 28, 30, 0 } } },
	{ { { 29, 0, 1 }, { 28, 30, 1 } } },
	{ { { 29, 0, 2 }, { 28, 30, 2 } } },
	{ { { 29, 0, 3 }, { 28, 31, 1 } } },
	{ { { 29, 0, 4 }, { 28, 31, 0 } } },
	{ { { 30, 0, 3 }, { 28, 31, 1 } } },
	{ { { 30, 0, 2 }, { 28, 31, 2 } } },
	{ { { 30, 0, 1 }, { 29, 31, 1 } } },
	{ { { 30, 0, 0 }, { 29, 31, 0 } } },
	{ { { 30, 0, 1 }, { 29, 31, 1 } } },
	{ { { 30, 0, 2 }, { 29, 31, 2 } } },
	{ { { 30, 0, 3 }, { 30, 31, 1 } } },
	{ { { 30, 0, 4 }, { 30, 31, 0 } } },
	{ { { 31, 0, 3 }, { 30, 31, 1 } } },
	{ { { 31, 0, 2 }, { 30, 31, 2 } } },
	{ { { 31, 0, 1 }, { 31, 31, 1 } } },
	{ { { 31, 0, 0 }, { 31, 31, 0 } } }
};

SingleColourLookup const lookup_6_3[] =
{
	{ { { 0, 0, 0 }, { 0, 0, 0 } } },
	{ { { 0, 0, 1 }, { 0, 1, 1 } } },
	{ { { 0, 0, 2 }, { 0, 1, 0 } } },
	{ { { 1, 0, 1 }, { 0, 2, 1 } } },
	{ { { 1, 0, 0 }, { 0, 2, 0 } } },
	{ { { 1, 0, 1 }, { 0, 3, 1 } } },
	{ { { 1, 0, 2 }, { 0, 3, 0 } } },
	{ { { 2, 0, 1 }, { 0, 4, 1 } } },
	{ { { 2, 0, 0 }, { 0, 4, 0 } } },
	{ { { 2, 0, 1 }, { 0, 5, 1 } } },
	{ { { 2, 0, 2 }, { 0, 5, 0 } } },
	{ { { 3, 0, 1 }, { 0, 6, 1 } } },
	{ { { 3, 0, 0 }, { 0, 6, 0 } } },
	{ { { 3, 0, 1 }, { 0, 7, 1 } } },
	{ { { 3, 0, 2 }, { 0, 7, 0 } } },
	{ { { 4, 0, 1 }, { 0, 8, 1 } } },
	{ { { 4, 0, 0 }, { 0, 8, 0 } } },
	{ { { 4, 0, 1 }, { 0, 9, 1 } } },
	{ { { 4, 0, 2 }, { 0, 9, 0 } } },
	{ { { 5, 0, 1 }, { 0, 10, 1 } } },
	{ { { 5, 0, 0 }, { 0, 10, 0 } } },
	{ { { 5, 0, 1 }, { 0, 11, 1 } } },
	{ { { 5, 0, 2 }, { 0, 11, 0 } } },
	{ { { 6, 0, 1 }, { 0, 12, 1 } } },
	{ { { 6, 0, 0 }, { 0, 12, 0 } } },
	{ { { 6, 0, 1 }, { 0, 13, 1 } } },
	{ { { 6, 0, 2 }, { 0, 13, 0 } } },
	{ { { 7, 0, 1 }, { 0, 14, 1 } } },
	{ { { 7, 0, 0 }, { 0, 14, 0 } } },
	{ { { 7, 0, 1 }, { 0, 15, 1 } } },
	{ { { 7, 0, 2 }, { 0, 15, 0 } } },
	{ { { 8, 0, 1 }, { 0, 16, 1 } } },
	{ { { 8, 0, 0 }, { 0, 16, 0 } } },
	{ { { 8, 0, 1 }, { 0, 17, 1 } } },
	{ { { 8, 0, 2 }, { 0, 17, 0 } } },
	{ { { 9, 0, 1 }, { 0, 18, 1 } } },
	{ { { 9, 0, 0 }, { 0, 18, 0 } } },
	{ { { 9, 0, 1 }, { 0, 19, 1 } } },
	{ { { 9, 0, 2 }, { 0, 19, 0 } } },
	{ { { 10, 0, 1 }, { 0, 20, 1 } } },
	{ { { 10, 0, 0 }, { 0, 20, 0 } } },
	{ { { 10, 0, 1 }, { 0, 21, 1 } } },
	{ { { 10, 0, 2 }, { 0, 21, 0 } } },
	{ { { 11, 0, 1 }, { 0, 22, 1 } } },
	{ { { 11, 0, 0 }, { 0, 22, 0 } } },
	{ { { 11, 0, 1 }, { 0, 23, 1 } } },
	{ { { 11, 0, 2 }, { 0, 23, 0 } } },
	{ { { 12, 0, 1 }, { 0, 24, 1 } } },
	{ { { 12, 0, 0 }, { 0, 24, 0 } } },
	{ { { 12, 0, 1 }, { 0, 25, 1 } } },
	{ { { 12, 0, 2 }, { 0, 25, 0 } } },
	{ { { 13, 0, 1 }, { 0, 26, 1 } } },
	{ { { 13, 0, 0 }, { 0, 26, 0 } } },
	{ { { 13, 0, 1 }, { 0, 27, 1 } } },
	{ { { 13, 0, 2 }, { 0, 27, 0 } } },
	{ { { 14, 0, 1 }, { 0, 28, 1 } } },
	{ { { 14, 0, 0 }, { 0, 28, 0 } } },
	{ { { 14, 0, 1 }, { 0, 29, 1 } } },
	{ { { 14, 0, 2 }, { 0, 29, 0 } } },
	{ { { 15, 0, 1 }, { 0, 30, 1 } } },
	{ { { 15, 0, 0 }, { 0, 30, 0 } } },
	{ { { 15, 0, 1 }, { 0, 31, 1 } } },
	{ { { 15, 0, 2 }, { 0, 31, 0 } } },
	{ { { 16, 0, 2 }, { 1, 31, 1 } } },
	{ { { 16, 0, 1 }, { 1, 31, 0 } } },
	{ { { 16, 0, 0 }, { 0, 32, 0 } } },
	{ { { 16, 0, 1 }, { 2, 31, 0 } } },
	{ { { 16, 0, 2 }, { 0, 33, 0 } } },
	{ { { 17, 0, 1 }, { 3, 31, 0 } } },
	{ { { 17, 0, 0 }, { 0, 34, 0 } } },
	{ { { 17, 0, 1 }, { 4, 31, 0 } } },
	{ { { 17, 0, 2 }, { 0, 35, 0 } } },
	{ { { 18, 0, 1 }, { 5, 31, 0 } } },
	{ { { 18, 0, 0 }, { 0, 36, 0 } } },
	{ { { 18, 0, 1 }, { 6, 31, 0 } } },
	{ { { 18, 0, 2 }, { 0, 37, 0 } } },
	{ { { 19, 0, 1 }, { 7, 31, 0 } } },
	{ { { 19, 0, 0 }, { 0, 38, 0 } } },
	{ { { 19, 0, 1 }, { 8, 31, 0 } } },
	{ { { 19, 0, 2 }, { 0, 39, 0 } } },
	{ { { 20, 0, 1 }, { 9, 31, 0 } } },
	{ { { 20, 0, 0 }, { 0, 40, 0 } } },
	{ { { 20, 0, 1 }, { 10, 31, 0 } } },
	{ { { 20, 0, 2 }, { 0, 41, 0 } } },
	{ { { 21, 0, 1 }, { 11, 31, 0 } } },
	{ { { 21, 0, 0 }, { 0, 42, 0 } } },
	{ { { 21, 0, 1 }, { 12, 31, 0 } } },
	{ { { 21, 0, 2 }, { 0, 43, 0 } } },
	{ { { 22, 0, 1 }, { 13, 31, 0 } } },
	{ { { 22, 0, 0 }, { 0, 44, 0 } } },
	{ { { 22, 0, 1 }, { 14, 31, 0 } } },
	{ { { 22, 0, 2 }, { 0, 45, 0 } } },
	{ { { 23, 0, 1 }, { 15, 31, 0 } } },
	{ { { 23, 0, 0 }, { 0, 46, 0 } } },
	{ { { 23, 0, 1 }, { 0, 47, 1 } } },
	{ { { 23, 0, 2 }, { 0, 47, 0 } } },
	{ { { 24, 0, 1 }, { 0, 48, 1 } } },
	{ { { 24, 0, 0 }, { 0, 48, 0 } } },
	{ { { 24, 0, 1 }, { 0, 49, 1 } } },
	{ { { 24, 0, 2 }, { 0, 49, 0 } } },
	{ { { 25, 0, 1 }, { 0, 50, 1 } } },
	{ { { 25, 0, 0 }, { 0, 50, 0 } } },
	{ { { 25, 0, 1 }, { 0, 51, 1 } } },
	{ { { 25, 0, 2 }, { 0, 51, 0 } } },
	{ { { 26, 0, 1 }, { 0, 52, 1 } } },
	{ { { 26, 0, 0 }, { 0, 52, 0 } } },
	{ { { 26, 0, 1 }, { 0, 53, 1 } } },
	{ { { 26, 0, 2 }, { 0, 53, 0 } } },
	{ { { 27, 0, 1 }, { 0, 54, 1 } } },
	{ { { 27, 0, 0 }, { 0, 54, 0 } } },
	{ { { 27, 0, 1 }, { 0, 55, 1 } } },
	{ { { 27, 0, 2 }, { 0, 55, 0 } } },
	{ { { 28, 0, 1 }, { 0, 56, 1 } } },
	{ { { 28, 0, 0 }, { 0, 56, 0 } } },
	{ { { 28, 0, 1 }, { 0, 57, 1 } } },
	{ { { 28, 0, 2 }, { 0, 57, 0 } } },
	{ { { 29, 0, 1 }, { 0, 58, 1 } } },
	{ { { 29, 0, 0 }, { 0, 58, 0 } } },
	{ { { 29, 0, 1 }, { 0, 59, 1 } } },
	{ { { 29, 0, 2 }, { 0, 59, 0 } } },
	{ { { 30, 0, 1 }, { 0, 60, 1 } } },
	{ { { 30, 0, 0 }, { 0, 60, 0 } } },
	{ { { 30, 0, 1 }, { 0, 61, 1 } } },
	{ { { 30, 0, 2 }, { 0, 61, 0 } } },
	{ { { 31, 0, 1 }, { 0, 62, 1 } } },
	{ { { 31, 0, 0 }, { 0, 62, 0 } } },
	{ { { 31, 0, 1 }, { 0, 63, 1 } } },
	{ { { 31, 0, 2 }, { 0, 63, 0 } } },
	{ { { 32, 0, 2 }, { 1, 63, 1 } } },
	{ { { 32, 0, 1 }, { 1, 63, 0 } } },
	{ { { 32, 0, 0 }, { 16, 48, 0 } } },
	{ { { 32, 0, 1 }, { 2, 63, 0 } } },
	{ { { 32, 0, 2 }, { 16, 49, 0 } } },
	{ { { 33, 0, 1 }, { 3, 63, 0 } } },
	{ { { 33, 0, 0 }, { 16, 50, 0 } } },
	{ { { 33, 0, 1 }, { 4, 63, 0 } } },
	{ { { 33, 0, 2 }, { 16, 51, 0 } } },
	{ { { 34, 0, 1 }, { 5, 63, 0 } } },
	{ { { 34, 0, 0 }, { 16, 52, 0 } } },
	{ { { 34, 0, 1 }, { 6, 63, 0 } } },
	{ { { 34, 0, 2 }, { 16, 53, 0 } } },
	{ { { 35, 0, 1 }, { 7, 63, 0 } } },
	{ { { 35, 0, 0 }, { 16, 54, 0 } } },
	{ { { 35, 0, 1 }, { 8, 63, 0 } } },
	{ { { 35, 0, 2 }, { 16, 55, 0 } } },
	{ { { 36, 0, 1 }, { 9, 63, 0 } } },
	{ { { 36, 0, 0 }, { 16, 56, 0 } } },
	{ { { 36, 0, 1 }, { 10, 63, 0 } } },
	{ { { 36, 0, 2 }, { 16, 57, 0 } } },
	{ { { 37, 0, 1 }, { 11, 63, 0 } } },
	{ { { 37, 0, 0 }, { 16, 58, 0 } } },
	{ { { 37, 0, 1 }, { 12, 63, 0 } } },
	{ { { 37, 0, 2 }, { 16, 59, 0 } } },
	{ { { 38, 0, 1 }, { 13, 63, 0 } } },
	{ { { 38, 0, 0 }, { 16, 60, 0 } } },
	{ { { 38, 0, 1 }, { 14, 63, 0 } } },
	{ { { 38, 0, 2 }, { 16, 61, 0 } } },
	{ { { 39, 0, 1 }, { 15, 63, 0 } } },
	{ { { 39, 0, 0 }, { 16, 62, 0 } } },
	{ { { 39, 0, 1 }, { 16, 63, 1 } } },
	{ { { 39, 0, 2 }, { 16, 63, 0 } } },
	{ { { 40, 0, 1 }, { 17, 63, 1 } } },
	{ { { 40, 0, 0 }, { 17, 63, 0 } } },
	{ { { 40, 0, 1 }, { 18, 63, 1 } } },
	{ { { 40, 0, 2 }, { 18, 63, 0 } } },
	{ { { 41, 0, 1 }, { 19, 63, 1 } } },
	{ { { 41, 0, 0 }, { 19, 63, 0 } } },
	{ { { 41, 0, 1 }, { 20, 63, 1 } } },
	{ { { 41, 0, 2 }, { 20, 63, 0 } } },
	{ { { 42, 0, 1 }, { 21, 63, 1 } } },
	{ { { 42, 0, 0 }, { 21, 63, 0 } } },
	{ { { 42, 0, 1 }, { 22, 63, 1 } } },
	{ { { 42, 0, 2 }, { 22, 63, 0 } } },
	{ { { 43, 0, 1 }, { 23, 63, 1 } } },
	{ { { 43, 0, 0 }, { 23, 63, 0 } } },
	{ { { 43, 0, 1 }, { 24, 63, 1 } } },
	{ { { 43, 0, 2 }, { 24, 63, 0 } } },
	{ { { 44, 0, 1 }, { 25, 63, 1 } } },
	{ { { 44, 0, 0 }, { 25, 63, 0 } } },
	{ { { 44, 0, 1 }, { 26, 63, 1 } } },
	{ { { 44, 0, 2 }, { 26, 63, 0 } } },
	{ { { 45, 0, 1 }, { 27, 63, 1 } } },
	{ { { 45, 0, 0 }, { 27, 63, 0 } } },
	{ { { 45, 0, 1 }, { 28, 63, 1 } } },
	{ { { 45, 0, 2 }, { 28, 63, 0 } } },
	{ { { 46, 0, 1 }, { 29, 63, 1 } } },
	{ { { 46, 0, 0 }, { 29, 63, 0 } } },
	{ { { 46, 0, 1 }, { 30, 63, 1 } } },
	{ { { 46, 0, 2 }, { 30, 63, 0 } } },
	{ { { 47, 0, 1 }, { 31, 63, 1 } } },
	{ { { 47, 0, 0 }, { 31, 63, 0 } } },
	{ { { 47, 0, 1 }, { 32, 63, 1 } } },
	{ { { 47, 0, 2 }, { 32, 63, 0 } } },
	{ { { 48, 0, 2 }, { 33, 63, 1 } } },
	{ { { 48, 0, 1 }, { 33, 63, 0 } } },
	{ { { 48, 0, 0 }, { 48, 48, 0 } } },
	{ { { 48, 0, 1 }, { 34, 63, 0 } } },
	{ { { 48, 0, 2 }, { 48, 49, 0 } } },
	{ { { 49, 0, 1 }, { 35, 63, 0 } } },
	{ { { 49, 0, 0 }, { 48, 50, 0 } } },
	{ { { 49, 0, 1 }, { 36, 63, 0 } } },
	{ { { 49, 0, 2 }, { 48, 51, 0 } } },
	{ { { 50, 0, 1 }, { 37, 63, 0 } } },
	{ { { 50, 0, 0 }, { 48, 52, 0 } } },
	{ { { 50, 0, 1 }, { 38, 63, 0 } } },
	{ { { 50, 0, 2 }, { 48, 53, 0 } } },
	{ { { 51, 0, 1 }, { 39, 63, 0 } } },
	{ { { 51, 0, 0 }, { 48, 54, 0 } } },
	{ { { 51, 0, 1 }, { 40, 63, 0 } } },
	{ { { 51, 0, 2 }, { 48, 55, 0 } } },
	{ { { 52, 0, 1 }, { 41, 63, 0 } } },
	{ { { 52, 0, 0 }, { 48, 56, 0 } } },
	{ { { 52, 0, 1 }, { 42, 63, 0 } } },
	{ { { 52, 0, 2 }, { 48, 57, 0 } } },
	{ { { 53, 0, 1 }, { 43, 63, 0 } } },
	{ { { 53, 0, 0 }, { 48, 58, 0 } } },
	{ { { 53, 0, 1 }, { 44, 63, 0 } } },
	{ { { 53, 0, 2 }, { 48, 59, 0 } } },
	{ { { 54, 0, 1 }, { 45, 63, 0 } } },
	{ { { 54, 0, 0 }, { 48, 60, 0 } } },
	{ { { 54, 0, 1 }, { 46, 63, 0 } } },
	{ { { 54, 0, 2 }, { 48, 61, 0 } } },
	{ { { 55, 0, 1 }, { 47, 63, 0 } } },
	{ { { 55, 0, 0 }, { 48, 62, 0 } } },
	{ { { 55, 0, 1 }, { 48, 63, 1 } } },
	{ { { 55, 0, 2 }, { 48, 63, 0 } } },
	{ { { 56, 0, 1 }, { 49, 63, 1 } } },
	{ { { 56, 0, 0 }, { 49, 63, 0 } } },
	{ { { 56, 0, 1 }, { 50, 63, 1 } } },
	{ { { 56, 0, 2 }, { 50, 63, 0 } } },
	{ { { 57, 0, 1 }, { 51, 63, 1 } } },
	{ { { 57, 0, 0 }, { 51, 63, 0 } } },
	{ { { 57, 0, 1 }, { 52, 63, 1 } } },
	{ { { 57, 0, 2 }, { 52, 63, 0 } } },
	{ { { 58, 0, 1 }, { 53, 63, 1 } } },
	{ { { 58, 0, 0 }, { 53, 63, 0 } } },
	{ { { 58, 0, 1 }, { 54, 63, 1 } } },
	{ { { 58, 0, 2 }, { 54, 63, 0 } } },
	{ { { 59, 0, 1 }, { 55, 63, 1 } } },
	{ { { 59, 0, 0 }, { 55, 63, 0 } } },
	{ { { 59, 0, 1 }, { 56, 63, 1 } } },
	{ { { 59, 0, 2 }, { 56, 63, 0 } } },
	{ { { 60, 0, 1 }, { 57, 63, 1 } } },
	{ { { 60, 0, 0 }, { 57, 63, 0 } } },
	{ { { 60, 0, 1 }, { 58, 63, 1 } } },
	{ { { 60, 0, 2 }, { 58, 63, 0 } } },
	{ { { 61, 0, 1 }, { 59, 63, 1 } } },
	{ { { 61, 0, 0 }, { 59, 63, 0 } } },
	{ { { 61, 0, 1 }, { 60, 63, 1 } } },
	{ { { 61, 0, 2 }, { 60, 63, 0 } } },
	{ { { 62, 0, 1 }, { 61, 63, 1 } } },
	{ { { 62, 0, 0 }, { 61, 63, 0 } } },
	{ { { 62, 0, 1 }, { 62, 63, 1 } } },
	{ { { 62, 0, 2 }, { 62, 63, 0 } } },
	{ { { 63, 0, 1 }, { 63, 63, 1 } } },
	{ { { 63, 0, 0 }, { 63, 63, 0 } } }
};

SingleColourLookup const lookup_5_4[] =
{
	{ { { 0, 0, 0 }, { 0, 0, 0 } } },
	{ { { 0, 0, 1 }, { 0, 1, 1 } } },
	{ { { 0, 0, 2 }, { 0, 1, 0 } } },
	{ { { 0, 0, 3 }, { 0, 1, 1 } } },
	{ { { 0, 0, 4 }, { 0, 2, 1 } } },
	{ { { 1, 0, 3 }, { 0, 2, 0 } } },
	{ { { 1, 0, 2 }, { 0, 2, 1 } } },
	{ { { 1, 0, 1 }, { 0, 3, 1 } } },
	{ { { 1, 0, 0 }, { 0, 3, 0 } } },
	{ { { 1, 0, 1 }, { 1, 2, 1 } } },
	{ { { 1, 0, 2 }, { 1, 2, 0 } } },
	{ { { 1, 0, 3 }, { 0, 4, 0 } } },
	{ { { 1, 0, 4 }, { 0, 5, 1 } } },
	{ { { 2, 0, 3 }, { 0, 5, 0 } } },
	{ { { 2, 0, 2 }, { 0, 5, 1 } } },
	{ { { 2, 0, 1 }, { 0, 6, 1 } } },
	{ { { 2, 0, 0 }, { 0, 6, 0 } } },
	{ { { 2, 0, 1 }, { 2, 3, 1 } } },
	{ { { 2, 0, 2 }, { 2, 3, 0 } } },
	{ { { 2, 0, 3 }, { 0, 7, 0 } } },
	{ { { 2, 0, 4 }, { 1, 6, 1 } } },
	{ { { 3, 0, 3 }, { 1, 6, 0 } } },
	{ { { 3, 0, 2 }, { 0, 8, 0 } } },
	{ { { 3, 0, 1 }, { 0, 9, 1 } } },
	{ { { 3, 0, 0 }, { 0, 9, 0 } } },
	{ { { 3, 0, 1 }, { 0, 9, 1 } } },
	{ { { 3, 0, 2 }, { 0, 10, 1 } } },
	{ { { 3, 0, 3 }, { 0, 10, 0 } } },
	{ { { 3, 0, 4 }, { 2, 7, 1 } } },
	{ { { 4, 0, 4 }, { 2, 7, 0 } } },
	{ { { 4, 0, 3 }, { 0, 11, 0 } } },
	{ { { 4, 0, 2 }, { 1, 10, 1 } } },
	{ { { 4, 0, 1 }, { 1, 10, 0 } } },
	{ { { 4, 0, 0 }, { 0, 12, 0 } } },
	{ { { 4, 0, 1 }, { 0, 13, 1 } } },
	{ { { 4, 0, 2 }, { 0, 13, 0 } } },
	{ { { 4, 0, 3 }, { 0, 13, 1 } } },
	{ { { 4, 0, 4 }, { 0, 14, 1 } } },
	{ { { 5, 0, 3 }, { 0, 14, 0 } } },
	{ { { 5, 0, 2 }, { 2, 11, 1 } } },
	{ { { 5, 0, 1 }, { 2, 11, 0 } } },
	{ { { 5, 0, 0 }, { 0, 15, 0 } } },
	{ { { 5, 0, 1 }, { 1, 14, 1 } } },
	{ { { 5, 0, 2 }, { 1, 14, 0 } } },
	{ { { 5, 0, 3 }, { 0, 16, 0 } } },
	{ { { 5, 0, 4 }, { 0, 17, 1 } } },
	{ { { 6, 0, 3 }, { 0, 17, 0 } } },
	{ { { 6, 0, 2 }, { 0, 17, 1 } } },
	{ { { 6, 0, 1 }, { 0, 18, 1 } } },
	{ { { 6, 0, 0 }, { 0, 18, 0 } } },
	{ { { 6, 0, 1 }, { 2, 15, 1 } } },
	{ { { 6, 0, 2 }, { 2, 15, 0 } } },
	{ { { 6, 0, 3 }, { 0, 19, 0 } } },
	{ { { 6, 0, 4 }, { 1, 18, 1 } } },
	{ { { 7, 0, 3 }, { 1, 18, 0 } } },
	{ { { 7, 0, 2 }, { 0, 20, 0 } } },
	{ { { 7, 0, 1 }, { 0, 21, 1 } } },
	{ { { 7, 0, 0 }, { 0, 21, 0 } } },
	{ { { 7, 0, 1 }, { 0, 21, 1 } } },
	{ { { 7, 0, 2 }, { 0, 22, 1 } } },
	{ { { 7, 0, 3 }, { 0, 22, 0 } } },
	{ { { 7, 0, 4 }, { 2, 19, 1 } } },
	{ { { 8, 0, 4 }, { 2, 19, 0 } } },
	{ { { 8, 0, 3 }, { 0, 23, 0 } } },
	{ { { 8, 0, 2 }, { 1, 22, 1 } } },
	{ { { 8, 0, 1 }, { 1, 22, 0 } } },
	{ { { 8, 0, 0 }, { 0, 24, 0 } } },
	{ { { 8, 0, 1 }, { 0, 25, 1 } } },
	{ { { 8, 0, 2 }, { 0, 25, 0 } } },
	{ { { 8, 0, 3 }, { 0, 25, 1 } } },
	{ { { 8, 0, 4 }, { 0, 26, 1 } } },
	{ { { 9, 0, 3 }, { 0, 26, 0 } } },
	{ { { 9, 0, 2 }, { 2, 23, 1 } } },
	{ { { 9, 0, 1 }, { 2, 23, 0 } } },
	{ { { 9, 0, 0 }, { 0, 27, 0 } } },
	{ { { 9, 0, 1 }, { 1, 26, 1 } } },
	{ { { 9, 0, 2 }, { 1, 26, 0 } } },
	{ { { 9, 0, 3 }, { 0, 28, 0 } } },
	{ { { 9, 0, 4 }, { 0, 29, 1 } } },
	{ { { 10, 0, 3 }, { 0, 29, 0 } } },
	{ { { 10, 0, 2 }, { 0, 29, 1 } } },
	{ { { 10, 0, 1 }, { 0, 30, 1 } } },
	{ { { 10, 0, 0 }, { 0, 30, 0 } } },
	{ { { 10, 0, 1 }, { 2, 27, 1 } } },
	{ { { 10, 0, 2 }, { 2, 27, 0 } } },
	{ { { 10, 0, 3 }, { 0, 31, 0 } } },
	{ { { 10, 0, 4 }, { 1, 30, 1 } } },
	{ { { 11, 0, 3 }, { 1, 30, 0 } } },
	{ { { 11, 0, 2 }, { 4, 24, 0 } } },
	{ { { 11, 0, 1 }, { 1, 31, 1 } } },
	{ { { 11, 0, 0 }, { 1, 31, 0 } } },
	{ { { 11, 0, 1 }, { 1, 31, 1 } } },
	{ { { 11, 0, 2 }, { 2, 30, 1 } } },
	{ { { 11, 0, 3 }, { 2, 30, 0 } } },
	{ { { 11, 0, 4 }, { 2, 31, 1 } } },
	{ { { 12, 0, 4 }, { 2, 31, 0 } } },
	{ { { 12, 0, 3 }, { 4, 27, 0 } } },
	{ { { 12, 0, 2 }, { 3, 30, 1 } } },
	{ { { 12, 0, 1 }, { 3, 30, 0 } } },
	{ { { 12, 0, 0 }, { 4, 28, 0 } } },
	{ { { 12, 0, 1 }, { 3, 31, 1 } } },
	{ { { 12, 0, 2 }, { 3, 31, 0 } } },
	{ { { 12, 0, 3 }, { 3, 31, 1 } } },
	{ { { 12, 0, 4 }, { 4, 30, 1 } } },
	{ { { 13, 0, 3 }, { 4, 30, 0 } } },
	{ { { 13, 0, 2 }, { 6, 27, 1 } } },
	{ { { 13, 0, 1 }, { 6, 27, 0 } } },
	{ { { 13, 0, 0 }, { 4, 31, 0 } } },
	{ { { 13, 0, 1 }, { 5, 30, 1 } } },
	{ { { 13, 0, 2 }, { 5, 30, 0 } } },
	{ { { 13, 0, 3 }, { 8, 24, 0 } } },
	{ { { 13, 0, 4 }, { 5, 31, 1 } } },
	{ { { 14, 0, 3 }, { 5, 31, 0 } } },
	{ { { 14, 0, 2 }, { 5, 31, 1 } } },
	{ { { 14, 0, 1 }, { 6, 30, 1 } } },
	{ { { 14, 0, 0 }, { 6, 30, 0 } } },
	{ { { 14, 0, 1 }, { 6, 31, 1 } } },
	{ { { 14, 0, 2 }, { 6, 31, 0 } } },
	{ { { 14, 0, 3 }, { 8, 27, 0 } } },
	{ { { 14, 0, 4 }, { 7, 30, 1 } } },
	{ { { 15, 0, 3 }, { 7, 30, 0 } } },
	{ { { 15, 0, 2 }, { 8, 28, 0 } } },
	{ { { 15, 0, 1 }, { 7, 31, 1 } } },
	{ { { 15, 0, 0 }, { 7, 31, 0 } } },
	{ { { 15, 0, 1 }, { 7, 31, 1 } } },
	{ { { 15, 0, 2 }, { 8, 30, 1 } } },
	{ { { 15, 0, 3 }, { 8, 30, 0 } } },
	{ { { 15, 0, 4 }, { 10, 27, 1 } } },
	{ { { 16, 0, 4 }, { 10, 27, 0 } } },
	{ { { 16, 0, 3 }, { 8, 31, 0 } } },
	{ { { 16, 0, 2 }, { 9, 30, 1 } } },
	{ { { 16, 0, 1 }, { 9, 30, 0 } } },
	{ { { 16, 0, 0 }, { 12, 24, 0 } } },
	{ { { 16, 0, 1 }, { 9, 31, 1 } } },
	{ { { 16, 0, 2 }, { 9, 31, 0 } } },
	{ { { 16, 0, 3 }, { 9, 31, 1 } } },
	{ { { 16, 0, 4 }, { 10, 30, 1 } } },
	{ { { 17, 0, 3 }, { 10, 30, 0 } } },
	{ { { 17, 0, 2 }, { 10, 31, 1 } } },
	{ { { 17, 0, 1 }, { 10, 31, 0 } } },
	{ { { 17, 0, 0 }, { 12, 27, 0 } } },
	{ { { 17, 0, 1 }, { 11, 30, 1 } } },
	{ { { 17, 0, 2 }, { 11, 30, 0 } } },
	{ { { 17, 0, 3 }, { 12, 28, 0 } } },
	{ { { 17, 0, 4 }, { 11, 31, 1 } } },
	{ { { 18, 0, 3 }, { 11, 31, 0 } } },
	{ { { 18, 0, 2 }, { 11, 31, 1 } } },
	{ { { 18, 0, 1 }, { 12, 30, 1 } } },
	{ { { 18, 0, 0 }, { 12, 30, 0 } } },
	{ { { 18, 0, 1 }, { 14, 27, 1 } } },
	{ { { 18, 0, 2 }, { 14, 27, 0 } } },
	{ { { 18, 0, 3 }, { 12, 31, 0 } } },
	{ { { 18, 0, 4 }, { 13, 30, 1 } } },
	{ { { 19, 0, 3 }, { 13, 30, 0 } } },
	{ { { 19, 0, 2 }, { 16, 24, 0 } } },
	{ { { 19, 0, 1 }, { 13, 31, 1 } } },
	{ { { 19, 0, 0 }, { 13, 31, 0 } } },
	{ { { 19, 0, 1 }, { 13, 31, 1 } } },
	{ { { 19, 0, 2 }, { 14, 30, 1 } } },
	{ { { 19, 0, 3 }, { 14, 30, 0 } } },
	{ { { 19, 0, 4 }, { 14, 31, 1 } } },
	{ { { 20, 0, 4 }, { 14, 31, 0 } } },
	{ { { 20, 0, 3 }, { 16, 27, 0 } } },
	{ { { 20, 0, 2 }, { 15, 30, 1 } } },
	{ { { 20, 0, 1 }, { 15, 30, 0 } } },
	{ { { 20, 0, 0 }, { 16, 28, 0 } } },
	{ { { 20, 0, 1 }, { 15, 31, 1 } } },
	{ { { 20, 0, 2 }, { 15, 31, 0 } } },
	{ { { 20, 0, 3 }, { 15, 31, 1 } } },
	{ { { 20, 0, 4 }, { 16, 30, 1 } } },
	{ { { 21, 0, 3 }, { 16, 30, 0 } } },
	{ { { 21, 0, 2 }, { 18, 27, 1 } } },
	{ { { 21, 0, 1 }, { 18, 27, 0 } } },
	{ { { 21, 0, 0 }, { 16, 31, 0 } } },
	{ { { 21, 0, 1 }, { 17, 30, 1 } } },
	{ { { 21, 0, 2 }, { 17, 30, 0 } } },
	{ { { 21, 0, 3 }, { 20, 24, 0 } } },
	{ { { 21, 0, 4 }, { 17, 31, 1 } } },
	{ { { 22, 0, 3 }, { 17, 31, 0 } } },
	{ { { 22, 0, 2 }, { 17, 31, 1 } } },
	{ { { 22, 0, 1 }, { 18, 30, 1 } } },
	{ { { 22, 0, 0 }, { 18, 30, 0 } } },
	{ { { 22, 0, 1 }, { 18, 31, 1 } } },
	{ { { 22, 0, 2 }, { 18, 31, 0 } } },
	{ { { 22, 0, 3 }, { 20, 27, 0 } } },
	{ { { 22, 0, 4 }, { 19, 30, 1 } } },
	{ { { 23, 0, 3 }, { 19, 30, 0 } } },
	{ { { 23, 0, 2 }, { 20, 28, 0 } } },
	{ { { 23, 0, 1 }, { 19, 31, 1 } } },
	{ { { 23, 0, 0 }, { 19, 31, 0 } } },
	{ { { 23, 0, 1 }, { 19, 31, 1 } } },
	{ { { 23, 0, 2 }, { 20, 30, 1 } } },
	{ { { 23, 0, 3 }, { 20, 30, 0 } } },
	{ { { 23, 0, 4 }, { 22, 27, 1 } } },
	{ { { 24, 0, 4 }, { 22, 27, 0 } } },
	{ { { 24, 0, 3 }, { 20, 31, 0 } } },
	{ { { 24, 0, 2 }, { 21, 30, 1 } } },
	{ { { 24, 0, 1 }, { 21, 30, 0 } } },
	{ { { 24, 0, 0 }, { 24, 24, 0 } } },
	{ { { 24, 0, 1 }, { 21, 31, 1 } } },
	{ { { 24, 0, 2 }, { 21, 31, 0 } } },
	{ { { 24, 0, 3 }, { 21, 31, 1 } } },
	{ { { 24, 0, 4 }, { 22, 30, 1 } } },
	{ { { 25, 0, 3 }, { 22, 30, 0 } } },
	{ { { 25, 0, 2 }, { 22, 31, 1 } } },
	{ { { 25, 0, 1 }, { 22, 31, 0 } } },
	{ { { 25, 0, 0 }, { 24, 27, 0 } } },
	{ { { 25, 0, 1 }, { 23, 30, 1 } } },
	{ { { 25, 0, 2 }, { 23, 30, 0 } } },
	{ { { 25, 0, 3 }, { 24, 28, 0 } } },
	{ { { 25, 0, 4 }, { 23, 31, 1 } } },
	{ { { 26, 0, 3 }, { 23, 31, 0 } } },
	{ { { 26, 0, 2 }, { 23, 31, 1 } } },
	{ { { 26, 0, 1 }, { 24, 30, 1 } } },
	{ { { 26, 0, 0 }, { 24, 30, 0 } } },
	{ { { 26, 0, 1 }, { 26, 27, 1 } } },
	{ { { 26, 0, 2 }, { 26, 27, 0 } } },
	{ { { 26, 0, 3 }, { 24, 31, 0 } } },
	{ { { 26, 0, 4 }, { 25, 30, 1 } } },
	{ { { 27, 0, 3 }, { 25, 30, 0 } } },
	{ { { 27, 0, 2 }, { 28, 24, 0 } } },
	{ { { 27, 0, 1 }, { 25, 31, 1 } } },
	{ { { 27, 0, 0 }, { 25, 31, 0 } } },
	{ { { 27, 0, 1 }, { 25, 31, 1 } } },
	{ { { 27, 0, 2 }, { 26, 30, 1 } } },
	{ { { 27, 0, 3 }, { 26, 30, 0 } } },
	{ { { 27, 0, 4 }, { 26, 31, 1 } } },
	{ { { 28, 0, 4 }, { 26, 31, 0 } } },
	{ { { 28, 0, 3 }, { 28, 27, 0 } } },
	{ { { 28, 0, 2 }, { 27, 30, 1 } } },
	{ { { 28, 0, 1 }, { 27, 30, 0 } } },
	{ { { 28, 0, 0 }, { 28, 28, 0 } } },
	{ { { 28, 0, 1 }, { 27, 31, 1 } } },
	{ { { 28, 0, 2 }, { 27, 31, 0 } } },
	{ { { 28, 0, 3 }, { 27, 31, 1 } } },
	{ { { 28, 0, 4 }, { 28, 30, 1 } } },
	{ { { 29, 0, 3 }, { 28, 30, 0 } } },
	{ { { 29, 0, 2 }, { 30, 27, 1 } } },
	{ { { 29, 0, 1 }, { 30, 27, 0 } } },
	{ { { 29, 0, 0 }, { 28, 31, 0 } } },
	{ { { 29, 0, 1 }, { 29, 30, 1 } } },
	{ { { 29, 0, 2 }, { 29, 30, 0 } } },
	{ { { 29, 0, 3 }, { 29, 30, 1 } } },
	{ { { 29, 0, 4 }, { 29, 31, 1 } } },
	{ { { 30, 0, 3 }, { 29, 31, 0 } } },
	{ { { 30, 0, 2 }, { 29, 31, 1 } } },
	{ { { 30, 0, 1 }, { 30, 30, 1 } } },
	{ { { 30, 0, 0 }, { 30, 30, 0 } } },
	{ { { 30, 0, 1 }, { 30, 31, 1 } } },
	{ { { 30, 0, 2 }, { 30, 31, 0 } } },
	{ { { 30, 0, 3 }, { 30, 31, 1 } } },
	{ { { 30, 0, 4 }, { 31, 30, 1 } } },
	{ { { 31, 0, 3 }, { 31, 30, 0 } } },
	{ { { 31, 0, 2 }, { 31, 30, 1 } } },
	{ { { 31, 0, 1 }, { 31, 31, 1 } } },
	{ { { 31, 0, 0 }, { 31, 31, 0 } } }
};

SingleColourLookup const lookup_6_4[] =
{
	{ { { 0, 0, 0 }, { 0, 0, 0 } } },
	{ { { 0, 0, 1 }, { 0, 1, 0 } } },
	{ { { 0, 0, 2 }, { 0, 2, 0 } } },
	{ { { 1, 0, 1 }, { 0, 3, 1 } } },
	{ { { 1, 0, 0 }, { 0, 3, 0 } } },
	{ { { 1, 0, 1 }, { 0, 4, 0 } } },
	{ { { 1, 0, 2 }, { 0, 5, 0 } } },
	{ { { 2, 0, 1 }, { 0, 6, 1 } } },
	{ { { 2, 0, 0 }, { 0, 6, 0 } } },
	{ { { 2, 0, 1 }, { 0, 7, 0 } } },
	{ { { 2, 0, 2 }, { 0, 8, 0 } } },
	{ { { 3, 0, 1 }, { 0, 9, 1 } } },
	{ { { 3, 0, 0 }, { 0, 9, 0 } } },
	{ { { 3, 0, 1 }, { 0, 10, 0 } } },
	{ { { 3, 0, 2 }, { 0, 11, 0 } } },
	{ { { 4, 0, 1 }, { 0, 12, 1 } } },
	{ { { 4, 0, 0 }, { 0, 12, 0 } } },
	{ { { 4, 0, 1 }, { 0, 13, 0 } } },
	{ { { 4, 0, 2 }, { 0, 14, 0 } } },
	{ { { 5, 0, 1 }, { 0, 15, 1 } } },
	{ { { 5, 0, 0 }, { 0, 15, 0 } } },
	{ { { 5, 0, 1 }, { 0, 16, 0 } } },
	{ { { 5, 0, 2 }, { 1, 15, 0 } } },
	{ { { 6, 0, 1 }, { 0, 17, 0 } } },
	{ { { 6, 0, 0 }, { 0, 18, 0 } } },
	{ { { 6, 0, 1 }, { 0, 19, 0 } } },
	{ { { 6, 0, 2 }, { 3, 14, 0 } } },
	{ { { 7, 0, 1 }, { 0, 20, 0 } } },
	{ { { 7, 0, 0 }, { 0, 21, 0 } } },
	{ { { 7, 0, 1 }, { 0, 22, 0 } } },
	{ { { 7, 0, 2 }, { 4, 15, 0 } } },
	{ { { 8, 0, 1 }, { 0, 23, 0 } } },
	{ { { 8, 0, 0 }, { 0, 24, 0 } } },
	{ { { 8, 0, 1 }, { 0, 25, 0 } } },
	{ { { 8, 0, 2 }, { 6, 14, 0 } } },
	{ { { 9, 0, 1 }, { 0, 26, 0 } } },
	{ { { 9, 0, 0 }, { 0, 27, 0 } } },
	{ { { 9, 0, 1 }, { 0, 28, 0 } } },
	{ { { 9, 0, 2 }, { 7, 15, 0 } } },
	{ { { 10, 0, 1 }, { 0, 29, 0 } } },
	{ { { 10, 0, 0 }, { 0, 30, 0 } } },
	{ { { 10, 0, 1 }, { 0, 31, 0 } } },
	{ { { 10, 0, 2 }, { 9, 14, 0 } } },
	{ { { 11, 0, 1 }, { 0, 32, 0 } } },
	{ { { 11, 0, 0 }, { 0, 33, 0 } } },
	{ { { 11, 0, 1 }, { 2, 30, 0 } } },
	{ { { 11, 0, 2 }, { 0, 34, 0 } } },
	{ { { 12, 0, 1 }, { 0, 35, 0 } } },
	{ { { 12, 0, 0 }, { 0, 36, 0 } } },
	{ { { 12, 0, 1 }, { 3, 31, 0 } } },
	{ { { 12, 0, 2 }, { 0, 37, 0 } } },
	{ { { 13, 0, 1 }, { 0, 38, 0 } } },
	{ { { 13, 0, 0 }, { 0, 39, 0 } } },
	{ { { 13, 0, 1 }, { 5, 30, 0 } } },
	{ { { 13, 0, 2 }, { 0, 40, 0 } } },
	{ { { 14, 0, 1 }, { 0, 41, 0 } } },
	{ { { 14, 0, 0 }, { 0, 42, 0 } } },
	{ { { 14, 0, 1 }, { 6, 31, 0 } } },
	{ { { 14, 0, 2 }, { 0, 43, 0 } } },
	{ { { 15, 0, 1 }, { 0, 44, 0 } } },
	{ { { 15, 0, 0 }, { 0, 45, 0 } } },
	{ { { 15, 0, 1 }, { 8, 30, 0 } } },
	{ { { 15, 0, 2 }, { 0, 46, 0 } } },
	{ { { 16, 0, 2 }, { 0, 47, 0 } } },
	{ { { 16, 0, 1 }, { 1, 46, 0 } } },
	{ { { 16, 0, 0 }, { 0, 48, 0 } } },
	{ { { 16, 0, 1 }, { 0, 49, 0 } } },
	{ { { 16, 0, 2 }, { 0, 50, 0 } } },
	{ { { 17, 0, 1 }, { 2, 47, 0 } } },
	{ { { 17, 0, 0 }, { 0, 51, 0 } } },
	{ { { 17, 0, 1 }, { 0, 52, 0 } } },
	{ { { 17, 0, 2 }, { 0, 53, 0 } } },
	{ { { 18, 0, 1 }, { 4, 46, 0 } } },
	{ { { 18, 0, 0 }, { 0, 54, 0 } } },
	{ { { 18, 0, 1 }, { 0, 55, 0 } } },
	{ { { 18, 0, 2 }, { 0, 56, 0 } } },
	{ { { 19, 0, 1 }, { 5, 47, 0 } } },
	{ { { 19, 0, 0 }, { 0, 57, 0 } } },
	{ { { 19, 0, 1 }, { 0, 58, 0 } } },
	{ { { 19, 0, 2 }, { 0, 59, 0 } } },
	{ { { 20, 0, 1 }, { 7, 46, 0 } } },
	{ { { 20, 0, 0 }, { 0, 60, 0 } } },
	{ { { 20, 0, 1 }, { 0, 61, 0 } } },
	{ { { 20, 0, 2 }, { 0, 62, 0 } } },
	{ { { 21, 0, 1 }, { 8, 47, 0 } } },
	{ { { 21, 0, 0 }, { 0, 63, 0 } } },
	{ { { 21, 0, 1 }, { 1, 62, 0 } } },
	{ { { 21, 0, 2 }, { 1, 63, 0 } } },
	{ { { 22, 0, 1 }, { 10, 46, 0 } } },
	{ { { 22, 0, 0 }, { 2, 62, 0 } } },
	{ { { 22, 0, 1 }, { 2, 63, 0 } } },
	{ { { 22, 0, 2 }, { 3, 62, 0 } } },
	{ { { 23, 0, 1 }, { 11, 47, 0 } } },
	{ { { 23, 0, 0 }, { 3, 63, 0 } } },
	{ { { 23, 0, 1 }, { 4, 62, 0 } } },
	{ { { 23, 0, 2 }, { 4, 63, 0 } } },
	{ { { 24, 0, 1 }, { 13, 46, 0 } } },
	{ { { 24, 0, 0 }, { 5, 62, 0 } } },
	{ { { 24, 0, 1 }, { 5, 63, 0 } } },
	{ { { 24, 0, 2 }, { 6, 62, 0 } } },
	{ { { 25, 0, 1 }, { 14, 47, 0 } } },
	{ { { 25, 0, 0 }, { 6, 63, 0 } } },
	{ { { 25, 0, 1 }, { 7, 62, 0 } } },
	{ { { 25, 0, 2 }, { 7, 63, 0 } } },
	{ { { 26, 0, 1 }, { 16, 45, 0 } } },
	{ { { 26, 0, 0 }, { 8, 62, 0 } } },
	{ { { 26, 0, 1 }, { 8, 63, 0 } } },
	{ { { 26, 0, 2 }, { 9, 62, 0 } } },
	{ { { 27, 0, 1 }, { 16, 48, 0 } } },
	{ { { 27, 0, 0 }, { 9, 63, 0 } } },
	{ { { 27, 0, 1 }, { 10, 62, 0 } } },
	{ { { 27, 0, 2 }, { 10, 63, 0 } } },
	{ { { 28, 0, 1 }, { 16, 51, 0 } } },
	{ { { 28, 0, 0 }, { 11, 62, 0 } } },
	{ { { 28, 0, 1 }, { 11, 63, 0 } } },
	{ { { 28, 0, 2 }, { 12, 62, 0 } } },
	{ { { 29, 0, 1 }, { 16, 54, 0 } } },
	{ { { 29, 0, 0 }, { 12, 63, 0 } } },
	{ { { 29, 0, 1 }, { 13, 62, 0 } } },
	{ { { 29, 0, 2 }, { 13, 63, 0 } } },
	{ { { 30, 0, 1 }, { 16, 57, 0 } } },
	{ { { 30, 0, 0 }, { 14, 62, 0 } } },
	{ { { 30, 0, 1 }, { 14, 63, 0 } } },
	{ { { 30, 0, 2 }, { 15, 62, 0 } } },
	{ { { 31, 0, 1 }, { 16, 60, 0 } } },
	{ { { 31, 0, 0 }, { 15, 63, 0 } } },
	{ { { 31, 0, 1 }, { 24, 46, 0 } } },
	{ { { 31, 0, 2 }, { 16, 62, 0 } } },
	{ { { 32, 0, 2 }, { 16, 63, 0 } } },
	{ { { 32, 0, 1 }, { 17, 62, 0 } } },
	{ { { 32, 0, 0 }, { 25, 47, 0 } } },
	{ { { 32, 0, 1 }, { 17, 63, 0 } } },
	{ { { 32, 0, 2 }, { 18, 62, 0 } } },
	{ { { 33, 0, 1 }, { 18, 63, 0 } } },
	{ { { 33, 0, 0 }, { 27, 46, 0 } } },
	{ { { 33, 0, 1 }, { 19, 62, 0 } } },
	{ { { 33, 0, 2 }, { 19, 63, 0 } } },
	{ { { 34, 0, 1 }, { 20, 62, 0 } } },
	{ { { 34, 0, 0 }, { 28, 47, 0 } } },
	{ { { 34, 0, 1 }, { 20, 63, 0 } } },
	{ { { 34, 0, 2 }, { 21, 62, 0 } } },
	{ { { 35, 0, 1 }, { 21, 63, 0 } } },
	{ { { 35, 0, 0 }, { 30, 46, 0 } } },
	{ { { 35, 0, 1 }, { 22, 62, 0 } } },
	{ { { 35, 0, 2 }, { 22, 63, 0 } } },
	{ { { 36, 0, 1 }, { 23, 62, 0 } } },
	{ { { 36, 0, 0 }, { 31, 47, 0 } } },
	{ { { 36, 0, 1 }, { 23, 63, 0 } } },
	{ { { 36, 0, 2 }, { 24, 62, 0 } } },
	{ { { 37, 0, 1 }, { 24, 63, 0 } } },
	{ { { 37, 0, 0 }, { 32, 47, 0 } } },
	{ { { 37, 0, 1 }, { 25, 62, 0 } } },
	{ { { 37, 0, 2 }, { 25, 63, 0 } } },
	{ { { 38, 0, 1 }, { 26, 62, 0 } } },
	{ { { 38, 0, 0 }, { 32, 50, 0 } } },
	{ { { 38, 0, 1 }, { 26, 63, 0 } } },
	{ { { 38, 0, 2 }, { 27, 62, 0 } } },
	{ { { 39, 0, 1 }, { 27, 63, 0 } } },
	{ { { 39, 0, 0 }, { 32, 53, 0 } } },
	{ { { 39, 0, 1 }, { 28, 62, 0 } } },
	{ { { 39, 0, 2 }, { 28, 63, 0 } } },
	{ { { 40, 0, 1 }, { 29, 62, 0 } } },
	{ { { 40, 0, 0 }, { 32, 56, 0 } } },
	{ { { 40, 0, 1 }, { 29, 63, 0 } } },
	{ { { 40, 0, 2 }, { 30, 62, 0 } } },
	{ { { 41, 0, 1 }, { 30, 63, 0 } } },
	{ { { 41, 0, 0 }, { 32, 59, 0 } } },
	{ { { 41, 0, 1 }, { 31, 62, 0 } } },
	{ { { 41, 0, 2 }, { 31, 63, 0 } } },
	{ { { 42, 0, 1 }, { 32, 61, 0 } } },
	{ { { 42, 0, 0 }, { 32, 62, 0 } } },
	{ { { 42, 0, 1 }, { 32, 63, 0 } } },
	{ { { 42, 0, 2 }, { 41, 46, 0 } } },
	{ { { 43, 0, 1 }, { 33, 62, 0 } } },
	{ { { 43, 0, 0 }, { 33, 63, 0 } } },
	{ { { 43, 0, 1 }, { 34, 62, 0 } } },
	{ { { 43, 0, 2 }, { 42, 47, 0 } } },
	{ { { 44, 0, 1 }, { 34, 63, 0 } } },
	{ { { 44, 0, 0 }, { 35, 62, 0 } } },
	{ { { 44, 0, 1 }, { 35, 63, 0 } } },
	{ { { 44, 0, 2 }, { 44, 46, 0 } } },
	{ { { 45, 0, 1 }, { 36, 62, 0 } } },
	{ { { 45, 0, 0 }, { 36, 63, 0 } } },
	{ { { 45, 0, 1 }, { 37, 62, 0 } } },
	{ { { 45, 0, 2 }, { 45, 47, 0 } } },
	{ { { 46, 0, 1 }, { 37, 63, 0 } } },
	{ { { 46, 0, 0 }, { 38, 62, 0 } } },
	{ { { 46, 0, 1 }, { 38, 63, 0 } } },
	{ { { 46, 0, 2 }, { 47, 46, 0 } } },
	{ { { 47, 0, 1 }, { 39, 62, 0 } } },
	{ { { 47, 0, 0 }, { 39, 63, 0 } } },
	{ { { 47, 0, 1 }, { 40, 62, 0 } } },
	{ { { 47, 0, 2 }, { 48, 46, 0 } } },
	{ { { 48, 0, 2 }, { 40, 63, 0 } } },
	{ { { 48, 0, 1 }, { 41, 62, 0 } } },
	{ { { 48, 0, 0 }, { 41, 63, 0 } } },
	{ { { 48, 0, 1 }, { 48, 49, 0 } } },
	{ { { 48, 0, 2 }, { 42, 62, 0 } } },
	{ { { 49, 0, 1 }, { 42, 63, 0 } } },
	{ { { 49, 0, 0 }, { 43, 62, 0 } } },
	{ { { 49, 0, 1 }, { 48, 52, 0 } } },
	{ { { 49, 0, 2 }, { 43, 63, 0 } } },
	{ { { 50, 0, 1 }, { 44, 62, 0 } } },
	{ { { 50, 0, 0 }, { 44, 63, 0 } } },
	{ { { 50, 0, 1 }, { 48, 55, 0 } } },
	{ { { 50, 0, 2 }, { 45, 62, 0 } } },
	{ { { 51, 0, 1 }, { 45, 63, 0 } } },
	{ { { 51, 0, 0 }, { 46, 62, 0 } } },
	{ { { 51, 0, 1 }, { 48, 58, 0 } } },
	{ { { 51, 0, 2 }, { 46, 63, 0 } } },
	{ { { 52, 0, 1 }, { 47, 62, 0 } } },
	{ { { 52, 0, 0 }, { 47, 63, 0 } } },
	{ { { 52, 0, 1 }, { 48, 61, 0 } } },
	{ { { 52, 0, 2 }, { 48, 62, 0 } } },
	{ { { 53, 0, 1 }, { 56, 47, 0 } } },
	{ { { 53, 0, 0 }, { 48, 63, 0 } } },
	{ { { 53, 0, 1 }, { 49, 62, 0 } } },
	{ { { 53, 0, 2 }, { 49, 63, 0 } } },
	{ { { 54, 0, 1 }, { 58, 46, 0 } } },
	{ { { 54, 0, 0 }, { 50, 62, 0 } } },
	{ { { 54, 0, 1 }, { 50, 63, 0 } } },
	{ { { 54, 0, 2 }, { 51, 62, 0 } } },
	{ { { 55, 0, 1 }, { 59, 47, 0 } } },
	{ { { 55, 0, 0 }, { 51, 63, 0 } } },
	{ { { 55, 0, 1 }, { 52, 62, 0 } } },
	{ { { 55, 0, 2 }, { 52, 63, 0 } } },
	{ { { 56, 0, 1 }, { 61, 46, 0 } } },
	{ { { 56, 0, 0 }, { 53, 62, 0 } } },
	{ { { 56, 0, 1 }, { 53, 63, 0 } } },
	{ { { 56, 0, 2 }, { 54, 62, 0 } } },
	{ { { 57, 0, 1 }, { 62, 47, 0 } } },
	{ { { 57, 0, 0 }, { 54, 63, 0 } } },
	{ { { 57, 0, 1 }, { 55, 62, 0 } } },
	{ { { 57, 0, 2 }, { 55, 63, 0 } } },
	{ { { 58, 0, 1 }, { 56, 62, 1 } } },
	{ { { 58, 0, 0 }, { 56, 62, 0 } } },
	{ { { 58, 0, 1 }, { 56, 63, 0 } } },
	{ { { 58, 0, 2 }, { 57, 62, 0 } } },
	{ { { 59, 0, 1 }, { 57, 63, 1 } } },
	{ { { 59, 0, 0 }, { 57, 63, 0 } } },
	{ { { 59, 0, 1 }, { 58, 62, 0 } } },
	{ { { 59, 0, 2 }, { 58, 63, 0 } } },
	{ { { 60, 0, 1 }, { 59, 62, 1 } } },
	{ { { 60, 0, 0 }, { 59, 62, 0 } } },
	{ { { 60, 0, 1 }, { 59, 63, 0 } } },
	{ { { 60, 0, 2 }, { 60, 62, 0 } } },
	{ { { 61, 0, 1 }, { 60, 63, 1 } } },
	{ { { 61, 0, 0 }, { 60, 63, 0 } } },
	{ { { 61, 0, 1 }, { 61, 62, 0 } } },
	{ { { 61, 0, 2 }, { 61, 63, 0 } } },
	{ { { 62, 0, 1 }, { 62, 62, 1 } } },
	{ { { 62, 0, 0 }, { 62, 62, 0 } } },
	{ { { 62, 0, 1 }, { 62, 63, 0 } } },
	{ { { 62, 0, 2 }, { 63, 62, 0 } } },
	{ { { 63, 0, 1 }, { 63, 63, 1 } } },
	{ { { 63, 0, 0 }, { 63, 63, 0 } } }
};
