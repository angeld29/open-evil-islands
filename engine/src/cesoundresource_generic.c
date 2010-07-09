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
 *  Supported decoders:
 *  1. Ogg Vorbis (lossy compression).
 *  2. FLAC (lossless compression).
 *  3. WAVE (limited, for backward compatibility with original EI resources).
 *  4. MPEG (limited, for backward compatibility with original EI resources).
 *  5. Bink (limited, for backward compatibility with original EI resources).
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

#include <FLAC/stream_decoder.h>

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
#include "cewave.h"
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
} ce_vorbis;

static size_t ce_vorbis_read_wrap(void* ptr, size_t size, size_t nmemb, void* datasource)
{
	return ce_mem_file_read(datasource, ptr, size, nmemb);
}

static int ce_vorbis_seek_wrap(void* datasource, ogg_int64_t offset, int whence)
{
	return ce_mem_file_seek(datasource, offset, whence);
}

static long ce_vorbis_tell_wrap(void* datasource)
{
	return ce_mem_file_tell(datasource);
}

static bool ce_vorbis_test(ce_sound_probe* sound_probe)
{
	char fourcc[4];
	if (4 != ce_mem_file_read(sound_probe->mem_file, fourcc, 1, 4) ||
			0 != memcmp(fourcc, "OggS", 4)) {
		return false;
	}

	ce_mem_file_rewind(sound_probe->mem_file);

	OggVorbis_File vf;
	if (0 == ov_test_callbacks(sound_probe->mem_file, &vf, NULL, 0, (ov_callbacks)
			{ce_vorbis_read_wrap, ce_vorbis_seek_wrap, NULL, ce_vorbis_tell_wrap})) {
		sound_probe->size = sizeof(ce_vorbis);
		assert(sizeof(OggVorbis_File) <= CE_SOUND_PROBE_BUFFER_CAPACITY);
		memcpy(sound_probe->buffer, &vf, sizeof(OggVorbis_File));
		return true;
	}

	ov_clear(&vf);
	return false;
}

