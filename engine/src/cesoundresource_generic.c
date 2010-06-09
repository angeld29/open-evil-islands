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
#include <math.h>
#include <assert.h>

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

#ifdef CE_ENABLE_PROPRIETARY
#include <mad.h>
#endif

#include <fftw3.h>

#include "celib.h"
#include "cemath.h"
#include "cealloc.h"
#include "celogging.h"
#include "cebyteorder.h"
#include "cebitarray.h"
#include "ceerror.h"
#include "cebink.h"
#include "cesoundresource.h"

/*
 *  Ogg Vorbis (C) 1994-2001 Xiph.Org Foundation
 *
 *  See also:
 *  1. http://www.xiph.org/ogg/doc/
 *  2. http://www.xiph.org/vorbis/doc/
*/

typedef struct {
	OggVorbis_File vf;
	int bitstream;
	long sample_size;
} ce_vorbis;

static size_t ce_vorbis_read_wrap(void* ptr, size_t size, size_t nmemb, void* datasource)
{
	return ce_memfile_read(datasource, ptr, size, nmemb);
}

static int ce_vorbis_seek_wrap(void* datasource, ogg_int64_t offset, int whence)
{
	return ce_memfile_seek(datasource, offset, whence);
}

static long ce_vorbis_tell_wrap(void* datasource)
{
	return ce_memfile_tell(datasource);
}

static bool ce_vorbis_test(ce_memfile* memfile)
{
	OggVorbis_File vf;
	if (0 == ov_test_callbacks(memfile, &vf, NULL, 0, (ov_callbacks)
			{ce_vorbis_read_wrap, ce_vorbis_seek_wrap, NULL, ce_vorbis_tell_wrap})) {
		ov_clear(&vf);
		return true;
	}
	return false;
}

static bool ce_vorbis_ctor(ce_soundresource* soundresource)
{
	ce_vorbis* vorbis = (ce_vorbis*)soundresource->impl;

	if (0 != ov_open_callbacks(soundresource->memfile, &vorbis->vf, NULL, 0,
			(ov_callbacks){ce_vorbis_read_wrap, ce_vorbis_seek_wrap, NULL, ce_vorbis_tell_wrap})) {
		ce_logging_error("vorbis: input does not appear to be an Ogg Vorbis audio");
		return false;
	}

	vorbis_info* info = ov_info(&vorbis->vf, -1);
	if (NULL == info) {
		ce_logging_error("vorbis: could not get stream info");
		return false;
	}

	// a vorbis file has no particular number of bits per sample,
	// so use words, see also ov_read
	soundresource->bits_per_sample = 16;
	soundresource->sample_rate = info->rate;
	soundresource->channel_count = info->channels;

	vorbis->sample_size = soundresource->channel_count *
							(soundresource->bits_per_sample / 8);

	ce_logging_debug("vorbis: input is %ld bit/s (%ld bit/s nominal), %u Hz, %u channel",
		ov_bitrate(&vorbis->vf, -1), info->bitrate_nominal,
		soundresource->sample_rate, soundresource->channel_count);

	return true;
}

static void ce_vorbis_dtor(ce_soundresource* soundresource)
{
	ce_vorbis* vorbis = (ce_vorbis*)soundresource->impl;
	ov_clear(&vorbis->vf);
}

static size_t ce_vorbis_read(ce_soundresource* soundresource, void* data, size_t size)
{
	ce_vorbis* vorbis = (ce_vorbis*)soundresource->impl;

	for (;;) {
		long code = ov_read(&vorbis->vf, data, size,
					ce_is_big_endian(), 2, 1, &vorbis->bitstream);
		if (code >= 0) {
			if (vorbis->vf.vd.granulepos >= 0) {
				soundresource->time =
					vorbis_granule_time(&vorbis->vf.vd,
										vorbis->vf.vd.granulepos);
			} else {
				soundresource->time +=
					vorbis_granule_time(&vorbis->vf.vd,
										code / vorbis->sample_size);
			}
			return code;
		}
		ce_logging_warning("vorbis: error in the stream");
	}
}

