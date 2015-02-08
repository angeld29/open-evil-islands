/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#include <cstdio>
#include <vector>

#include "alloc.hpp"
#include "logging.hpp"
#include "byteorder.hpp"
#include "bitmanipulations.hpp"
#include "bink.hpp"

namespace cursedearth
{
    enum {
        CE_BINK_HEADER_SIZE = 44,
        CE_BINK_AUDIO_HEADER_SIZE = 4 + 4 + 4,
    };

    bool ce_bink_header_read(ce_bink_header* bink_header, ce_mem_file* mem_file)
    {
        uint8_t header[CE_BINK_HEADER_SIZE];

        if (sizeof(header) != ce_mem_file_read(mem_file, header, 1, sizeof(header))) {
            return false;
        }

        union {
            uint8_t* u8;
            uint32_t* u32;
        } ptr = {header};

        if ('B' != ptr.u8[0] || 'I' != ptr.u8[1] || 'K' != ptr.u8[2]) {
            return false;
        }

        bink_header->four_cc = le2cpu(*ptr.u32++);
        bink_header->file_size = le2cpu(*ptr.u32++);
        bink_header->frame_count = le2cpu(*ptr.u32++);
        bink_header->largest_frame_size = le2cpu(*ptr.u32++);
        bink_header->last_frame = le2cpu(*ptr.u32++);
        bink_header->video_width = le2cpu(*ptr.u32++);
        bink_header->video_height = le2cpu(*ptr.u32++);
        bink_header->fps_dividend = le2cpu(*ptr.u32++);
        bink_header->fps_divider = le2cpu(*ptr.u32++);
        bink_header->video_flags = le2cpu(*ptr.u32++);
        bink_header->audio_track_count = le2cpu(*ptr.u32++);

        return true;
    }

    bool ce_bink_audio_track_read(ce_bink_audio_track* bink_audio_track, ce_mem_file* mem_file)
    {
        uint8_t header[CE_BINK_AUDIO_HEADER_SIZE];

        if (sizeof(header) != ce_mem_file_read(mem_file, header, 1, sizeof(header))) {
            return false;
        }

        union {
            uint8_t* u8;
            uint16_t* u16;
        } ptr = { header };

        ptr.u8 += 4; // not authoritative, skip

        bink_audio_track->sample_rate = le2cpu(*ptr.u16++);
        bink_audio_track->flags = le2cpu(*ptr.u16++);

        ptr.u8 += 4; // not used, skip

        return true;
    }

    bool ce_bink_audio_track_skip(size_t n, ce_mem_file* mem_file)
    {
        std::vector<uint8_t> header(CE_BINK_AUDIO_HEADER_SIZE * n);
        return header.size() == ce_mem_file_read(mem_file, header.data(), 1, header.size());
    }

    bool ce_bink_index_read(ce_bink_index* bink_indices, size_t n, ce_mem_file* mem_file)
    {
        uint32_t pos, next_pos;

        ce_mem_file_read(mem_file, &next_pos, 4, 1);

        for (size_t i = 0; i < n; ++i) {
            pos = next_pos;
            ce_mem_file_read(mem_file, &next_pos, 4, 1);

            // bit 0 indicates that frame is a keyframe; I'm not using it
            bink_indices[i].pos = clear_bit(pos, 0);
            bink_indices[i].length = clear_bit(next_pos, 0);

            if (bink_indices[i].length <= bink_indices[i].pos) {
                return false;
            }

            bink_indices[i].length -= bink_indices[i].pos;
        }

        return true;
    }
}