static bool ce_vorbis_ctor(ce_sound_resource* sound_resource, ce_sound_probe* sound_probe)
{
	ce_vorbis* vorbis = (ce_vorbis*)sound_resource->impl;
	memcpy(&vorbis->vf, sound_probe->buffer, sizeof(OggVorbis_File));

	if (0 != ov_test_open(&vorbis->vf)) {
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
	ce_sound_format_init(&sound_resource->sound_format, 16, info->rate, info->channels);

	ce_logging_debug("vorbis: audio is %ld bits per second (%ld bits per second nominal)",
		ov_bitrate(&vorbis->vf, -1), info->bitrate_nominal);

	return true;
}

static void ce_vorbis_dtor(ce_sound_resource* sound_resource)
{
	ce_vorbis* vorbis = (ce_vorbis*)sound_resource->impl;
	ov_clear(&vorbis->vf);
}

static size_t ce_vorbis_read(ce_sound_resource* sound_resource, void* data, size_t size)
{
	ce_vorbis* vorbis = (ce_vorbis*)sound_resource->impl;

	for (;;) {
		long code = ov_read(&vorbis->vf, data, size,
					ce_is_big_endian(), 2, 1, &vorbis->bitstream);
		if (code >= 0) {
			return code;
		}
		ce_logging_warning("vorbis: error in the stream");
	}
}

static bool ce_vorbis_reset(ce_sound_resource* sound_resource)
{
	ce_vorbis* vorbis = (ce_vorbis*)sound_resource->impl;
	// infinite loop occurs if seeking at position 0 with multi stream oggs
	return 0 == ov_raw_seek(&vorbis->vf, 1);
}

/*
 *  FLAC: Free Lossless Audio Codec (C) 2000-2007 Josh Coalson
 *
 *  See also:
 *  1. http://flac.sourceforge.net/
 *  2. http://flac.sourceforge.net/api/index.html
*/

typedef struct {
	FLAC__StreamDecoder* decoder;
	size_t output_buffer_pos, output_buffer_size;
	uint8_t output_buffer[FLAC__MAX_BLOCK_SIZE * 4 /*FIXME: sample size*/];
} ce_flac;

static bool ce_flac_test(ce_sound_probe* sound_probe)
{
	char fourcc[4];
	sound_probe->size = sizeof(ce_flac);
	return 4 == ce_mem_file_read(sound_probe->mem_file, fourcc, 1, 4) &&
			0 == memcmp(fourcc, "fLaC", 4);
}

FLAC__StreamDecoderReadStatus ce_flac_read_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
	FLAC__byte buffer[], size_t* bytes, void* client_data)
{
	ce_sound_resource* sound_resource = client_data;

	if (0 == *bytes) {
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
	}

	if (ce_mem_file_eof(sound_resource->mem_file)) {
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	}

	*bytes = ce_mem_file_read(sound_resource->mem_file, buffer, sizeof(FLAC__byte), *bytes);

	if (ce_mem_file_error(sound_resource->mem_file)) {
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
	}

	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderSeekStatus ce_flac_seek_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
	FLAC__uint64 absolute_byte_offset, void *client_data)
{
	ce_sound_resource* sound_resource = client_data;
	return ce_mem_file_seek(sound_resource->mem_file,
		absolute_byte_offset, CE_MEM_FILE_SEEK_SET) < 0 ?
		FLAC__STREAM_DECODER_SEEK_STATUS_ERROR :
		FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

FLAC__StreamDecoderTellStatus ce_flac_tell_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
	FLAC__uint64* absolute_byte_offset, void* client_data)
{
	ce_sound_resource* sound_resource = client_data;
	long int pos = ce_mem_file_tell(sound_resource->mem_file);

	if (pos < 0) {
		return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
	}

	*absolute_byte_offset = pos;
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus ce_flac_length_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
	FLAC__uint64* stream_length, void* client_data)
{
	ce_sound_resource* sound_resource = client_data;
	long int size = ce_mem_file_size(sound_resource->mem_file);

	if (size < 0) {
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
	}

	*stream_length = size;
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

FLAC__bool ce_flac_eof_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder), void* client_data)
{
	ce_sound_resource* sound_resource = client_data;
	return ce_mem_file_eof(sound_resource->mem_file) ? true : false;
}

FLAC__StreamDecoderWriteStatus ce_flac_write_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
	const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data)
{
	ce_sound_resource* sound_resource = client_data;
	ce_flac* flac = (ce_flac*)sound_resource->impl;
	FLAC__int16* output_buffer = (FLAC__int16*)flac->output_buffer;

	flac->output_buffer_pos = 0;
	flac->output_buffer_size = sound_resource->sound_format.sample_size * frame->header.blocksize;

	if (flac->output_buffer_size > sizeof(flac->output_buffer)) {
		ce_logging_debug("blocksize %u, channels %u, out sz %zu, max %zu",
			frame->header.blocksize, frame->header.channels,
			flac->output_buffer_size, sizeof(flac->output_buffer));
	}

	assert(flac->output_buffer_size <= sizeof(flac->output_buffer));

	for (unsigned int i = 0; i < frame->header.blocksize; ++i) {
		for (unsigned int j = 0; j < frame->header.channels; ++j) {
			*output_buffer++ = buffer[j][i];
		}
	}

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void ce_flac_metadata_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
	const FLAC__StreamMetadata* metadata, void* client_data)
{
	ce_sound_resource* sound_resource = client_data;

	if (FLAC__METADATA_TYPE_STREAMINFO == metadata->type) {
		ce_sound_format_init(&sound_resource->sound_format,
								metadata->data.stream_info.bits_per_sample,
								metadata->data.stream_info.sample_rate,
								metadata->data.stream_info.channels);
	}
}