static bool ce_vorbis_reset(ce_soundresource* soundresource)
{
	ce_vorbis* vorbis = (ce_vorbis*)soundresource->impl;
	return 0 == ov_raw_seek(&vorbis->vf, 1);
}

#ifdef CE_ENABLE_PROPRIETARY
/*
 *  MAD: MPEG Audio Decoder (C) 2000-2004 Underbit Technologies, Inc.
 *
 *  See also:
 *  1. libmad source code
 *  2. madlld (C) 2001-2004 Bertrand Petit
 *     a simple sample demonstrating how the low-level libmad API can be used
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
} ce_mad_dither;

typedef struct {
	mad_fixed_t peak_clipping;
	mad_fixed_t peak_sample;
} ce_mad_stats;

typedef struct {
	struct mad_stream stream;
	struct mad_frame frame;
	struct mad_synth synth;
	mad_timer_t timer;
	ce_mad_dither dither[2]; // for 2 channels
	ce_mad_stats stats;
	size_t output_buffer_size;
	unsigned char* output_buffer;
	unsigned char input_buffer[CE_MAD_DATA_SIZE];
} ce_mad;

static void ce_mad_error(ce_soundresource* soundresource, ce_logging_level level)
{
	ce_mad* mad = (ce_mad*)soundresource->impl;
	ce_logging_report(level, "mad: decoding error 0x%04x (%s)",
		mad->stream.error, mad_stream_errorstr(&mad->stream));
}

static bool ce_mad_input(ce_soundresource* soundresource)
{
	ce_mad* mad = (ce_mad*)soundresource->impl;

	// libmad may not consume all bytes of the input buffer
	size_t remaining = 0;

	if (NULL != mad->stream.next_frame) {
		remaining = mad->stream.bufend - mad->stream.next_frame;
		memmove(mad->input_buffer, mad->stream.next_frame, remaining);
	}

	size_t size = ce_memfile_read(soundresource->memfile,
		mad->input_buffer + remaining, 1,
		CE_MAD_INPUT_BUFFER_CAPACITY - remaining);

	if (ce_memfile_error(soundresource->memfile)) {
		ce_error_report_c_last("mad");
		return false;
	}

	if (0 == size) {
		// EOF and no data available
		return false;
	}

	// when decoding the last frame of a file, it must be followed by
	// MAD_BUFFER_GUARD zero bytes if one wants to decode that last frame
	if (ce_memfile_eof(soundresource->memfile)) {
		memset(mad->input_buffer + remaining + size, 0, CE_MAD_INPUT_BUFFER_GUARD);
		size += CE_MAD_INPUT_BUFFER_GUARD;
	}

	mad_stream_buffer(&mad->stream,
						mad->input_buffer,
							remaining + size);

	return true;
}

// 32-bit pseudo-random number generator
static inline mad_fixed_t ce_mad_prng(mad_fixed_t state)
{
	return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

// generic linear sample quantize and dither routine
static int16_t ce_mad_scale(mad_fixed_t sample,
							ce_mad_dither* dither,
							ce_mad_stats* stats)
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
	const mad_fixed_t random = ce_mad_prng(dither->random);
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

static bool ce_mad_decode(ce_soundresource* soundresource)
{
	ce_mad* mad = (ce_mad*)soundresource->impl;

	mad->output_buffer = mad->input_buffer +
		CE_MAD_INPUT_BUFFER_CAPACITY + CE_MAD_INPUT_BUFFER_GUARD;

	while (-1 == mad_frame_decode(&mad->frame, &mad->stream)) {
		if (MAD_ERROR_BUFLEN == mad->stream.error ||
				MAD_ERROR_BUFPTR == mad->stream.error) {
			// the input bucket must be filled if it becomes empty
			// or if it's the first execution of the function
			if (!ce_mad_input(soundresource)) {
				return false;
			}
			// new input data loaded successfully, reset error code
			mad->stream.error = MAD_ERROR_NONE;
		} else {
			if (!MAD_RECOVERABLE(mad->stream.error)) {
				ce_mad_error(soundresource, CE_LOGGING_LEVEL_ERROR);
				return false;
			}
			ce_mad_error(soundresource, CE_LOGGING_LEVEL_DEBUG);
		}
	}

	mad_timer_add(&mad->timer, mad->frame.header.duration);
	soundresource->time = 1e-3f * mad_timer_count(mad->timer, MAD_UNITS_MILLISECONDS);

	// once decoded the frame is synthesized to PCM samples
	mad_synth_frame(&mad->synth, &mad->frame);

	// synthesized samples must be converted from libmad's fixed
	// point number to the consumer format
	unsigned int sample_count  = mad->synth.pcm.length;
	unsigned int channel_count = mad->synth.pcm.channels;

	mad->output_buffer_size = 2 * sample_count * channel_count;
	assert(mad->output_buffer_size <= CE_MAD_OUTPUT_BUFFER_CAPACITY);

	// convert to signed 16 bit host endian integers
	int16_t* output_buffer = (int16_t*)mad->output_buffer;

	for (unsigned int i = 0; i < sample_count; ++i) {
		for (unsigned int j = 0; j < channel_count; ++j) {
			output_buffer[channel_count * i + j] =
				ce_mad_scale(mad->synth.pcm.samples[j][i],
							&mad->dither[j], &mad->stats);
			ce_le2cpu16s((uint16_t*)&output_buffer[channel_count * i + j]);
		}
	}

	return true;
}

static void ce_mad_init(ce_mad* mad)
{
	mad_stream_init(&mad->stream);
	mad_frame_init(&mad->frame);
	mad_synth_init(&mad->synth);
	mad_timer_reset(&mad->timer);

	memset(&mad->dither, 0, sizeof(mad->dither));
	memset(&mad->stats, 0, sizeof(mad->stats));

	mad->output_buffer_size = 0;
}

static void ce_mad_clean(ce_mad* mad)
{
	mad_synth_finish(&mad->synth);
	mad_frame_finish(&mad->frame);
	mad_stream_finish(&mad->stream);
}

static bool ce_mad_test(ce_memfile* memfile)
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

static bool ce_mad_ctor(ce_soundresource* soundresource)
{
	ce_mad* mad = (ce_mad*)soundresource->impl;
	ce_mad_init(mad);

	if (!ce_mad_decode(soundresource)) {
		ce_logging_error("mad: input does not appear to be a MPEG audio");
		return false;
	}

	soundresource->bits_per_sample = 16;
	soundresource->sample_rate = mad->frame.header.samplerate;
	soundresource->channel_count = MAD_MODE_SINGLE_CHANNEL ==
									mad->frame.header.mode ? 1 : 2;

	ce_logging_debug("mad: input is %lu bit/s, %u Hz, %u channel",
		mad->frame.header.bitrate, soundresource->sample_rate, soundresource->channel_count);

	return true;
}

static void ce_mad_dtor(ce_soundresource* soundresource)
{
	ce_mad* mad = (ce_mad*)soundresource->impl;
	ce_mad_clean(mad);
}

static size_t ce_mad_read(ce_soundresource* soundresource, void* data, size_t size)
{
	ce_mad* mad = (ce_mad*)soundresource->impl;

	if (0 == mad->output_buffer_size) {
		ce_mad_decode(soundresource);
	}

	size = ce_smin(size, mad->output_buffer_size);
	memcpy(data, mad->output_buffer, size);

	mad->output_buffer_size -= size;
	mad->output_buffer += size;

	return size;
}

static bool ce_mad_reset(ce_soundresource* soundresource)
{
	ce_mad* mad = (ce_mad*)soundresource->impl;

	ce_mad_clean(mad);
	ce_mad_init(mad);

	return true;
}

/*
 *  Bink Audio (C) RAD Game Tools, Inc.
 *
 *  See also:
 *  1. http://wiki.multimedia.cx/index.php?title=Bink_Audio
 *  2. FFmpeg source code
*/

