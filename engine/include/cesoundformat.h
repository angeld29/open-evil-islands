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

#ifndef CE_SOUNDFORMAT_H
#define CE_SOUNDFORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned int bits_per_sample;
	unsigned int samples_per_sec;
	unsigned int channel_count;
	unsigned int sample_size;
	unsigned int bytes_per_sec;
} ce_sound_format;

static inline void ce_sound_format_init(ce_sound_format* sound_format,
										unsigned int bits_per_sample,
										unsigned int samples_per_sec,
										unsigned int channel_count)
{
	sound_format->bits_per_sample = bits_per_sample;
	sound_format->samples_per_sec = samples_per_sec;
	sound_format->channel_count = channel_count;
	sound_format->sample_size = channel_count * (bits_per_sample / 8);
	sound_format->bytes_per_sec = samples_per_sec * sound_format->sample_size;
}

#ifdef __cplusplus
}
#endif

#endif /* CE_SOUNDFORMAT_H */
