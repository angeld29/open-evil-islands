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

#include <stdio.h>
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cebyteorder.h"
#include "cebitop.h"
#include "cebink.h"

enum {
	CE_BINK_HEADER_SIZE = 44,
	CE_BINK_AUDIO_HEADER_SIZE = 4 + 4 + 4,
};

bool ce_binkheader_read(ce_binkheader* binkheader, ce_memfile* memfile)
{
	uint8_t header[CE_BINK_HEADER_SIZE];

	if (sizeof(header) != ce_memfile_read(memfile, header, 1, sizeof(header))) {
		return false;
	}

	union {
		uint8_t* u8;
		uint32_t* u32;
	} ptr = {header};

	if ('B' != *ptr.u8++ || 'I' != *ptr.u8++ || 'K' != *ptr.u8++) {
		return false;
	}

	binkheader->revision = *ptr.u8++;
	binkheader->file_size = ce_le2cpu32(*ptr.u32++);
	binkheader->frame_count = ce_le2cpu32(*ptr.u32++);
	binkheader->largest_frame_size = ce_le2cpu32(*ptr.u32++);
	binkheader->last_frame = ce_le2cpu32(*ptr.u32++);
	binkheader->video_width = ce_le2cpu32(*ptr.u32++);
	binkheader->video_height = ce_le2cpu32(*ptr.u32++);
	binkheader->fps_dividend = ce_le2cpu32(*ptr.u32++);
	binkheader->fps_divider = ce_le2cpu32(*ptr.u32++);
	binkheader->video_flags = ce_le2cpu32(*ptr.u32++);
	binkheader->audio_track_count = ce_le2cpu32(*ptr.u32++);

	return true;
}

bool ce_binktrack_read(ce_binktrack* binktrack, ce_memfile* memfile)
{
	uint8_t header[CE_BINK_AUDIO_HEADER_SIZE];

	if (sizeof(header) != ce_memfile_read(memfile, header, 1, sizeof(header))) {
		return false;
	}

	union {
		uint8_t* u8;
		uint16_t* u16;
	} ptr = {header};

	ptr.u8 += 4; // not authoritative, skip

	binktrack->sample_rate = ce_le2cpu16(*ptr.u16++);
	binktrack->flags = ce_le2cpu16(*ptr.u16++);

	ptr.u8 += 4; // not used, skip

	return true;
}

bool ce_binktrack_skip(size_t n, ce_memfile* memfile)
{
	uint8_t header[CE_BINK_AUDIO_HEADER_SIZE * n];
	return sizeof(header) == ce_memfile_read(memfile, header, 1, sizeof(header));
}

bool ce_bink_read_indices(ce_binkindex* binkindices, size_t n, ce_memfile* memfile)
{
	uint32_t pos, next_pos;

	ce_memfile_read(memfile, &next_pos, 4, 1);

	for (size_t i = 0; i < n; ++i) {
		pos = next_pos;
		ce_memfile_read(memfile, &next_pos, 4, 1);

		binkindices[i].keyframe = ce_bittst32(pos, 0);
		binkindices[i].pos = ce_bitclr32(pos, 0);
		binkindices[i].length = ce_bitclr32(next_pos, 0);

		if (binkindices[i].length <= binkindices[i].pos) {
			return false;
		}

		binkindices[i].length -= binkindices[i].pos;

		//ce_logging_debug("pos %u, len %u, kf %d\n", binkindex->pos, binkindex->length, (int)binkindex->keyframe);
	}

	return true;
}

// TODO: for video
	/*if (0 == binkheader->frame_count ||
			0 == binkheader->video_width || 0 == binkheader->video_height ||
			0 == binkheader->fps_dividend || 0 == binkheader->fps_divider ||
			binkheader->largest_frame_size > binkheader->file_size ||
			binkheader->frame_count > CE_BINK_MAX_FRAMES ||
			binkheader->video_width > CE_BINK_MAX_VIDEO_WIDTH ||
			binkheader->video_height > CE_BINK_MAX_VIDEO_HEIGHT) {
		return false;
	}*/