enum {
	CE_BINK_CHANNEL_COUNT = 2, // interleaved for the RDFT format variant
	CE_BINK_BLOCK_SIZE = CE_BINK_CHANNEL_COUNT << 11,
	CE_BINK_MAX_AUDIO_FRAME_SIZE = 192000, // 1 second of 48khz 32bit audio
};

typedef struct {
	ce_binkheader header;
	ce_binktrack track;
	unsigned int frame_size; // transform size (in samples)
	unsigned int window_size; // overlap size (in samples)
	unsigned int block_size;
	unsigned int band_count;
	unsigned int band_freqs[25 + 1];
	float coeffs[2 * CE_BINK_BLOCK_SIZE];
	int16_t previous[CE_BINK_BLOCK_SIZE / 16]; // coeffs from previous audio block
	size_t output_buffer_pos, output_buffer_size;
	uint8_t output_buffer[CE_BINK_MAX_AUDIO_FRAME_SIZE];
	ce_binkindex* indices;
	ce_bitarray* bitarray;
	fftwf_plan plan;
	size_t index;
	float root;
	bool first;
	float fps;
} ce_bink;

static const uint16_t ce_bink_critical_freqs[25] = {
    100,   200,  300,  400,  510,  630,  770,   920,
    1080,  1270, 1480, 1720, 2000, 2320, 2700,  3150,
    3700,  4400, 5300, 6400, 7700, 9500, 12000, 15500,
    24500,
};