void ce_flac_error_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
	FLAC__StreamDecoderErrorStatus status, void* CE_UNUSED(client_data))
{
	ce_logging_error("flac: an error %d occurred during decompression", status);
}

static bool ce_flac_ctor(ce_sound_resource* sound_resource, ce_sound_probe* CE_UNUSED(sound_probe))
{
	ce_flac* flac = (ce_flac*)sound_resource->impl;
	flac->decoder = FLAC__stream_decoder_new();

	ce_mem_file_rewind(sound_resource->mem_file);

	if (FLAC__STREAM_DECODER_INIT_STATUS_OK != FLAC__stream_decoder_init_stream(flac->decoder,
			ce_flac_read_callback, ce_flac_seek_callback, ce_flac_tell_callback,
			ce_flac_length_callback, ce_flac_eof_callback, ce_flac_write_callback,
			ce_flac_metadata_callback, ce_flac_error_callback, sound_resource)) {
		ce_logging_error("flac: input does not appear to be a FLAC audio");
		return false;
	}

	if (!FLAC__stream_decoder_process_until_end_of_metadata(flac->decoder)) {
		ce_logging_error("flac: process_until_end_of_metadata failed");
		return false;
	}

	return true;
}

static void ce_flac_dtor(ce_sound_resource* sound_resource)
{
	ce_flac* flac = (ce_flac*)sound_resource->impl;

	if (NULL != flac->decoder) {
		if (!FLAC__stream_decoder_finish(flac->decoder)) {
			// TODO: FLAC__stream_decoder_get_state(flac->decoder)
			ce_logging_error("flac: finishing the decoding process failed");
		}
		FLAC__stream_decoder_delete(flac->decoder);
	}
}

static void ce_flac_decode(ce_sound_resource* sound_resource)
{
	ce_flac* flac = (ce_flac*)sound_resource->impl;

	if (!FLAC__stream_decoder_process_single(flac->decoder)) {
		// TODO: FLAC__stream_decoder_get_state(flac->decoder)
		ce_logging_error("flac: decode one metadata block or audio frame failed");
	}
}

static size_t ce_flac_read(ce_sound_resource* sound_resource, void* data, size_t size)
{
	ce_flac* flac = (ce_flac*)sound_resource->impl;

	if (0 == flac->output_buffer_size) {
		ce_flac_decode(sound_resource);
	}

	size = ce_min(size_t, size, flac->output_buffer_size);
	memcpy(data, flac->output_buffer + flac->output_buffer_pos, size);

	flac->output_buffer_pos += size;
	flac->output_buffer_size -= size;

	return size;
}

static bool ce_flac_reset(ce_sound_resource* sound_resource)
{
	ce_flac* flac = (ce_flac*)sound_resource->impl;

	if (!FLAC__stream_decoder_reset(flac->decoder)) {
		// TODO: FLAC__stream_decoder_get_state(flac->decoder)
		ce_logging_error("flac: resetting the decoding process failed");
	}

	return false;
}

/*
 *  Waveform Audio File Format (C) Microsoft & IBM
 *
 *  See also:
 *  1. cewave.h
*/

typedef struct {
	ce_wave_header wave_header;
	size_t output_buffer_pos, output_buffer_size;
	void* block;
	char data[];
} ce_wave;

static bool ce_wave_test(ce_sound_probe* sound_probe)
{
	ce_wave_header wave_header;
	if (ce_wave_header_read(&wave_header, sound_probe->mem_file)) {
		sound_probe->size = sizeof(ce_wave);
		if (CE_WAVE_FORMAT_IMA_ADPCM == wave_header.format.tag) {
			sound_probe->size += ce_wave_ima_adpcm_samples_storage_size(&wave_header);
			sound_probe->size += ce_wave_ima_adpcm_block_storage_size(&wave_header);
		}
		assert(sizeof(ce_wave_header) <= CE_SOUND_PROBE_BUFFER_CAPACITY);
		memcpy(sound_probe->buffer, &wave_header, sizeof(ce_wave_header));
		return true;
	}
	return false;
}

