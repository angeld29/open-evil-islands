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

#include <libavcodec/avcodec.h>

#include "celib.h"
#include "cemath.h"
#include "cealloc.h"
#include "celogging.h"
#include "cebyteorder.h"
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

static size_t ce_vorbis_size_hint(ce_memfile* memfile)
{
	ce_unused(memfile);
	return sizeof(ce_vorbis);
}

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

static size_t ce_mad_size_hint(ce_memfile* memfile)
{
	ce_unused(memfile);
	return sizeof(ce_mad);
}

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
 *  2. FFmpeg (C) Michael Niedermayer
*/

typedef struct {
	size_t frame_index;
	ce_binkheader header;
	ce_binktrack track;
	ce_binkindex* indices;
	AVCodec* codec;
	AVCodecContext* codec_context;
	AVPacket packet;
	size_t output_pos, output_size;
	uint8_t output[AVCODEC_MAX_AUDIO_FRAME_SIZE];
	uint8_t data[];
} ce_bink;

static size_t ce_bink_size_hint(ce_memfile* memfile)
{
	ce_binkheader header;
	return sizeof(ce_bink) + (ce_binkheader_read(&header, memfile) ?
		sizeof(ce_binkindex) * header.frame_count +
		header.largest_frame_size + FF_INPUT_BUFFER_PADDING_SIZE : 0);
}

static bool ce_bink_test(ce_memfile* memfile)
{
	ce_binkheader binkheader;
	return ce_binkheader_read(&binkheader, memfile) &&
			0 != binkheader.audio_track_count;
}

static void ce_bink_error(void* ptr, int av_level, const char* format, va_list args)
{
	ce_unused(ptr);
	ce_logging_level level;
	switch (av_level) {
	case AV_LOG_PANIC:
	case AV_LOG_FATAL:
		level = CE_LOGGING_LEVEL_FATAL;
		break;
	case AV_LOG_ERROR:
		level = CE_LOGGING_LEVEL_ERROR;
		break;
	case AV_LOG_WARNING:
		level = CE_LOGGING_LEVEL_WARNING;
		break;
	case AV_LOG_INFO:
		level = CE_LOGGING_LEVEL_INFO;
		break;
	case AV_LOG_DEBUG:
		level = CE_LOGGING_LEVEL_DEBUG;
		break;
	default:
		level = CE_LOGGING_LEVEL_WRITE;
		break;
	}
	char buffer[strlen(format) + 32];
	snprintf(buffer, sizeof(buffer), "bink: libAVcodec reported that %s", format);
	ce_logging_report_va(level, buffer, args);
}

static bool ce_bink_ctor(ce_soundresource* soundresource)
{
	ce_bink* bink = (ce_bink*)soundresource->impl;

	if (!ce_binkheader_read(&bink->header, soundresource->memfile)) {
		ce_logging_error("bink: input does not appear to be a Bink audio");
		return false;
	}

	if (1 != bink->header.audio_track_count) {
		ce_logging_error("bink: only one audio track supported");
		return false;
	}

	if (!ce_binktrack_read(&bink->track, soundresource->memfile)) {
		ce_logging_error("bink: input does not appear to be a Bink audio");
		return false;
	}

	// only RDFT supported (all original EI BIKs use RDFT)
	if (CE_BINK_AUDIO_FLAG_USE_DCT & bink->track.flags) {
		ce_logging_error("bink: DCT audio algorithm not supported");
		return false;
	}

	soundresource->bits_per_sample = 16;
	soundresource->sample_rate = bink->track.sample_rate;
	soundresource->channel_count = CE_BINK_AUDIO_FLAG_STEREO &
									bink->track.flags ? 2 : 1;

	ce_logging_debug("bink: input is %u Hz, %u channel",
		soundresource->sample_rate, soundresource->channel_count);

	bink->indices = (ce_binkindex*)bink->data;

	if (!ce_binkindex_read(bink->indices, bink->header.frame_count, soundresource->memfile)) {
		ce_logging_error("bink: invalid frame index table");
		return false;
	}

	av_log_set_callback(ce_bink_error);

	avcodec_init();
	avcodec_register_all();

	bink->codec = avcodec_find_decoder(CODEC_ID_BINKAUDIO_RDFT);
	if (NULL == bink->codec) {
		ce_logging_error("bink: RDFT audio codec not found");
		return false;
	}

	bink->codec_context = avcodec_alloc_context();
	bink->codec_context->codec_tag = bink->header.four_cc;
	bink->codec_context->sample_rate = soundresource->sample_rate;
	bink->codec_context->channels = soundresource->channel_count;

	if (avcodec_open(bink->codec_context, bink->codec) < 0) {
		ce_logging_error("bink: could not open RDFT audio codec");
		return false;
	}

	av_init_packet(&bink->packet);
	bink->packet.data = bink->data + sizeof(ce_binkindex) * bink->header.frame_count;

	return true;
}