static const uint8_t ce_bink_rle_lengths[16] = {
	2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64
};

static void ce_bink_error_obsolete(const char* message)
{
	ce_logging_error(message);
	ce_logging_warning("bink: this codec is no longer supported; use Ogg Vorbis instead");
}

static bool ce_bink_test(ce_memfile* memfile)
{
	ce_binkheader binkheader;
	return ce_binkheader_read(&binkheader, memfile) &&
			0 != binkheader.audio_track_count;
}

static bool ce_bink_ctor(ce_soundresource* soundresource)
{
	ce_bink* bink = (ce_bink*)soundresource->impl;

	if (!ce_binkheader_read(&bink->header, soundresource->memfile)) {
		ce_logging_error("bink: input does not appear to be a Bink audio");
		return false;
	}

	if (CE_BINK_REVISION_I != bink->header.revision) {
		ce_bink_error_obsolete("bink: unsupported version");
		return false;
	}

	if (1 != bink->header.audio_track_count) {
		ce_bink_error_obsolete("bink: only one audio track supported");
		return false;
	}

	if (!ce_binktrack_read(&bink->track, soundresource->memfile)) {
		ce_logging_error("bink: input does not appear to be a Bink audio");
		return false;
	}

	if (CE_BINK_AUDIO_FLAG_USE_DCT & bink->track.flags) {
		ce_bink_error_obsolete("bink: DCT audio algorithm not supported");
		return false;
	}

	soundresource->bits_per_sample = 16;
	soundresource->sample_rate = bink->track.sample_rate;
	soundresource->channel_count = CE_BINK_AUDIO_FLAG_STEREO &
									bink->track.flags ? 2 : 1;

	ce_logging_debug("bink: input is %u Hz, %u channel",
		soundresource->sample_rate, soundresource->channel_count);

	size_t frame_size_bits = 11;
	if (soundresource->sample_rate < 44100) frame_size_bits = 10;
	if (soundresource->sample_rate < 22050) frame_size_bits = 9;

	// audio is already interleaved for the RDFT format variant
	unsigned int sample_rate = soundresource->sample_rate *
								soundresource->channel_count;

	bink->frame_size = (1 << frame_size_bits) * soundresource->channel_count;
	bink->window_size = bink->frame_size / 16;
	bink->block_size = bink->frame_size - bink->window_size;
	bink->root = 2.0f / sqrtf(bink->frame_size);
	bink->first = true;
	bink->fps = bink->header.fps_dividend / (float)bink->header.fps_divider;

	ce_logging_debug("bink: largest_frame_size %u, fps %f",
		bink->header.largest_frame_size, bink->fps);

	ce_logging_debug("bink: frame_size %u, window_size %u, block_size %u, root %f",
		bink->frame_size, bink->window_size, bink->block_size, bink->root);

	if (2 == soundresource->channel_count) {
		++frame_size_bits;
	}

	ce_logging_debug("bink: %lu", frame_size_bits);

	// calculate number of bands
	unsigned int sample_rate_half = (sample_rate + 1) / 2;

	ce_logging_debug("bink: sample_rate_half %u", sample_rate_half);

	for (bink->band_count = 1; bink->band_count < 25; ++bink->band_count) {
		if (sample_rate_half <= ce_bink_critical_freqs[bink->band_count - 1]) {
			break;
		}
	}

	ce_logging_debug("bink: band_count %u", bink->band_count);

	// populate bands data
	bink->band_freqs[0] = 1;

	for (unsigned int i = 1; i < bink->band_count; ++i) {
		bink->band_freqs[i] = ce_bink_critical_freqs[i - 1] *
								(bink->frame_size / 2) / sample_rate_half;
		printf("%u ", bink->band_freqs[i]);
	}
	bink->band_freqs[bink->band_count] = bink->frame_size / 2;
	printf("%u\n", bink->band_freqs[bink->band_count]);

	ce_logging_debug("bink: frame_size %u, window_size %u, sample_rate_half %u, "
		"band_count %u",
		bink->frame_size, bink->window_size, sample_rate_half, bink->band_count);

	bink->indices = ce_alloc(sizeof(ce_binkindex) * bink->header.frame_count);

	if (!ce_bink_read_indices(bink->indices, bink->header.frame_count, soundresource->memfile)) {
		ce_logging_error("bink: invalid frame index table");
		return false;
	}

	bink->bitarray = ce_bitarray_new(bink->header.largest_frame_size);

	bink->plan = fftwf_plan_dft_c2r_1d(CE_BINK_BLOCK_SIZE,
		(fftwf_complex*)bink->coeffs, bink->coeffs,
		FFTW_MEASURE | FFTW_PRESERVE_INPUT | FFTW_PATIENT | FFTW_UNALIGNED);

	if (NULL == bink->plan) {
		ce_logging_error("bink: could not create a FFT plan");
		return false;
	}

	return true;
}