static bool ce_wave_ctor(ce_sound_resource* sound_resource, ce_sound_probe* sound_probe)
{
	ce_wave* wave = (ce_wave*)sound_resource->impl;
	memcpy(&wave->wave_header, sound_probe->buffer, sizeof(ce_wave_header));

	ce_sound_format_init(&sound_resource->sound_format,
						CE_WAVE_FORMAT_IMA_ADPCM == wave->wave_header.format.tag ?
						16 : wave->wave_header.format.bits_per_sample,
						wave->wave_header.format.samples_per_sec,
						wave->wave_header.format.channel_count);

	wave->block = wave->data + ce_wave_ima_adpcm_samples_storage_size(&wave->wave_header);

	return true;
}

static void ce_wave_dtor(ce_sound_resource* CE_UNUSED(sound_resource))
{
}

static size_t ce_wave_read(ce_sound_resource* sound_resource, void* data, size_t size)
{
	ce_wave* wave = (ce_wave*)sound_resource->impl;

	if (ce_mem_file_eof(sound_resource->mem_file)) {
		return 0;
	}

	if (CE_WAVE_FORMAT_PCM == wave->wave_header.format.tag &&
			wave->wave_header.format.channel_count == 2) {
		return ce_mem_file_read(sound_resource->mem_file, data, 1, size);
	}

	if (CE_WAVE_FORMAT_PCM == wave->wave_header.format.tag) {
		int16_t* s = (int16_t*)data;
		size_t i;
		for (i = 0; i < size; i += 2 * 2) {
			int16_t sample;
			ce_mem_file_read(sound_resource->mem_file, &sample, 2, 1);
			*s++ = sample;
			*s++ = sample;
		}
		return ce_clamp(size_t, i + 1, 0, size);
	}

	if (0 == wave->output_buffer_size) {
		ce_mem_file_read(sound_resource->mem_file, wave->block, 1, ce_wave_ima_adpcm_block_storage_size(&wave->wave_header));

		if (CE_WAVE_FORMAT_IMA_ADPCM == wave->wave_header.format.tag) {
			ce_wave_ima_adpcm_decode(wave->data, wave->block, &wave->wave_header);
		}

		wave->output_buffer_pos = 0;
		wave->output_buffer_size = 2 * wave->wave_header.format.extra.ima_adpcm.samples_per_block * wave->wave_header.format.channel_count;
	}

	if (1 == wave->wave_header.format.channel_count) {
		int16_t* s1 = (int16_t*)data;
		int16_t* s2 = (int16_t*)(wave->data + wave->output_buffer_pos);

		size = ce_min(size_t, size, wave->output_buffer_size * 2);
		size_t count = size / 4;

		for (size_t i = 0; i < count; ++i) {
			s1[2 * i + 0] = s2[i];
			s1[2 * i + 1] = s2[i];
		}

		wave->output_buffer_pos += size / 2;
		wave->output_buffer_size -= size / 2;
	} else {
		size = ce_min(size_t, size, wave->output_buffer_size);
		memcpy(data, wave->data + wave->output_buffer_pos, size);

		wave->output_buffer_pos += size;
		wave->output_buffer_size -= size;
	}

	return size;
}

static bool ce_wave_reset(ce_sound_resource* sound_resource)
{
	ce_wave* wave = (ce_wave*)sound_resource->impl;
	wave->output_buffer_pos = 0;
	wave->output_buffer_size = 0;
	return true;
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
	ce_mad_dither dither[2]; // for 2 channels
	ce_mad_stats stats;
	size_t output_buffer_size;
	unsigned char* output_buffer;
	unsigned char input_buffer[CE_MAD_DATA_SIZE];
} ce_mad;

