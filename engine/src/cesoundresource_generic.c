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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

#ifdef CE_ENABLE_PROPRIETARY
#include <mad.h>
#endif

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cebyteorder.h"
#include "ceerror.h"
#include "cesoundresource.h"

/*
 *  For more information about vorbis,
 *  see http://www.xiph.org/vorbis/doc/ and
 *  http://www.xiph.org/vorbis/doc/vorbisfile/index.html
*/

typedef struct {
	OggVorbis_File vf;
	int bitstream;
	long sample_size;
} ce_soundresource_vorbis;

static size_t ce_vorbis_read(void* ptr, size_t size, size_t nmemb, void* datasource)
{
	return ce_memfile_read(datasource, ptr, size, nmemb);
}

static int ce_vorbis_seek(void* datasource, ogg_int64_t offset, int whence)
{
	return ce_memfile_seek(datasource, offset, whence);
}

static long ce_vorbis_tell(void* datasource)
{
	return ce_memfile_tell(datasource);
}

static bool ce_soundresource_vorbis_test(ce_memfile* memfile)
{
	OggVorbis_File vf;
	if (0 == ov_test_callbacks(memfile, &vf, NULL, 0, (ov_callbacks)
			{ce_vorbis_read, ce_vorbis_seek, NULL, ce_vorbis_tell})) {
		ov_clear(&vf);
		return true;
	}
	return false;
}

static bool ce_soundresource_vorbis_ctor(ce_soundresource* soundresource)
{
	ce_soundresource_vorbis* vorbisresource = (ce_soundresource_vorbis*)soundresource->impl;

	if (0 != ov_open_callbacks(soundresource->memfile, &vorbisresource->vf, NULL, 0,
			(ov_callbacks){ce_vorbis_read, ce_vorbis_seek, NULL, ce_vorbis_tell})) {
		ce_logging_error("soundresource: vorbis: input does not appear to be an ogg vorbis audio");
		return false;
	}

	vorbis_info* info = ov_info(&vorbisresource->vf, -1);
	if (NULL == info) {
		ce_logging_error("soundresource: vorbis: could not get stream info");
		return false;
	}

	// a vorbis file has no particular number of bits per sample,
	// so use words, see also ov_read
	soundresource->bps = 16;
	soundresource->rate = info->rate;
	soundresource->channels = info->channels;

	vorbisresource->sample_size = soundresource->channels *
									(soundresource->bps / 8);

	ce_logging_debug("soundresource: vorbis: input is "
					"%ld bit/s (%ld bit/s nominal), %u Hz, %u channel",
		ov_bitrate(&vorbisresource->vf, -1), info->bitrate_nominal,
		soundresource->rate, soundresource->channels);

	return true;
}

static void ce_soundresource_vorbis_dtor(ce_soundresource* soundresource)
{
	ce_soundresource_vorbis* vorbisresource = (ce_soundresource_vorbis*)soundresource->impl;
	ov_clear(&vorbisresource->vf);
}

static size_t ce_soundresource_vorbis_read(ce_soundresource* soundresource, void* data, size_t size)
{
	ce_soundresource_vorbis* vorbisresource = (ce_soundresource_vorbis*)soundresource->impl;

	for (;;) {
		long code = ov_read(&vorbisresource->vf, data, size,
					ce_is_big_endian(), 2, 1, &vorbisresource->bitstream);
		if (code >= 0) {
			if (vorbisresource->vf.vd.granulepos >= 0) {
				soundresource->time =
					vorbis_granule_time(&vorbisresource->vf.vd,
										vorbisresource->vf.vd.granulepos);
			} else {
				soundresource->time +=
					vorbis_granule_time(&vorbisresource->vf.vd,
										code / vorbisresource->sample_size);
			}
			return code;
		}
		ce_logging_warning("soundresource: vorbis: error in the stream");
	}
}

static bool ce_soundresource_vorbis_reset(ce_soundresource* soundresource)
{
	ce_soundresource_vorbis* vorbisresource = (ce_soundresource_vorbis*)soundresource->impl;
	return 0 == ov_raw_seek(&vorbisresource->vf, 1);
}

