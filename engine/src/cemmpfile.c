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
 *  1. DDS WIC Codec
 *     Copyright (C) 2006 Simon Brown <si@sjbrown.co.uk>
 *     http://code.google.com/p/dds-wic-codec/
*/

/*
 *  See doc/formats/mmpfile.txt for more details.
*/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "celib.h"
#include "cevec3.h"
#include "cebyteorder.h"
#include "cemmpfile.h"

void ce_mmpfile_decompress_pnt3(void* dst, const void* src, int size)
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

int dxt_get_storage_requirements(int width, int height, int format)
{
	assert(CE_MMP_DXT1 == format || CE_MMP_DXT3 == format);
	int block_size = CE_MMP_DXT1 == format ? 8 : 16;
	int block_count = ((width + 3) >> 2) * ((height + 3) >> 2);
	return block_size * block_count;
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
	bool isDxt1 = CE_MMP_DXT1 == format;
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

	if( CE_MMP_DXT3 == format )
		colourBlock = (uint8_t*)block + 8;

	// create the minimal point set
	ce_mmpfile_colorset cs;
	ce_mmpfile_colorset_init(&cs, rgba, mask, format);

	// check the compression type and compress colour
	if (1 == cs.count)
	{
		// always do a single colour fit
		//SingleColourFit fit( &colours, flags );
		//fit.Compress( colourBlock );
	}
	else if( 0 == cs.count )
	{
		// do a range fit
		//RangeFit fit( &colours, flags );
		//fit.Compress( colourBlock );
	}
	else
	{
		// default to a cluster fit (could be iterative or not)
		//ClusterFit fit( &colours, flags );
		//fit.Compress( colourBlock );
	}

	// compress alpha separately if necessary
	if( CE_MMP_DXT3 == format )
		CompressAlphaDxt3( rgba, mask, alphaBock );
}

void dxt_compress_image(void* dst, const void* src,
						int width, int height, int format)
{
	assert(CE_MMP_DXT1 == format || CE_MMP_DXT3 == format);

	const uint8_t* s = src;

	// initialise the block output
	uint8_t* targetBlock = dst;
	int bytesPerBlock = CE_MMP_DXT1 == format ? 8 : 16;

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

	if( CE_MMP_DXT3 == format )
		colourBlock = (uint8_t const*)block + 8;

	// decompress colour
	DecompressColour( rgba, colourBlock, CE_MMP_DXT1 == format );

	// decompress alpha separately if necessary
	if( CE_MMP_DXT3 == format )
		DecompressAlphaDxt3( rgba, alphaBock );
}

void dxt_decompress_image(void* dst, const void* src,
							int width, int height, int format )
{
	assert(CE_MMP_DXT1 == format || CE_MMP_DXT3 == format);

	uint8_t* rgba = dst;

	// initialise the block input
	const uint8_t* sourceBlock = src;
	int bytesPerBlock = CE_MMP_DXT1 == format ? 8 : 16;

	// loop over blocks
	for( int y = 0; y < height; y += 4 )
	{
		for( int x = 0; x < width; x += 4 )
		{
			// decompress the block
			uint8_t targetRgba[4*16];
			Decompress( targetRgba, sourceBlock, format );

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
						uint8_t* targetPixel = rgba + 4*( width*sy + sx );

						// copy the rgba value
						for( int i = 0; i < 4; ++i )
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
			sourceBlock += bytesPerBlock;
		}
	}
}