static bool ce_mad_test(ce_sound_probe* sound_probe)
{
	unsigned char* buffer = ce_alloc(CE_MAD_INPUT_BUFFER_CAPACITY);
	size_t size = ce_mem_file_read(sound_probe->mem_file, buffer, 1, CE_MAD_INPUT_BUFFER_CAPACITY);

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
	bool result = MAD_ERROR_NONE == stream.error || MAD_RECOVERABLE(stream.error);

	mad_header_finish(&header);
	mad_stream_finish(&stream);

	ce_free(buffer, CE_MAD_INPUT_BUFFER_CAPACITY);

	sound_probe->size = sizeof(ce_mad);

	return result;
}

static void ce_mad_error(ce_sound_resource* sound_resource, ce_logging_level level)
{
	ce_mad* mad = (ce_mad*)sound_resource->impl;
	ce_logging_report(level, "mad: decoding error 0x%04x (%s)",
		mad->stream.error, mad_stream_errorstr(&mad->stream));
}

static bool ce_mad_input(ce_sound_resource* sound_resource)
{
	ce_mad* mad = (ce_mad*)sound_resource->impl;

	// libmad may not consume all bytes of the input buffer
	size_t remaining = 0;

	if (NULL != mad->stream.next_frame) {
		remaining = mad->stream.bufend - mad->stream.next_frame;
		memmove(mad->input_buffer, mad->stream.next_frame, remaining);
	}

	size_t size = ce_mem_file_read(sound_resource->mem_file,
		mad->input_buffer + remaining, 1,
		CE_MAD_INPUT_BUFFER_CAPACITY - remaining);

	if (ce_mem_file_error(sound_resource->mem_file)) {
		ce_error_report_c_last("mad");
		return false;
	}

	if (0 == size) {
		// EOF and no data available
		return false;
	}

	// when decoding the last frame of a file, it must be followed by
	// MAD_BUFFER_GUARD zero bytes if one wants to decode that last frame
	if (ce_mem_file_eof(sound_resource->mem_file)) {
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

static bool ce_mad_decode(ce_sound_resource* sound_resource)
{
	ce_mad* mad = (ce_mad*)sound_resource->impl;

	mad->output_buffer = mad->input_buffer +
		CE_MAD_INPUT_BUFFER_CAPACITY + CE_MAD_INPUT_BUFFER_GUARD;

	while (-1 == mad_frame_decode(&mad->frame, &mad->stream)) {
		if (MAD_ERROR_BUFLEN == mad->stream.error ||
				MAD_ERROR_BUFPTR == mad->stream.error) {
			// the input bucket must be filled if it becomes empty
			// or if it's the first execution of the function
			if (!ce_mad_input(sound_resource)) {
				return false;
			}
			// new input data loaded successfully, reset error code
			mad->stream.error = MAD_ERROR_NONE;
		} else {
			if (!MAD_RECOVERABLE(mad->stream.error)) {
				ce_mad_error(sound_resource, CE_LOGGING_LEVEL_ERROR);
				return false;
			}
			ce_mad_error(sound_resource, CE_LOGGING_LEVEL_DEBUG);
		}
	}

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

static bool ce_mad_ctor(ce_sound_resource* sound_resource, ce_sound_probe* CE_UNUSED(sound_probe))
{
	ce_mad* mad = (ce_mad*)sound_resource->impl;
	ce_mad_init(mad);

	ce_mem_file_rewind(sound_resource->mem_file);

	if (!ce_mad_decode(sound_resource)) {
		ce_logging_error("mad: input does not appear to be a MPEG audio");
		return false;
	}

	ce_sound_format_init(&sound_resource->sound_format,
						16, mad->frame.header.samplerate,
						MAD_MODE_SINGLE_CHANNEL == mad->frame.header.mode ? 1 : 2);

	ce_logging_debug("mad: audio is %lu bits per second", mad->frame.header.bitrate);

	return true;
}

static void ce_mad_dtor(ce_sound_resource* sound_resource)
{
	ce_mad* mad = (ce_mad*)sound_resource->impl;
	ce_mad_clean(mad);
}

static size_t ce_mad_read(ce_sound_resource* sound_resource, void* data, size_t size)
{
	ce_mad* mad = (ce_mad*)sound_resource->impl;

	if (0 == mad->output_buffer_size) {
		ce_mad_decode(sound_resource);
	}

	size = ce_min(size_t, size, mad->output_buffer_size);
	memcpy(data, mad->output_buffer, size);

	mad->output_buffer_size -= size;
	mad->output_buffer += size;

	return size;
}

static bool ce_mad_reset(ce_sound_resource* sound_resource)
{
	ce_mad* mad = (ce_mad*)sound_resource->impl;

	ce_mad_clean(mad);
	ce_mad_init(mad);

	return true;
}
#endif /* CE_ENABLE_PROPRIETARY */

/*
 *  Bink Audio (C) RAD Game Tools, Inc.
 *
 *  See also:
 *  1. http://wiki.multimedia.cx/index.php?title=Bink_Audio
 *  2. FFmpeg (C) Michael Niedermayer
*/

typedef struct {
	size_t frame_index;
	ce_bink_header header;
	ce_bink_audio_track audio_track;
	ce_bink_index* indices;
	AVCodec* codec;
	AVCodecContext codec_context;
	AVPacket packet;
	size_t output_pos, output_size;
	uint8_t output[AVCODEC_MAX_AUDIO_FRAME_SIZE];
	uint8_t data[];
} ce_bink;

static bool ce_bink_test(ce_sound_probe* sound_probe)
{
	ce_bink_header header;
	if (ce_bink_header_read(&header, sound_probe->mem_file) && 0 != header.audio_track_count) {
		sound_probe->size = sizeof(ce_bink) +
			sizeof(ce_bink_index) * header.frame_count +
			header.largest_frame_size + FF_INPUT_BUFFER_PADDING_SIZE;
		assert(sizeof(ce_bink_header) <= CE_SOUND_PROBE_BUFFER_CAPACITY);
		memcpy(sound_probe->buffer, &header, sizeof(ce_bink_header));
		return true;
	}
	return false;
}

static bool ce_bink_ctor(ce_sound_resource* sound_resource, ce_sound_probe* sound_probe)
{
	ce_bink* bink = (ce_bink*)sound_resource->impl;
	memcpy(&bink->header, sound_probe->buffer, sizeof(ce_bink_header));

	if (1 != bink->header.audio_track_count) {
		ce_logging_error("bink: only one audio track supported");
		return false;
	}

	if (!ce_bink_audio_track_read(&bink->audio_track, sound_resource->mem_file)) {
		ce_logging_error("bink: input does not appear to be a Bink audio");
		return false;
	}

	// only RDFT supported (all original EI BIKs use RDFT)
	if (CE_BINK_AUDIO_FLAG_USE_DCT & bink->audio_track.flags) {
		ce_logging_error("bink: DCT audio algorithm not supported");
		return false;
	}

	bink->indices = (ce_bink_index*)bink->data;
	if (!ce_bink_index_read(bink->indices, bink->header.frame_count, sound_resource->mem_file)) {
		ce_logging_error("bink: invalid frame index table");
		return false;
	}

	bink->codec = avcodec_find_decoder(CODEC_ID_BINKAUDIO_RDFT);
	if (NULL == bink->codec) {
		ce_logging_error("bink: RDFT audio codec not found");
		return false;
	}

	ce_sound_format_init(&sound_resource->sound_format,
						16, bink->audio_track.sample_rate,
						CE_BINK_AUDIO_FLAG_STEREO & bink->audio_track.flags ? 2 : 1);

	avcodec_get_context_defaults(&bink->codec_context);
	bink->codec_context.codec_tag = bink->header.four_cc;
	bink->codec_context.sample_rate = sound_resource->sound_format.samples_per_second;
	bink->codec_context.channels = sound_resource->sound_format.channel_count;

	if (avcodec_open(&bink->codec_context, bink->codec) < 0) {
		ce_logging_error("bink: could not open RDFT audio codec");
		return false;
	}

	av_init_packet(&bink->packet);
	bink->packet.data = bink->data + sizeof(ce_bink_index) * bink->header.frame_count;

	return true;
}

static void ce_bink_dtor(ce_sound_resource* sound_resource)
{
	ce_bink* bink = (ce_bink*)sound_resource->impl;

	if (NULL != bink->codec_context.codec) {
		avcodec_close(&bink->codec_context);
	}
}

static bool ce_bink_decode_frame(ce_sound_resource* sound_resource)
{
	ce_bink* bink = (ce_bink*)sound_resource->impl;

	if (bink->frame_index == bink->header.frame_count) {
		assert(ce_mem_file_eof(sound_resource->mem_file));
		return false;
	}

	uint32_t packet_size;
	ce_mem_file_read(sound_resource->mem_file, &packet_size, 4, 1);

	// our input buffer is largest_frame_size bytes
	assert(packet_size <= bink->header.largest_frame_size);
	if (packet_size > bink->header.largest_frame_size) {
		return false;
	}

	if (0 != packet_size) {
		bink->packet.size = ce_mem_file_read(sound_resource->mem_file,
											bink->packet.data, 1, packet_size);

		int size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
		int code = avcodec_decode_audio3(&bink->codec_context,
			(int16_t*)bink->output, &size, &bink->packet);

		if (code < 0 || (uint32_t)code != packet_size || size <= 0) {
			ce_logging_error("bink: codec error while decoding audio");
		} else {
			bink->output_pos = 0;
			bink->output_size = size;
		}
	}

	uint32_t frame_size = bink->indices[bink->frame_index++].length;
	frame_size -= packet_size + sizeof(packet_size);

	// skip video packet
	ce_mem_file_seek(sound_resource->mem_file, frame_size, CE_MEM_FILE_SEEK_CUR);

	return 0 == packet_size;
}

static size_t ce_bink_read(ce_sound_resource* sound_resource, void* data, size_t size)
{
	ce_bink* bink = (ce_bink*)sound_resource->impl;

	if (0 == bink->output_size) {
		do { ce_pass(); } while (ce_bink_decode_frame(sound_resource));
	}

	size = ce_min(size_t, size, bink->output_size);
	memcpy(data, bink->output + bink->output_pos, size);

	bink->output_pos += size;
	bink->output_size -= size;

	return size;
}

static bool ce_bink_reset(ce_sound_resource* sound_resource)
{
	ce_bink* bink = (ce_bink*)sound_resource->impl;

	bink->frame_index = 0;
	bink->output_size = 0;

	ce_mem_file_seek(sound_resource->mem_file,
		bink->indices[bink->frame_index].pos, CE_MEM_FILE_SEEK_SET);

	return true;
}

const ce_sound_resource_vtable ce_sound_resource_builtins[] = {
	{ce_vorbis_test,
	ce_vorbis_ctor, ce_vorbis_dtor,
	ce_vorbis_read, ce_vorbis_reset},
	{ce_flac_test,
	ce_flac_ctor, ce_flac_dtor,
	ce_flac_read, ce_flac_reset},
	{ce_wave_test,
	ce_wave_ctor, ce_wave_dtor,
	ce_wave_read, ce_wave_reset},
#ifdef CE_ENABLE_PROPRIETARY
	{ce_mad_test,
	ce_mad_ctor, ce_mad_dtor,
	ce_mad_read, ce_mad_reset},
#endif
	{ce_bink_test,
	ce_bink_ctor, ce_bink_dtor,
	ce_bink_read, ce_bink_reset},
};

const size_t CE_SOUND_RESOURCE_BUILTIN_COUNT = sizeof(ce_sound_resource_builtins) /
											sizeof(ce_sound_resource_builtins[0]);
