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

#ifndef CE_BIKFILE_H
#define CE_BIKFILE_H

#include <stdbool.h>
#include <stdint.h>

#include "cememfile.h"
#include "cevector.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 *  Bink container general routines
 *
 *  See also:
 *  1. http://wiki.multimedia.cx/index.php?title=Bink_Container
 *
 *  For concrete streams, please see corresponding modules.
*/

enum {
	CE_BINK_REVISION_B = 0x62,
	CE_BINK_REVISION_D = 0x64,
	CE_BINK_REVISION_F = 0x66,
	CE_BINK_REVISION_G = 0x67,
	CE_BINK_REVISION_H = 0x68,
	CE_BINK_REVISION_I = 0x69,
};

enum {
	CE_BINK_VIDEO_FLAG_HAS_ALPHA_PLANE = 0x100000,
	CE_BINK_VIDEO_FLAG_GRAYSCALE = 0x20000,
	CE_BINK_VIDEO_FLAG_HEIGHT_DOUBLED = 0,
	CE_BINK_VIDEO_FLAG_HEIGHT_INTERLACED = 0,
	CE_BINK_VIDEO_FLAG_WIDTH_DOUBLED = 0,
	CE_BINK_VIDEO_FLAG_WIDTH_AND_HEIGHT_DOUBLED = 0,
	CE_BINK_VIDEO_FLAG_WIDTH_AND_HEIGHT_INTERLACED = 0,
};

enum {
	// Bink Audio algorithm
	CE_BINK_AUDIO_FLAG_USE_DCT = 0x1000, // FFT otherwise
	CE_BINK_AUDIO_FLAG_STEREO = 0x2000,
	CE_BINK_AUDIO_FLAG_PREFER_16BITS = 0x4000,
};

enum {
	CE_BINK_EXTRADATA_SIZE = 1,
	CE_BINK_MAX_FRAMES = 1000000,
	CE_BINK_MAX_VIDEO_WIDTH = 32767,
	CE_BINK_MAX_VIDEO_HEIGHT = 32767,
	CE_BINK_MAX_AUDIO_TRACKS = 256,
};

/*
 *  Header
*/

typedef struct {
	uint8_t revision;
	uint32_t file_size; // not including the first 8 bytes
	uint32_t frame_count;
	uint32_t largest_frame_size;
	uint32_t last_frame;
	uint32_t video_width;
	uint32_t video_height;
	uint32_t fps_dividend;
	uint32_t fps_divider;
	uint32_t video_flags;
	uint32_t audio_track_count;
} ce_binkheader;

extern bool ce_binkheader_read(ce_binkheader* binkheader, ce_memfile* memfile);

/*
 *  Audio Track
*/

typedef struct {
	uint16_t sample_rate;
	uint16_t flags;
} ce_binktrack;

extern bool ce_binktrack_read(ce_binktrack* binktracks, size_t n, ce_memfile* memfile);
extern bool ce_binktrack_skip(size_t n, ce_memfile* memfile);

/*
 *  Frame Index Table
*/

typedef struct {
	bool keyframe;
	uint32_t pos;
	uint32_t length;
} ce_binkindex;

extern bool ce_binkindex_read(ce_binkindex* binkindices, size_t n, ce_memfile* memfile);

/*
 *  Frame
*/

// layout of the frame, only for illustration
typedef struct {
	struct {
		// a value of zero indicates no audio is present for this track
		uint32_t packet_length; // in bytes plus four bytes
		uint32_t sample_count; // number of samples in packet (in bytes)
		// audio packet here
	} audio_data[CE_BINK_MAX_AUDIO_TRACKS];
	// video packet here
} ce_binkframe;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_BIKFILE_H */