#ifdef CE_ENABLE_PROPRIETARY
/*
 *  For more information about mad, see libmad source code and
 *  madlld, a simple sample demonstrating how the low-level
 *  libmad API can be used, which is perfectly commented.
 *  libmad (C) 2000-2004 Underbit Technologies, Inc.
 *  madlld (C) 2001-2004 Bertrand Petit
*/

enum {
	CE_MAD_INPUT_BUFFER_CAPACITY = 4 * 8192,
	CE_MAD_INPUT_BUFFER_GUARD = MAD_BUFFER_GUARD,
	CE_MAD_OUTPUT_BUFFER_CAPACITY = 8192,
	CE_MAD_DATA_SIZE = CE_MAD_INPUT_BUFFER_CAPACITY +
						CE_MAD_INPUT_BUFFER_GUARD +
						CE_MAD_OUTPUT_BUFFER_CAPACITY
};

typedef struct {
	mad_fixed_t error[3];
	mad_fixed_t random;
} ce_soundresource_mad_dither;

typedef struct {
	mad_fixed_t peak_clipping;
	mad_fixed_t peak_sample;
} ce_soundresource_mad_stats;

typedef struct {
	struct mad_stream stream;
	struct mad_frame frame;
	struct mad_synth synth;
	mad_timer_t timer;
	ce_soundresource_mad_dither dither[2]; // for 2 channels
	ce_soundresource_mad_stats stats;
	size_t output_buffer_size;
	unsigned char* output_buffer;
	unsigned char input_buffer[CE_MAD_DATA_SIZE];
} ce_soundresource_mad;

static bool ce_soundresource_mad_test(ce_memfile* memfile)
{
	unsigned char* buffer = ce_alloc(CE_MAD_INPUT_BUFFER_CAPACITY);
	size_t size = ce_memfile_read(memfile, buffer, 1, CE_MAD_INPUT_BUFFER_CAPACITY);

	struct mad_stream stream;
	struct mad_header header;

	mad_stream_init(&stream);
	mad_header_init(&header);

	mad_stream_buffer(&stream, buffer, size);

	while (-1 == mad_header_decode(&header, &stream)) {
		if (!MAD_RECOVERABLE(stream.error)) {
			break;
		}
	}

	// libmad have no good test functions, so use these weak conditions
	bool ok = MAD_ERROR_NONE == stream.error || MAD_RECOVERABLE(stream.error);

	mad_header_finish(&header);
	mad_stream_finish(&stream);

	ce_free(buffer, CE_MAD_INPUT_BUFFER_CAPACITY);

	return ok;
}

static void ce_soundresource_mad_error(ce_soundresource_mad* madresource,
													ce_logging_level level)
{
	ce_logging_report(level, "soundresource: mad: decoding error 0x%04x (%s)",
		madresource->stream.error, mad_stream_errorstr(&madresource->stream));
}

static bool ce_soundresource_mad_input(ce_soundresource* soundresource)
{
	ce_soundresource_mad* madresource = (ce_soundresource_mad*)soundresource->impl;

	// libmad may not consume all bytes of the input buffer
	size_t remaining = 0;

	if (NULL != madresource->stream.next_frame) {
		remaining = madresource->stream.bufend -
					madresource->stream.next_frame;
		memmove(madresource->input_buffer,
			madresource->stream.next_frame, remaining);
	}

	size_t size = ce_memfile_read(soundresource->memfile,
		madresource->input_buffer + remaining, 1,
		CE_MAD_INPUT_BUFFER_CAPACITY - remaining);

	if (ce_memfile_error(soundresource->memfile)) {
		ce_error_report_c_last("soundresource: mad");
		return false;
	}

	if (0 == size) {
		// EOF and no data available
		return false;
	}

	// when decoding the last frame of a file, it must be followed by
	// MAD_BUFFER_GUARD zero bytes if one wants to decode that last frame
	if (ce_memfile_eof(soundresource->memfile)) {
		memset(madresource->input_buffer + remaining + size, 0, CE_MAD_INPUT_BUFFER_GUARD);
		size += CE_MAD_INPUT_BUFFER_GUARD;
	}

	mad_stream_buffer(&madresource->stream,
						madresource->input_buffer,
							remaining + size);

	return true;
}