static void ce_bink_dtor(ce_soundresource* soundresource)
{
	ce_bink* bink = (ce_bink*)soundresource->impl;

	if (NULL != bink->codec_context) {
		avcodec_close(bink->codec_context);
		av_free(bink->codec_context);
	}
}

static bool ce_bink_decode_frame(ce_soundresource* soundresource)
{
	ce_bink* bink = (ce_bink*)soundresource->impl;

	if (bink->frame_index == bink->header.frame_count) {
		assert(ce_memfile_eof(soundresource->memfile));
		return false;
	}

	uint32_t packet_size;
	ce_memfile_read(soundresource->memfile, &packet_size, 4, 1);

	// our input buffer is largest_frame_size bytes
	assert(packet_size <= bink->header.largest_frame_size);

	if (0 != packet_size) {
		bink->packet.size = ce_memfile_read(soundresource->memfile,
											bink->packet.data, 1, packet_size);

		int output_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
		int input_size = avcodec_decode_audio3(bink->codec_context,
			(int16_t*)bink->output, &output_size, &bink->packet);

		if (input_size != (int)packet_size || output_size <= 0) {
			ce_logging_error("bink: error while decoding");
		} else {
			bink->output_pos = 0;
			bink->output_size = output_size;
		}
	}

	uint32_t frame_size = bink->indices[bink->frame_index++].length;
	frame_size -= packet_size + sizeof(packet_size);

	// skip video packet
	ce_memfile_seek(soundresource->memfile, frame_size, CE_MEMFILE_SEEK_CUR);

	return 0 == packet_size;
}

static size_t ce_bink_read(ce_soundresource* soundresource, void* data, size_t size)
{
	ce_bink* bink = (ce_bink*)soundresource->impl;

	if (0 == bink->output_size) {
		do { /* nothing */ } while (ce_bink_decode_frame(soundresource));
	}

	size = ce_smin(size, bink->output_size);
	memcpy(data, bink->output + bink->output_pos, size);

	bink->output_pos += size;
	bink->output_size -= size;

	return size;
}

static bool ce_bink_reset(ce_soundresource* soundresource)
{
	ce_bink* bink = (ce_bink*)soundresource->impl;

	bink->frame_index = 0;
	ce_memfile_seek(soundresource->memfile,
		bink->indices[bink->frame_index].pos, CE_MEMFILE_SEEK_SET);

	return true;
}
#endif /* CE_ENABLE_PROPRIETARY */

const ce_soundresource_vtable ce_soundresource_builtins[] = {
	{ce_vorbis_size_hint, ce_vorbis_test, ce_vorbis_ctor,
	ce_vorbis_dtor, ce_vorbis_read, ce_vorbis_reset},
#ifdef CE_ENABLE_PROPRIETARY
	{ce_mad_size_hint, ce_mad_test, ce_mad_ctor,
	ce_mad_dtor, ce_mad_read, ce_mad_reset},
	{ce_bink_size_hint, ce_bink_test, ce_bink_ctor,
	ce_bink_dtor, ce_bink_read, ce_bink_reset},
#endif
};

const size_t CE_SOUNDRESOURCE_BUILTIN_COUNT = sizeof(ce_soundresource_builtins) /
											sizeof(ce_soundresource_builtins[0]);
