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

#include <cassert>
#include <cstring>
#include <limits>
#include <memory>

#include "math.hpp"
#include "logging.hpp"
#include "wave.hpp"

namespace cursedearth
{
    const char* ce_wave_four_cc_riff = "RIFF";
    const char* ce_wave_four_cc_wave = "WAVE";
    const char* ce_wave_four_cc_format = "fmt ";
    const char* ce_wave_four_cc_fact = "fact";
    const char* ce_wave_four_cc_data = "data";

    bool ce_wave_header_read_riff(ce_wave_header* wave_header, ce_mem_file* mem_file)
    {
        memcpy(wave_header->riff.four_cc, ce_wave_four_cc_riff, 4);
        wave_header->riff.size = ce_mem_file_read_u32le(mem_file);
        return true;
    }

    bool ce_wave_header_read_wave(ce_wave_header* wave_header, ce_mem_file*)
    {
        memcpy(wave_header->wave.four_cc, ce_wave_four_cc_wave, 4);
        return true;
    }

    bool ce_wave_header_read_format_ima_adpcm(ce_wave_header* wave_header, ce_mem_file* mem_file)
    {
        wave_header->format.extra.ima_adpcm.size = ce_mem_file_read_u16le(mem_file);
        wave_header->format.extra.ima_adpcm.samples_per_block = ce_mem_file_read_u16le(mem_file);
        assert(2 * (wave_header->format.block_align - 4 * wave_header->format.channel_count) /
            wave_header->format.channel_count + 1 == wave_header->format.extra.ima_adpcm.samples_per_block);
        return true;
    }

    bool ce_wave_header_read_format(ce_wave_header* wave_header, ce_mem_file* mem_file)
    {
        memcpy(wave_header->format.four_cc, ce_wave_four_cc_format, 4);
        wave_header->format.size = ce_mem_file_read_u32le(mem_file);
        wave_header->format.tag = ce_mem_file_read_u16le(mem_file);
        wave_header->format.channel_count = ce_mem_file_read_u16le(mem_file);
        wave_header->format.samples_per_sec = ce_mem_file_read_u32le(mem_file);
        wave_header->format.bytes_per_sec = ce_mem_file_read_u32le(mem_file);
        wave_header->format.block_align = ce_mem_file_read_u16le(mem_file);
        wave_header->format.bits_per_sample = ce_mem_file_read_u16le(mem_file);

        bool result = false;
        switch (wave_header->format.tag) {
        case CE_WAVE_FORMAT_PCM:
            result = true;
            break;
        case CE_WAVE_FORMAT_IMA_ADPCM:
            result = ce_wave_header_read_format_ima_adpcm(wave_header, mem_file);
            break;
        }

        return result;
    }

    bool ce_wave_header_read_fact(ce_wave_header* wave_header, ce_mem_file* mem_file)
    {
        memcpy(wave_header->fact.four_cc, ce_wave_four_cc_fact, 4);
        wave_header->fact.size = ce_mem_file_read_u32le(mem_file);
        wave_header->fact.uncompressed_size = ce_mem_file_read_u32le(mem_file);
        return true;
    }

    bool ce_wave_header_read_data(ce_wave_header* wave_header, ce_mem_file* mem_file)
    {
        memcpy(wave_header->data.four_cc, ce_wave_four_cc_data, 4);
        wave_header->data.size = ce_mem_file_read_u32le(mem_file);
        return true;
    }

    bool ce_wave_header_check(const ce_wave_header* wave_header)
    {
        if (0 != memcmp(wave_header->riff.four_cc, ce_wave_four_cc_riff, 4) ||
                0 != memcmp(wave_header->wave.four_cc, ce_wave_four_cc_wave, 4) ||
                0 != memcmp(wave_header->format.four_cc, ce_wave_four_cc_format, 4) ||
                0 != memcmp(wave_header->data.four_cc, ce_wave_four_cc_data, 4)) {
            return false;
        }

        if (CE_WAVE_FORMAT_PCM != wave_header->format.tag &&
                0 != memcmp(wave_header->fact.four_cc, ce_wave_four_cc_fact, 4)) {
            return false;
        }

        return true;
    }