// 32-bit pseudo-random number generator
static inline mad_fixed_t ce_soundresource_mad_prng(mad_fixed_t state)
{
	return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

// generic linear sample quantize and dither routine
static int16_t ce_soundresource_mad_scale(mad_fixed_t sample,
	ce_soundresource_mad_dither* dither, ce_soundresource_mad_stats* stats)
{
	// noise shape
	sample += dither->error[0] - dither->error[1] + dither->error[2];

	dither->error[2] = dither->error[1];
	dither->error[1] = dither->error[0] / 2;

	// bias
	mad_fixed_t output = sample + (1L << (MAD_F_FRACBITS - 16));

	const unsigned int scalebits = MAD_F_FRACBITS + 1 - 16;
	const mad_fixed_t mask = (1L << scalebits) - 1;

	// dither
	const mad_fixed_t random = ce_soundresource_mad_prng(dither->random);
	output += (random & mask) - (dither->random & mask);

	dither->random = random;

	enum {
		CLIP_MIN = -MAD_F_ONE,
		CLIP_MAX =  MAD_F_ONE - 1
	};

	// clip
	if (output >= stats->peak_sample) {
		if (output > CLIP_MAX) {
			if (output - CLIP_MAX > stats->peak_clipping) {
				stats->peak_clipping = output - CLIP_MAX;
			}
			output = CLIP_MAX;
			if (sample > CLIP_MAX) {
				sample = CLIP_MAX;
			}
		}
		stats->peak_sample = output;
	} else if (output < -stats->peak_sample) {
		if (output < CLIP_MIN) {
			if (CLIP_MIN - output > stats->peak_clipping) {
				stats->peak_clipping = CLIP_MIN - output;
			}
			output = CLIP_MIN;
			if (sample < CLIP_MIN) {
				sample = CLIP_MIN;
			}
		}
		stats->peak_sample = -output;
	}

	// quantize
	output &= ~mask;

	// error feedback
	dither->error[0] = sample - output;

	// scale
	return output >> scalebits;
}

static bool ce_soundresource_mad_decode(ce_soundresource* soundresource)
{
	ce_soundresource_mad* madresource = (ce_soundresource_mad*)soundresource->impl;

	madresource->output_buffer = madresource->input_buffer +
		CE_MAD_INPUT_BUFFER_CAPACITY + CE_MAD_INPUT_BUFFER_GUARD;

	while (-1 == mad_frame_decode(&madresource->frame, &madresource->stream)) {
		if (MAD_ERROR_BUFLEN == madresource->stream.error ||
				MAD_ERROR_BUFPTR == madresource->stream.error) {
			// the input bucket must be filled if it becomes empty
			// or if it's the first execution of the function
			if (!ce_soundresource_mad_input(soundresource)) {
				return false;
			}
			// new input data loaded successfully, reset error code
			madresource->stream.error = MAD_ERROR_NONE;
		} else {
			if (!MAD_RECOVERABLE(madresource->stream.error)) {
				ce_soundresource_mad_error(madresource, CE_LOGGING_LEVEL_ERROR);
				return false;
			}
			ce_soundresource_mad_error(madresource, CE_LOGGING_LEVEL_DEBUG);
		}
	}

	mad_timer_add(&madresource->timer, madresource->frame.header.duration);
	soundresource->time = 1e-3f * mad_timer_count(madresource->timer,
													MAD_UNITS_MILLISECONDS);

	// once decoded the frame is synthesized to PCM samples
	mad_synth_frame(&madresource->synth, &madresource->frame);

	// synthesized samples must be converted from libmad's fixed
	// point number to the consumer format
	unsigned int sample_count  = madresource->synth.pcm.length;
	unsigned int channel_count = madresource->synth.pcm.channels;

	madresource->output_buffer_size = 2 * sample_count * channel_count;
	assert(madresource->output_buffer_size <= CE_MAD_OUTPUT_BUFFER_CAPACITY);

	// convert to signed 16 bit host endian integers
	int16_t* output_buffer = (int16_t*)madresource->output_buffer;

	for (unsigned int i = 0; i < sample_count; ++i) {
		for (unsigned int j = 0; j < channel_count; ++j) {
			output_buffer[channel_count * i + j] =
				ce_soundresource_mad_scale(madresource->synth.pcm.samples[j][i],
					&madresource->dither[j], &madresource->stats);
			ce_le2cpu16s((uint16_t*)&output_buffer[channel_count * i + j]);
		}
	}

	return true;
}

static void ce_soundresource_mad_init(ce_soundresource_mad* madresource)
{
	mad_stream_init(&madresource->stream);
	mad_frame_init(&madresource->frame);
	mad_synth_init(&madresource->synth);
	mad_timer_reset(&madresource->timer);

	memset(&madresource->dither, 0, sizeof(madresource->dither));
	memset(&madresource->stats, 0, sizeof(madresource->stats));

	madresource->output_buffer_size = 0;
}

static void ce_soundresource_mad_clean(ce_soundresource_mad* madresource)
{
	mad_synth_finish(&madresource->synth);
	mad_frame_finish(&madresource->frame);
	mad_stream_finish(&madresource->stream);
}

static bool ce_soundresource_mad_ctor(ce_soundresource* soundresource)
{
	ce_soundresource_mad* madresource = (ce_soundresource_mad*)soundresource->impl;
	ce_soundresource_mad_init(madresource);

	if (!ce_soundresource_mad_decode(soundresource)) {
		ce_logging_error("soundresource: mad: input does not appear to be a MPEG audio");
		return false;
	}

	soundresource->bps = 16;
	soundresource->rate = madresource->frame.header.samplerate;
	soundresource->channels = MAD_MODE_SINGLE_CHANNEL ==
								madresource->frame.header.mode ? 1 : 2;

	ce_logging_debug("soundresource: mad: input is %lu bit/s, %u Hz, %u channel",
		madresource->frame.header.bitrate, soundresource->rate, soundresource->channels);

	return true;
}

static void ce_soundresource_mad_dtor(ce_soundresource* soundresource)
{
	ce_soundresource_mad* madresource = (ce_soundresource_mad*)soundresource->impl;
	ce_soundresource_mad_clean(madresource);
}

static size_t ce_soundresource_mad_read(ce_soundresource* soundresource, void* data, size_t size)
{
	ce_soundresource_mad* madresource = (ce_soundresource_mad*)soundresource->impl;

	if (0 == madresource->output_buffer_size) {
		ce_soundresource_mad_decode(soundresource);
	}

	size = ce_smin(size, madresource->output_buffer_size);
	memcpy(data, madresource->output_buffer, size);

	madresource->output_buffer_size -= size;
	madresource->output_buffer += size;

	return size;
}

static bool ce_soundresource_mad_reset(ce_soundresource* soundresource)
{
	ce_soundresource_mad* madresource = (ce_soundresource_mad*)soundresource->impl;

	ce_soundresource_mad_clean(madresource);
	ce_soundresource_mad_init(madresource);

	return true;
}
#endif /* CE_ENABLE_PROPRIETARY */

const ce_soundresource_vtable ce_soundresource_builtins[] = {
	{sizeof(ce_soundresource_vorbis), ce_soundresource_vorbis_test,
	ce_soundresource_vorbis_ctor, ce_soundresource_vorbis_dtor,
	ce_soundresource_vorbis_read, ce_soundresource_vorbis_reset},
#ifdef CE_ENABLE_PROPRIETARY
	{sizeof(ce_soundresource_mad), ce_soundresource_mad_test,
	ce_soundresource_mad_ctor, ce_soundresource_mad_dtor,
	ce_soundresource_mad_read, ce_soundresource_mad_reset},
#endif
};

const size_t CE_SOUNDRESOURCE_BUILTIN_COUNT = sizeof(ce_soundresource_builtins) /
											sizeof(ce_soundresource_builtins[0]);
