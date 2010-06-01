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

#if defined(__MINGW32__) && defined(__STRICT_ANSI__)
// HACK: add fseeko64 function prototype
#include <sys/types.h>
int __cdecl __MINGW_NOTHROW fseeko64(FILE*, off64_t, int);
#endif

#include <vorbis/vorbisfile.h>

#ifdef CE_NONFREE
#include <mad.h>
#endif

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "cebyteorder.h"
#include "cesoundresource.h"

typedef struct {
	OggVorbis_File vf;
	int bitstream;
} ce_soundresource_vorbis;

static bool ce_soundresource_vorbis_ctor(ce_soundresource* soundresource, va_list args)
{
	ce_soundresource_vorbis* vorbisresource = (ce_soundresource_vorbis*)soundresource->impl;

	ce_unused(OV_CALLBACKS_NOCLOSE);
	ce_unused(OV_CALLBACKS_STREAMONLY);
	ce_unused(OV_CALLBACKS_STREAMONLY_NOCLOSE);

	if (0 != ov_open_callbacks(va_arg(args, FILE*), &vorbisresource->vf, NULL, 0, OV_CALLBACKS_DEFAULT)) {
		ce_logging_error("soundresource (vorbis): input does not appear to be an ogg bitstream");
		return false;
	}

	vorbis_info* info = ov_info(&vorbisresource->vf, -1);
	if (NULL == info) {
		ce_logging_error("soundresource (vorbis): could not get stream info");
		return false;
	}

	// a vorbis file has no particular number of bits per sample,
	// so use words, see also ov_read
	soundresource->bps = 16;
	soundresource->rate = info->rate;
	soundresource->channels = info->channels;

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
			return code;
		}
		ce_logging_warning("soundresource (vorbis): error in the stream");
	}
}

#ifdef CE_NONFREE
enum {
	CE_SOUNDRESOURCE_MAD_INPUT_BUFFER_CAPACITY = 5 * 8192,
	CE_SOUNDRESOURCE_MAD_OUTPUT_BUFFER_CAPACITY = 8192,
};

typedef struct {
	FILE* file;
	struct mad_stream stream;
	struct mad_frame frame;
	struct mad_synth synth;
	size_t output_buffer_size;
	unsigned char* output_buffer;
	unsigned char* output_buffer2;
	unsigned char input_buffer[];
} ce_soundresource_mad;

static void ce_soundresource_mad_error(ce_soundresource_mad* madresource)
{
	ce_logging_error("soundresource (mad): decoding error 0x%04x (%s)",
		madresource->stream.error, mad_stream_errorstr(&madresource->stream));
}

static void ce_soundresource_mad_input(ce_soundresource_mad* madresource)
{
	size_t remaining = 0;
	size_t size = CE_SOUNDRESOURCE_MAD_INPUT_BUFFER_CAPACITY;

	if (NULL != madresource->stream.next_frame) {
		remaining = madresource->stream.bufend -
					madresource->stream.next_frame;

		memmove(madresource->input_buffer,
			madresource->stream.next_frame, remaining);

		ce_logging_debug("soundresource: memmove sz %lu rem %lu", size, remaining);
	}

	size = fread(madresource->input_buffer + remaining,
				1, size - remaining, madresource->file);

	if (0 == size) {
		ce_logging_debug("soundresource: eof");
		return /* EOF */;
	}

	// TODO if eof
	//memset(madresource->input_buffer + remaining + size, 0, MAD_BUFFER_GUARD);
	//size += MAD_BUFFER_GUARD;

	ce_logging_debug("soundresource: mad_stream_buffer sz %lu rem %lu", size, remaining);
	mad_stream_buffer(&madresource->stream, madresource->input_buffer, size + remaining);
}

// performs simple clipping or scaling down to 16 bits
// no dithering or noise shaping!
static inline int16_t ce_soundresource_mad_scale(mad_fixed_t sample)
{
	return sample >= MAD_F_ONE ? INT16_MAX :
			(sample <= -MAD_F_ONE ? INT16_MIN :
				sample >> (MAD_F_FRACBITS + 1 - 16));
}