static void ce_bink_dtor(ce_soundresource* soundresource)
{
	ce_bink* bink = (ce_bink*)soundresource->impl;

	if (NULL != bink->plan) {
		fftwf_destroy_plan(bink->plan);
	}

	ce_free(bink->indices, sizeof(ce_binkindex) * bink->header.frame_count);
	ce_bitarray_del(bink->bitarray);
}

static float ce_bink_get_float(ce_bitarray* bitarray)
{
	/*for (int i = 0; i < 32; ++i) {
		uint8_t t = ce_bitarray_get_bit(bitarray);
		printf("%hhu", t);
	}
	printf("\n");*/
	uint32_t exp = ce_bitarray_get_bits(bitarray, 5);
	/*for (int i = 0; i < 5; ++i) {
		printf("%u", (exp >> i) & 0x1);
	}
	printf("\n");*/
	uint32_t x = ce_bitarray_get_bits(bitarray, 23);
	/*for (int i = 0; i < 23; ++i) {
		printf("%u", (x >> i) & 0x1);
	}
	printf("\n");*/
	//ce_logging_debug("bink: exp %hhu, x %u", exp, x);
	float value = ldexpf(x, exp - 23);
	if (ce_bitarray_get_bit(bitarray)) {
		value = -value;
	}
	return value;
}

static int float_to_int16_one(const float *src){
    int_fast32_t tmp = *(const int32_t*)src;
    if(tmp & 0xf0000){
        tmp = (0x43c0ffff - tmp)>>31;
    }
    return tmp - 0x8000;
}

static void float_to_int16_interleave(int16_t* dst, const float* src, size_t len)
{
	//printf("float_to_int16_interleave len %lu, array len %d\n", len, (int)CE_BINK_BLOCK_SIZE);
	for (size_t i = 0; i < len; ++i) {
		dst[i] = float_to_int16_one(src + i);
		//printf("(%f,%hd) ", *(src + i), dst[i]);
	}
	/*printf("\n");
	fflush(stdout);
	*(int*)NULL = 1;*/
}

