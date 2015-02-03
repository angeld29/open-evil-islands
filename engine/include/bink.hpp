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

/**
 * Bink Container (C) RAD Game Tools, Inc.
 * http://wiki.multimedia.cx/index.php?title=Bink_Container
 */

#ifndef CE_BINK_HPP
#define CE_BINK_HPP

#include <cstddef>
#include <cstdint>

#include "memfile.hpp"
#include "vector.hpp"

namespace cursedearth
{
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
        CE_BINK_MAX_FRAMES = 1000000,
        CE_BINK_MAX_VIDEO_WIDTH = 32767,
        CE_BINK_MAX_VIDEO_HEIGHT = 32767,
        CE_BINK_MAX_AUDIO_TRACKS = 256,
    };

    /*
     *  Header
     */

    typedef struct {
        uint32_t four_cc;
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
    } ce_bink_header;

    extern bool ce_bink_header_read(ce_bink_header* bink_header, ce_mem_file* mem_file);

    /*
     *  Audio Track
     */

    typedef struct {
        uint16_t sample_rate;
        uint16_t flags;
    } ce_bink_audio_track;

    extern bool ce_bink_audio_track_read(ce_bink_audio_track* bink_audio_track, ce_mem_file* mem_file);
    extern bool ce_bink_audio_track_skip(size_t n, ce_mem_file* mem_file);

    /*
     *  Frame Index Table
     */

    typedef struct {
        uint32_t pos;
        uint32_t length;
    } ce_bink_index;

    extern bool ce_bink_index_read(ce_bink_index* bink_indices, size_t n, ce_mem_file* mem_file);

    /*
     *  Frame Layout (only for illustration)
     */

    typedef struct {
        struct {
            // a value of zero indicates no audio is present for this track
            uint32_t packet_length; // in bytes plus four bytes
            uint32_t sample_count; // number of samples in packet (in bytes)
            // audio packet here (variable length)
        } audio_data[CE_BINK_MAX_AUDIO_TRACKS];
        // video packet here (variable length)
    } ce_bink_frame;
}

#endif
