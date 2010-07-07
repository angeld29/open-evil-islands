/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
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
 *  Waveform Audio File Format (C) Microsoft & IBM
 *
 *  See also:
 *  1. http://wiki.multimedia.cx/index.php?title=IMA_ADPCM
 *  2. http://wiki.multimedia.cx/index.php?title=Microsoft_IMA_ADPCM
*/

#ifndef CE_WAVE_H
#define CE_WAVE_H

#include <stdint.h>

#include "cememfile.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	CE_WAVE_FORMAT_PCM = 0x1,
	CE_WAVE_FORMAT_IMA_ADPCM = 0x11,
};

typedef struct {
	uint8_t four_cc[4];
	uint32_t size;
} ce_wave_riff;

typedef struct {
	uint8_t four_cc[4];
} ce_wave_wave;

typedef struct {
	uint16_t size;
	uint16_t samples_per_block;
} ce_wave_ima_adpcm;

typedef struct {
	uint8_t four_cc[4];
	uint32_t size;
	uint16_t tag;
	uint16_t channel_count;
	uint32_t samples_per_sec;
	uint32_t bytes_per_sec;
	uint16_t block_align;
	uint16_t bits_per_sample;
	union {
		ce_wave_ima_adpcm ima_adpcm;
	} extra;
} ce_wave_format;

typedef struct {
	uint8_t four_cc[4];
	uint32_t size;
	uint32_t uncompressed_size;
} ce_wave_fact;

typedef struct {
	uint8_t four_cc[4];
	uint32_t size;
} ce_wave_data;

typedef struct {
	ce_wave_riff riff;
	ce_wave_wave wave;
	ce_wave_format format;
	ce_wave_fact fact;
	ce_wave_data data;
} ce_wave_header;

extern bool ce_wave_header_read(ce_wave_header* wave_header, ce_mem_file* mem_file);

#ifdef __cplusplus
}
#endif

#endif /* CE_WAVE_H */