    bool ce_wave_header_read(ce_wave_header* wave_header, ce_mem_file* mem_file)
    {
        memset(wave_header, 0, sizeof(ce_wave_header));

        bool done = false;
        while (!done) {
            bool result = false;
            char four_cc[4];

            memset(four_cc, 0, 4);
            ce_mem_file_read(mem_file, four_cc, 1, 4);

            if (0 == memcmp(four_cc, ce_wave_four_cc_riff, 4)) {
                result = ce_wave_header_read_riff(wave_header, mem_file);
            } else if (0 == memcmp(four_cc, ce_wave_four_cc_wave, 4)) {
                result = ce_wave_header_read_wave(wave_header, mem_file);
            } else if (0 == memcmp(four_cc, ce_wave_four_cc_format, 4)) {
                result = ce_wave_header_read_format(wave_header, mem_file);
            } else if (0 == memcmp(four_cc, ce_wave_four_cc_fact, 4)) {
                result = ce_wave_header_read_fact(wave_header, mem_file);
            } else if (0 == memcmp(four_cc, ce_wave_four_cc_data, 4)) {
                result = ce_wave_header_read_data(wave_header, mem_file);
                done = true;
            }

            if (!result) {
                return false;
            }
        }

        return ce_wave_header_check(wave_header);
    }

    int ce_wave_ima_adpcm_index_table[16] = {
        -1, -1, -1, -1, /* +0 - +3, decrease the step size */
         2,  4,  6,  8, /* +4 - +7, increase the step size */
        -1, -1, -1, -1,	/* -0 - -3, decrease the step size */
         2,  4,  6,  8,	/* -4 - -7, increase the step size */
    };

    unsigned int ce_wave_ima_adpcm_step_table[89] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
        50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230,
        253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963,
        1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
        3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487,
        12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
    };

    inline int ce_wave_ima_adpcm_clamp_step_index(int index)
    {
        return clamp(index, 0, 88);
    }

    void ce_wave_ima_adpcm_decode(void* dst, const void* src, const ce_wave_header* wave_header)
    {
        int16_t* samples = static_cast<int16_t*>(dst);
        const uint8_t* block = static_cast<const uint8_t*>(src);

        std::unique_ptr<int[]> step_indices(new int[wave_header->format.channel_count]);

        // read and check the block header
        for (size_t channel = 0; channel < wave_header->format.channel_count; ++channel) {
            int32_t current = block[channel * 4 + 0] | (block[channel * 4 + 1] << 8);
            if (current & 0x8000) {
                current -= 0x10000;
            }

            step_indices[channel] = ce_wave_ima_adpcm_clamp_step_index(block[channel * 4 + 2]);

            if (0 != block[channel * 4 + 3]) {
                ce_logging_error("wave: ima adpcm synchronization error");
            }

            samples[channel] = current;
        }

        // pull apart the packed 4 bit samples and store them in their
        // correct sample positions
        size_t block_index = 4 * wave_header->format.channel_count;
        size_t index_start = wave_header->format.channel_count;

        while (block_index < wave_header->format.block_align) {
            for (size_t channel = 0; channel < wave_header->format.channel_count; ++channel) {
                size_t index = index_start + channel;
                for (size_t k = 0; k < 4; ++k) {
                    uint8_t byte_code = block[block_index++];
                    samples[index] = byte_code & 0xf;
                    index += wave_header->format.channel_count;
                    samples[index] = (byte_code >> 4) & 0xf;
                    index += wave_header->format.channel_count;
                }
            }
            index_start += 8 * wave_header->format.channel_count;
        }

        // decode the encoded 4 bit samples
        for (size_t k = wave_header->format.channel_count; k < wave_header->format.extra.ima_adpcm.samples_per_block * (size_t)wave_header->format.channel_count; ++k) {
            size_t channel = (wave_header->format.channel_count > 1) ? (k % 2) : 0;

            uint8_t byte_code = samples[k] & 0xf;
            unsigned int step = ce_wave_ima_adpcm_step_table[step_indices[channel]];
            int diff = step >> 3;

            if (byte_code & 1) diff += step >> 2;
            if (byte_code & 2) diff += step >> 1;
            if (byte_code & 4) diff += step;
            if (byte_code & 8) diff = -diff;

            int32_t current = samples[k - wave_header->format.channel_count];
            current += diff;
            samples[k] = clamp(current, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());

            step_indices[channel] += ce_wave_ima_adpcm_index_table[byte_code];
            step_indices[channel] = ce_wave_ima_adpcm_clamp_step_index(step_indices[channel]);
        }
    }
}
