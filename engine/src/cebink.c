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
#include "cebyteorder.h"
#include "cebitop.h"
#include "cebink.h"

enum {
	CE_BINK_HEADER_SIZE = 44,
	CE_BINK_AUDIO_HEADER_SIZE = 4 + 4 + 4,
};

bool ce_binkheader_read(ce_binkheader* binkheader, ce_memfile* memfile)
{
	union {
		uint8_t* u8;
		uint16_t* u16;
		uint32_t* u32;
	} ptr;

	uint8_t header[CE_BINK_HEADER_SIZE];
	ptr.u8 = header;

	if (sizeof(header) != ce_memfile_read(memfile, header, 1, sizeof(header))) {
		return false;
	}

	if ('B' != *ptr.u8++ || 'I' != *ptr.u8++ || 'K' != *ptr.u8++) {
		return false;
	}

	binkheader->revision = *ptr.u8++;

	if (CE_BINK_REVISION_B != binkheader->revision &&
			CE_BINK_REVISION_D != binkheader->revision &&
			CE_BINK_REVISION_F != binkheader->revision &&
			CE_BINK_REVISION_G != binkheader->revision &&
			CE_BINK_REVISION_H != binkheader->revision &&
			CE_BINK_REVISION_I != binkheader->revision) {
		return false;
	}

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

	if (0 == binkheader->frame_count ||
			0 == binkheader->video_width || 0 == binkheader->video_height ||
			0 == binkheader->fps_dividend || 0 == binkheader->fps_divider ||
			binkheader->largest_frame_size > binkheader->file_size ||
			binkheader->frame_count > CE_BINK_MAX_FRAMES ||
			binkheader->video_width > CE_BINK_MAX_VIDEO_WIDTH ||
			binkheader->video_height > CE_BINK_MAX_VIDEO_HEIGHT ||
			binkheader->audio_track_count > CE_BINK_MAX_AUDIO_TRACKS) {
		return false;
	}

	uint8_t audio_header[CE_BINK_AUDIO_HEADER_SIZE * binkheader->audio_track_count];
	ptr.u8 = audio_header;

	if (sizeof(audio_header) != ce_memfile_read(memfile, audio_header, 1, sizeof(audio_header))) {
		return false;
	}

	ptr.u8 += 4 * binkheader->audio_track_count; // not authoritative, skip

	for (size_t i = 0; i < binkheader->audio_track_count; ++i) {
		binkheader->audio_tracks[i].sample_rate = ce_le2cpu16(*ptr.u16++);
		binkheader->audio_tracks[i].flags = ce_le2cpu16(*ptr.u16++);
	}

	ptr.u8 += 4 * binkheader->audio_track_count; // not used, skip

	return true;
}

ce_binkindex* ce_binkindex_new(void)
{
	return ce_alloc(sizeof(ce_binkindex));
}

void ce_binkindex_del(ce_binkindex* binkindex)
{
	ce_free(binkindex, sizeof(ce_binkindex));
}

ce_vector* ce_bink_read_indices(ce_binkheader* binkheader, ce_memfile* memfile)
{
	ce_vector* binkindices = ce_vector_new_reserved(binkheader->frame_count);
	uint32_t pos, next_pos;

	ce_memfile_read(memfile, &next_pos, 4, 1);

	for (uint32_t i = 0; i < binkheader->frame_count; ++i) {
		pos = next_pos;
		ce_memfile_read(memfile, &next_pos, 4, 1);

		assert(ce_bitclr32(next_pos, 0) >
				ce_bitclr32(pos, 0) && "invalid frame index table");

		ce_binkindex* binkindex = ce_binkindex_new();
		binkindex->keyframe = ce_bittst32(pos, 0);
		binkindex->pos = ce_bitclr32(pos, 0);
		binkindex->length = ce_bitclr32(next_pos, 0) - binkindex->pos;

		ce_vector_push_back(binkindices, binkindex);
	}

	return binkindices;
}