static void ce_soundresource_mad_decode(ce_soundresource_mad* madresource)
{
	madresource->output_buffer_size = 0;

	while (-1 == mad_frame_decode(&madresource->frame, &madresource->stream)) {
		if (MAD_ERROR_BUFLEN == madresource->stream.error) {
			// the input bucket must be filled if it becomes empty
			ce_soundresource_mad_input(madresource);
			madresource->stream.error = MAD_ERROR_NONE;
		} else {
			ce_soundresource_mad_error(madresource);
			if (!MAD_RECOVERABLE(madresource->stream.error)) {
				return;
			}
		}
	}

	mad_synth_frame(&madresource->synth, &madresource->frame);

	unsigned int sample_count  = madresource->synth.pcm.length;
	unsigned int channel_count = madresource->synth.pcm.channels;

	int16_t* output_buffer = (int16_t*)madresource->output_buffer;
	madresource->output_buffer_size = 2 * sample_count * channel_count;

	ce_logging_debug("soundresource: %lu %u",
		madresource->output_buffer_size, CE_SOUNDRESOURCE_MAD_OUTPUT_BUFFER_CAPACITY);

	if (madresource->output_buffer_size > CE_SOUNDRESOURCE_MAD_OUTPUT_BUFFER_CAPACITY) {
		madresource->output_buffer_size = CE_SOUNDRESOURCE_MAD_OUTPUT_BUFFER_CAPACITY;
		ce_logging_critical("soundresource (mad): output buffer overflow, truncated");
		assert(false);
	}

	// convert libmad's fixed point number to the consumer format
	// output sample(s) in 16-bit signed little-endian PCM
	for (unsigned int i = 0; i < sample_count; ++i) {
		for (unsigned int j = 0; j < channel_count; ++j) {
			output_buffer[channel_count * i + j] =
				ce_soundresource_mad_scale(madresource->synth.pcm.samples[j][i]);
		}
	}
}

static bool ce_soundresource_mad_ctor(ce_soundresource* soundresource, va_list args)
{
	ce_soundresource_mad* madresource = (ce_soundresource_mad*)soundresource->impl;
	madresource->file = va_arg(args, FILE*);

	madresource->output_buffer = madresource->input_buffer + MAD_BUFFER_GUARD +
								CE_SOUNDRESOURCE_MAD_INPUT_BUFFER_CAPACITY;

	mad_stream_init(&madresource->stream);
	mad_frame_init(&madresource->frame);
	mad_synth_init(&madresource->synth);

	ce_soundresource_mad_input(madresource);

	if (-1 == mad_header_decode(&madresource->frame.header,
								&madresource->stream)) {
		ce_soundresource_mad_error(madresource);
		return false;
	}

	soundresource->bps = 16;
	soundresource->rate = madresource->frame.header.samplerate;
	soundresource->channels = MAD_MODE_SINGLE_CHANNEL ==
								madresource->frame.header.mode ? 1 : 2;

	return true;
}

static void ce_soundresource_mad_dtor(ce_soundresource* soundresource)
{
	ce_soundresource_mad* madresource = (ce_soundresource_mad*)soundresource->impl;

	mad_synth_finish(&madresource->synth);
	mad_frame_finish(&madresource->frame);
	mad_stream_finish(&madresource->stream);
}

static size_t ce_soundresource_mad_read(ce_soundresource* soundresource, void* data, size_t size)
{
	ce_soundresource_mad* madresource = (ce_soundresource_mad*)soundresource->impl;

	if (0 == madresource->output_buffer_size) {
		ce_soundresource_mad_decode(madresource);
		madresource->output_buffer2 = madresource->output_buffer;
	}

	size = ce_smin(size, madresource->output_buffer_size);
	madresource->output_buffer_size -= size;

	//memcpy(data, madresource->output_buffer +
	//		madresource->output_buffer_size - size, size);
	memcpy(data, madresource->output_buffer2, size);
	madresource->output_buffer2 += size;

	return size;
}
#endif

const ce_soundresource_vtable ce_soundresource_builtins[] = {
	{sizeof(ce_soundresource_vorbis), ce_soundresource_vorbis_ctor,
	ce_soundresource_vorbis_dtor, ce_soundresource_vorbis_read, NULL},
#ifdef CE_NONFREE
	{sizeof(ce_soundresource_mad) + CE_SOUNDRESOURCE_MAD_INPUT_BUFFER_CAPACITY +
	MAD_BUFFER_GUARD + CE_SOUNDRESOURCE_MAD_OUTPUT_BUFFER_CAPACITY,
	ce_soundresource_mad_ctor, ce_soundresource_mad_dtor,
	ce_soundresource_mad_read, NULL},
#endif
};

const size_t CE_SOUNDRESOURCE_BUILTIN_COUNT = sizeof(ce_soundresource_builtins) /
											sizeof(ce_soundresource_builtins[0]);
