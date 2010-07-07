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

#include <string.h>
#include <assert.h>

#include "cewave.h"

static const char* ce_wave_four_cc_riff = "RIFF";
static const char* ce_wave_four_cc_wave = "WAVE";
static const char* ce_wave_four_cc_format = "fmt ";
static const char* ce_wave_four_cc_fact = "fact";
static const char* ce_wave_four_cc_data = "data";

static bool ce_wave_header_read_riff(ce_wave_header* wave_header, ce_mem_file* mem_file)
{
	memcpy(wave_header->riff.four_cc, ce_wave_four_cc_riff, 4);
	wave_header->riff.size = ce_mem_file_read_u32le(mem_file);
	return true;
}

static bool ce_wave_header_read_wave(ce_wave_header* wave_header, ce_mem_file* CE_UNUSED(mem_file))
{
	memcpy(wave_header->wave.four_cc, ce_wave_four_cc_wave, 4);
	return true;
}

static bool ce_wave_header_read_format_ima_adpcm(ce_wave_header* wave_header, ce_mem_file* mem_file)
{
	wave_header->format.extra.ima_adpcm.size = ce_mem_file_read_u16le(mem_file);
	wave_header->format.extra.ima_adpcm.samples_per_block = ce_mem_file_read_u16le(mem_file);
	return true;
}

static bool ce_wave_header_read_format(ce_wave_header* wave_header, ce_mem_file* mem_file)
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

static bool ce_wave_header_read_fact(ce_wave_header* wave_header, ce_mem_file* mem_file)
{
	memcpy(wave_header->fact.four_cc, ce_wave_four_cc_fact, 4);
	wave_header->fact.size = ce_mem_file_read_u32le(mem_file);
	wave_header->fact.uncompressed_size = ce_mem_file_read_u32le(mem_file);
	return true;
}

static bool ce_wave_header_read_data(ce_wave_header* wave_header, ce_mem_file* mem_file)
{
	memcpy(wave_header->data.four_cc, ce_wave_four_cc_data, 4);
	wave_header->data.size = ce_mem_file_read_u32le(mem_file);
	return true;
}

static bool ce_wave_header_check(const ce_wave_header* wave_header)
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