static void ce_bink_decode_block(ce_soundresource* soundresource, int16_t* samples)
{
	ce_bink* bink = (ce_bink*)soundresource->impl;

	float f1 = ce_bink_get_float(bink->bitarray);
	float f2 = ce_bink_get_float(bink->bitarray);
	//ce_logging_debug("bink: f1 f2 %f %f", f1, f2);

	float q, quant[25];

	bink->coeffs[0] = f1 * bink->root;
	bink->coeffs[1] = f2 * bink->root;

	for (size_t i = 0; i < bink->band_count; ++i) {
		uint32_t value = ce_bitarray_get_bits(bink->bitarray, 8);
		// constant is result of 0.066399999/log10(M_E)
		quant[i] = expf(ce_min(value, 95) * 0.15289164787221953823f) * bink->root;
	}

	// find band
	size_t k;
	for (k = 0; bink->band_freqs[k] < 1; ++k) {
		q = quant[k];
	}

	int test = 0;
	//ce_logging_debug("bink: parsecoeffs %lu", ce_bitarray_count(bink->bitarray));

	// parse coefficients
	for (size_t i = 2, j; i < bink->frame_size; ) {
		if (ce_bitarray_get_bit(bink->bitarray)) {
			j = i + ce_bink_rle_lengths[ce_bitarray_get_bits(bink->bitarray, 4)] * 8;
		} else {
			j = i + 8;
		}

		j = ce_smin(j, bink->frame_size);

		size_t width = ce_bitarray_get_bits(bink->bitarray, 4);
		if (0 == width) {
			memset(bink->coeffs + i, 0, (j - i) * sizeof(float));
			i = j;
			while (bink->band_freqs[k] * 2 < i) {
				q = quant[k++];
			}
		} else {
			while (i < j) {
				if (bink->band_freqs[k] * 2 == i) {
					q = quant[k++];
				}
				int coeff = ce_bitarray_get_bits(bink->bitarray, width);
				if (coeff) {
					if (ce_bitarray_get_bit(bink->bitarray)) {
						bink->coeffs[i] = -q * coeff;
					} else {
						bink->coeffs[i] =  q * coeff;
					}
				} else {
					bink->coeffs[i] = 0.0f;
				}
				++i;
			}
		}

		++test;
	}

	/*for (size_t i = 0; i < 50; ++i) {
		printf("%f ", bink->coeffs[i]);
	}
	printf("\n");*/

	/*for (size_t i = 0; i < 50; ++i) {
		printf("%hhu ", ((uint8_t*)bink->coeffs)[i]);
	}
	printf("\n");*/

	//ce_logging_debug("bink: bit_count %lu, test %d",
	//	ce_bitarray_count(bink->bitarray), test);

	fftwf_execute(bink->plan);

	for (size_t i = 0; i < bink->frame_size; ++i) {
		bink->coeffs[i] = 385.0f + bink->coeffs[i] * (1.0f / 32767.0f);
	}

	/*for (size_t i = 0; i < bink->frame_size; ++i) {
		printf("%f ", bink->coeffs[i]);
	}
	printf("\n\n\n");*/

	float_to_int16_interleave(samples, bink->coeffs, bink->frame_size);

	if (!bink->first) {
		for (int i = 0; i < (int)bink->window_size; ++i) {
			int shift = log2(bink->window_size);
			samples[i] = (bink->previous[i] * ((int)bink->window_size - i) +
						samples[i] * (int)i) /*/ (int)bink->window_size*/ >> shift;
		}
		/*FILE* f = fopen("test1.bin", "ab");
		fwrite(samples, 2, bink->window_size, f);
		fflush(f);
		fclose(f);*/
	} else {
		/*for (size_t i = 0; i < bink->window_size; ++i) {
			printf("%hd ", *(samples + bink->block_size + i));
		}
		printf("\n");*/
	}

	memcpy(bink->previous, samples + bink->block_size,
			bink->window_size * sizeof(int16_t));

	bink->first = false;
}

static bool ce_bink_decode_frame(ce_soundresource* soundresource)
{
	ce_bink* bink = (ce_bink*)soundresource->impl;

	if (bink->index == bink->header.frame_count) {
		assert(ce_memfile_eof(soundresource->memfile));
		ce_logging_debug("bink: ret 0");
		return false;
	}

	int16_t* samples = (int16_t*)bink->output_buffer;
	int16_t* samples_end = samples + CE_BINK_MAX_AUDIO_FRAME_SIZE / sizeof(int16_t);

	uint32_t frame_size = bink->indices[bink->index++].length;
	uint32_t packet_size;

	ce_memfile_read(soundresource->memfile, &packet_size, 4, 1);

	ce_logging_debug("bink: frame size %u, packet size %u, block size %u, idx pos %u, tell %ld, block %lu",
		frame_size, packet_size, bink->block_size, bink->indices[bink->index - 1].pos,
		ce_memfile_tell(soundresource->memfile), packet_size % bink->block_size);

	if (0 != packet_size) {
		ce_bitarray_reset(bink->bitarray, packet_size);
		ce_memfile_read(soundresource->memfile, bink->bitarray->array, 1, packet_size);

		uint32_t sample_count = ce_bitarray_get_bits(bink->bitarray, 32);
		ce_logging_debug("bink: sample count %u", sample_count);
		ce_logging_debug("bink: packet size %u", packet_size);

		while (ce_bitarray_count(bink->bitarray) / 8 < packet_size &&
				samples + bink->block_size <= samples_end) {
			ce_bink_decode_block(soundresource, samples);
			samples += bink->block_size;
			// 32 bit align
			//ce_logging_debug("bink: 1 count %lu", ce_bitarray_count(bink->bitarray));
			size_t n = ce_bitarray_count(bink->bitarray) % 32;
			if (0 != n) {
				ce_bitarray_skip_bits(bink->bitarray, 32 - n);
			}
			//ce_logging_debug("bink: 2 count %lu", ce_bitarray_count(bink->bitarray));
		}

		ce_logging_debug("bink: %u %ld", sample_count, (uint8_t*)samples - bink->output_buffer);

		bink->output_buffer_pos = 0;
		bink->output_buffer_size = ce_smin(sample_count, (uint8_t*)samples - bink->output_buffer);

		/*FILE* f = fopen("test1.bin", "ab");
		fwrite(bink->output_buffer, 1, bink->output_buffer_size, f);
		fflush(f);
		fclose(f);*/

		//float test = 1.0f / bink->fps, test2 = soundresource->sample_rate / bink->fps;
	}

	frame_size -= packet_size + sizeof(packet_size);

	// skip video packet
	ce_memfile_seek(soundresource->memfile, frame_size, CE_MEMFILE_SEEK_CUR);

	if (ce_memfile_eof(soundresource->memfile)) {
		ce_logging_debug("bink: eof ret size");
	}

	return 0 == packet_size;
}

static size_t ce_bink_read(ce_soundresource* soundresource, void* data, size_t size)
{
	ce_bink* bink = (ce_bink*)soundresource->impl;

	if (0 == bink->output_buffer_size) {
		do { // nothing
		} while (ce_bink_decode_frame(soundresource));
	}

	size = ce_smin(size, bink->output_buffer_size);
	memcpy(data, bink->output_buffer + bink->output_buffer_pos, size);

	bink->output_buffer_pos += size;
	bink->output_buffer_size -= size;

	return size;
}

static bool ce_bink_reset(ce_soundresource* soundresource)
{
	ce_unused(soundresource);
	return false;
}
#endif /* CE_ENABLE_PROPRIETARY */

const ce_soundresource_vtable ce_soundresource_builtins[] = {
	{sizeof(ce_vorbis), ce_vorbis_test, ce_vorbis_ctor,
	ce_vorbis_dtor, ce_vorbis_read, ce_vorbis_reset},
#ifdef CE_ENABLE_PROPRIETARY
	{sizeof(ce_mad), ce_mad_test, ce_mad_ctor,
	ce_mad_dtor, ce_mad_read, ce_mad_reset},
	{sizeof(ce_bink), ce_bink_test, ce_bink_ctor,
	ce_bink_dtor, ce_bink_read, ce_bink_reset},
#endif
};

const size_t CE_SOUNDRESOURCE_BUILTIN_COUNT = sizeof(ce_soundresource_builtins) /
											sizeof(ce_soundresource_builtins[0]);
