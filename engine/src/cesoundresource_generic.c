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

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

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
        sound_probe->name = "ogg vorbis";
        sound_probe->impl_size = sizeof(ce_vorbis);
        sound_probe->output_buffer_capacity = 8192;

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

static bool ce_vorbis_decode(ce_sound_resource* sound_resource)
{
    ce_vorbis* vorbis = (ce_vorbis*)sound_resource->impl;

    for (;;) {
        long code = ov_read(&vorbis->vf, sound_resource->output_buffer,
                            sound_resource->output_buffer_capacity,
                            ce_is_big_endian(), 2, 1, &vorbis->bitstream);
        if (code >= 0) {
            sound_resource->output_buffer_size = code;
            return true;
        }
        ce_logging_warning("vorbis: error in the stream");
    }

    return false;
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
    unsigned int block_size, min_block_size, max_block_size;
    ce_sound_format sound_format;
    FLAC__StreamDecoder* decoder;
    ce_mem_file* mem_file;
    FLAC__int16* buffer;
} ce_flac_bundle;

typedef struct {
    ce_flac_bundle* bundle;
} ce_flac;

static ce_flac_bundle* ce_flac_bundle_new(void)
{
    ce_flac_bundle* flac_bundle = ce_alloc_zero(sizeof(ce_flac_bundle));
    flac_bundle->decoder = FLAC__stream_decoder_new();
    return flac_bundle;
}

static void ce_flac_bundle_del(ce_flac_bundle* flac_bundle)
{
    if (NULL != flac_bundle) {
        FLAC__stream_decoder_finish(flac_bundle->decoder);
        FLAC__stream_decoder_delete(flac_bundle->decoder);
        ce_free(flac_bundle, sizeof(ce_flac_bundle));
    }
}

static const char* ce_flac_state_to_string(FLAC__StreamDecoderState state)
{
    switch (state) {
    case FLAC__STREAM_DECODER_SEARCH_FOR_METADATA:
        return "the decoder is ready to search for metadata";
    case FLAC__STREAM_DECODER_READ_METADATA:
        return "the decoder is ready to or is in the process of reading metadata";
    case FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC:
        return "the decoder is ready to or is in the process of searching for the frame sync code";
    case FLAC__STREAM_DECODER_READ_FRAME:
        return "the decoder is ready to or is in the process of reading a frame";
    case FLAC__STREAM_DECODER_END_OF_STREAM:
        return "the decoder has reached the end of the stream";
    case FLAC__STREAM_DECODER_OGG_ERROR:
        return "an error occurred in the underlying Ogg layer";
    case FLAC__STREAM_DECODER_SEEK_ERROR:
        return "an error occurred while seeking";
    case FLAC__STREAM_DECODER_ABORTED:
        return "the decoder was aborted by the read callback";
    case FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR:
        return "an error occurred allocating memory";
    case FLAC__STREAM_DECODER_UNINITIALIZED:
        return "the decoder is in the uninitialized state";
    }
    return "unknown state";
}

static const char* ce_flac_get_state_string(const FLAC__StreamDecoder* decoder)
{
    return ce_flac_state_to_string(FLAC__stream_decoder_get_state(decoder));
}

static void ce_flac_error(const FLAC__StreamDecoder* decoder, const char* message)
{
    ce_logging_error("flac: %s", message);
    ce_logging_error("flac: %s", ce_flac_get_state_string(decoder));
}

static FLAC__StreamDecoderReadStatus ce_flac_read_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
    FLAC__byte buffer[], size_t* bytes, void* client_data)
{
    ce_flac_bundle* flac_bundle = client_data;

    if (0 == *bytes) {
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }

    if (ce_mem_file_eof(flac_bundle->mem_file)) {
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }

    *bytes = ce_mem_file_read(flac_bundle->mem_file, buffer, sizeof(FLAC__byte), *bytes);

    if (ce_mem_file_error(flac_bundle->mem_file)) {
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }

    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

static FLAC__StreamDecoderSeekStatus ce_flac_seek_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
    FLAC__uint64 absolute_byte_offset, void *client_data)
{
    ce_flac_bundle* flac_bundle = client_data;
    return ce_mem_file_seek(flac_bundle->mem_file,
        absolute_byte_offset, CE_MEM_FILE_SEEK_SET) < 0 ?
        FLAC__STREAM_DECODER_SEEK_STATUS_ERROR :
        FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

static FLAC__StreamDecoderTellStatus ce_flac_tell_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
    FLAC__uint64* absolute_byte_offset, void* client_data)
{
    ce_flac_bundle* flac_bundle = client_data;
    long int pos = ce_mem_file_tell(flac_bundle->mem_file);

    if (pos < 0) {
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
    }

    *absolute_byte_offset = pos;
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

static FLAC__StreamDecoderLengthStatus ce_flac_length_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
    FLAC__uint64* stream_length, void* client_data)
{
    ce_flac_bundle* flac_bundle = client_data;
    long int size = ce_mem_file_size(flac_bundle->mem_file);

    if (size < 0) {
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    }

    *stream_length = size;
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

static FLAC__bool ce_flac_eof_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder), void* client_data)
{
    ce_flac_bundle* flac_bundle = client_data;
    return ce_mem_file_eof(flac_bundle->mem_file) ? true : false;
}

static FLAC__StreamDecoderWriteStatus ce_flac_write_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
    const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data)
{
    ce_flac_bundle* flac_bundle = client_data;
    flac_bundle->block_size = frame->header.blocksize;

    for (unsigned int i = 0; i < frame->header.blocksize; ++i) {
        for (unsigned int j = 0; j < frame->header.channels; ++j) {
            flac_bundle->buffer[i * frame->header.channels + j] = buffer[j][i];
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void ce_flac_metadata_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
    const FLAC__StreamMetadata* metadata, void* client_data)
{
    ce_flac_bundle* flac_bundle = client_data;

    if (FLAC__METADATA_TYPE_STREAMINFO == metadata->type) {
        flac_bundle->min_block_size = metadata->data.stream_info.min_blocksize;
        flac_bundle->max_block_size = metadata->data.stream_info.max_blocksize;
        ce_sound_format_init(&flac_bundle->sound_format,
                                metadata->data.stream_info.bits_per_sample,
                                metadata->data.stream_info.sample_rate,
                                metadata->data.stream_info.channels);
    }
}

static void ce_flac_error_callback(const FLAC__StreamDecoder* CE_UNUSED(decoder),
    FLAC__StreamDecoderErrorStatus status, void* CE_UNUSED(client_data))
{
    const char* status_string = "unknown status";

    switch (status) {
    case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
        status_string = "an error in the stream caused the decoder to lose synchronization";
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
        status_string = "the decoder encountered a corrupted frame header";
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
        status_string = "the frame's data did not match the CRC in the footer";
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
        status_string = "the decoder encountered reserved fields in use in the stream";
        break;
    }

    ce_logging_error("flac: an error occurred during decompression");
    ce_logging_error("flac: %s", status_string);
}

static bool ce_flac_test(ce_sound_probe* sound_probe)
{
    char four_cc[4];
    if (4 != ce_mem_file_read(sound_probe->mem_file, four_cc, 1, 4) ||
            0 != memcmp(four_cc, "fLaC", 4)) {
        return false;
    }

    ce_mem_file_rewind(sound_probe->mem_file);

    ce_flac_bundle* flac_bundle = ce_flac_bundle_new();
    flac_bundle->mem_file = sound_probe->mem_file;

    if (FLAC__STREAM_DECODER_INIT_STATUS_OK != FLAC__stream_decoder_init_stream(flac_bundle->decoder,
            ce_flac_read_callback, ce_flac_seek_callback, ce_flac_tell_callback,
            ce_flac_length_callback, ce_flac_eof_callback, ce_flac_write_callback,
            ce_flac_metadata_callback, ce_flac_error_callback, flac_bundle) ||
            !FLAC__stream_decoder_process_until_end_of_metadata(flac_bundle->decoder)) {
        ce_flac_bundle_del(flac_bundle);
        return false;
    }

    sound_probe->name = "flac";
    sound_probe->impl_size = sizeof(ce_flac);
    sound_probe->output_buffer_capacity = flac_bundle->sound_format.sample_size *
                                            flac_bundle->max_block_size;

    assert(sizeof(ce_flac_bundle*) <= CE_SOUND_PROBE_BUFFER_CAPACITY);
    memcpy(sound_probe->buffer, &flac_bundle, sizeof(ce_flac_bundle*));

    return true;
}

static bool ce_flac_ctor(ce_sound_resource* sound_resource, ce_sound_probe* CE_UNUSED(sound_probe))
{
    ce_flac* flac = (ce_flac*)sound_resource->impl;
    memcpy(&flac->bundle, sound_probe->buffer, sizeof(ce_flac_bundle*));
    flac->bundle->buffer = (FLAC__int16*)sound_resource->output_buffer;
    sound_resource->sound_format = flac->bundle->sound_format;
    return true;
}

static void ce_flac_dtor(ce_sound_resource* sound_resource)
{
    ce_flac* flac = (ce_flac*)sound_resource->impl;
    ce_flac_bundle_del(flac->bundle);
}

static bool ce_flac_decode(ce_sound_resource* sound_resource)
{
    ce_flac* flac = (ce_flac*)sound_resource->impl;

    if (!FLAC__stream_decoder_process_single(flac->bundle->decoder)) {
        ce_flac_error(flac->bundle->decoder, "fatal read, write, or memory allocation error occurred");
        return false;
    }

    if (FLAC__STREAM_DECODER_END_OF_STREAM == FLAC__stream_decoder_get_state(flac->bundle->decoder)) {
        return false;
    }

    sound_resource->output_buffer_size = flac->bundle->block_size *
                                        flac->bundle->sound_format.sample_size;
    return true;
}

static bool ce_flac_reset(ce_sound_resource* sound_resource)
{
    ce_flac* flac = (ce_flac*)sound_resource->impl;

    if (!FLAC__stream_decoder_reset(flac->bundle->decoder)) {
        ce_flac_error(flac->bundle->decoder, "resetting the decoding process failed");
        return false;
    }

    return true;
}

/*
 *  Waveform Audio File Format (C) Microsoft & IBM
 *
 *  See also:
 *  1. cewave.h
*/

typedef struct {
    ce_wave_header wave_header;
    long int data_offset;
} ce_wave;

static bool ce_wave_test(ce_sound_probe* sound_probe)
{
    ce_wave_header wave_header;
    if (ce_wave_header_read(&wave_header, sound_probe->mem_file)) {
        sound_probe->name = "wave";
        sound_probe->impl_size = sizeof(ce_wave);

        switch (wave_header.format.tag) {
        case CE_WAVE_FORMAT_PCM:
            sound_probe->output_buffer_capacity = 8192;
            break;

        case CE_WAVE_FORMAT_IMA_ADPCM:
            sound_probe->input_buffer_capacity = ce_wave_ima_adpcm_block_storage_size(&wave_header);
            sound_probe->output_buffer_capacity = ce_wave_ima_adpcm_samples_storage_size(&wave_header);
            break;
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

    wave->data_offset = ce_mem_file_tell(sound_resource->mem_file);

    return true;
}

static bool ce_wave_decode(ce_sound_resource* sound_resource)
{
    ce_wave* wave = (ce_wave*)sound_resource->impl;

    switch (wave->wave_header.format.tag) {
    case CE_WAVE_FORMAT_PCM:
        sound_resource->output_buffer_size = ce_mem_file_read(sound_resource->mem_file,
            sound_resource->output_buffer, 1, sound_resource->output_buffer_capacity);
        return true;

    case CE_WAVE_FORMAT_IMA_ADPCM:
        if (ce_mem_file_eof(sound_resource->mem_file)) {
            return false;
        }

        sound_resource->output_buffer_size = sound_resource->output_buffer_capacity;
        ce_mem_file_read(sound_resource->mem_file,
            sound_resource->input_buffer, 1, sound_resource->input_buffer_capacity);

        ce_wave_ima_adpcm_decode(sound_resource->output_buffer,
                                sound_resource->input_buffer, &wave->wave_header);
        return true;
    }

    return false;
}

static bool ce_wave_reset(ce_sound_resource* sound_resource)
{
    ce_wave* wave = (ce_wave*)sound_resource->impl;
    ce_mem_file_seek(sound_resource->mem_file, wave->data_offset, CE_MEM_FILE_SEEK_SET);
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
    CE_MAD_OUTPUT_BUFFER_CAPACITY = 8192,
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

    sound_probe->name = "mad";
    sound_probe->impl_size = sizeof(ce_mad);
    sound_probe->input_buffer_capacity = CE_MAD_INPUT_BUFFER_CAPACITY + MAD_BUFFER_GUARD;
    sound_probe->output_buffer_capacity = CE_MAD_OUTPUT_BUFFER_CAPACITY;

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
        memmove(sound_resource->input_buffer, mad->stream.next_frame, remaining);
    }

    size_t size = ce_mem_file_read(sound_resource->mem_file,
        sound_resource->input_buffer + remaining, 1,
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
        memset(sound_resource->input_buffer + remaining + size, 0, MAD_BUFFER_GUARD);
        size += MAD_BUFFER_GUARD;
    }

    mad_stream_buffer(&mad->stream,
        (const unsigned char*)sound_resource->input_buffer, remaining + size);

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

    sound_resource->output_buffer_size = 2 * sample_count * channel_count;
    assert(sound_resource->output_buffer_size <= sound_resource->output_buffer_capacity);

    // convert to signed 16 bit host endian integers
    int16_t* output_buffer = (int16_t*)sound_resource->output_buffer;

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
 *  1. cebink.h
 *  2. http://wiki.multimedia.cx/index.php?title=Bink_Audio
 *  3. FFmpeg (C) Michael Niedermayer
*/

typedef struct {
    size_t frame_index;
    ce_bink_header header;
    ce_bink_audio_track audio_track;
    AVCodec* codec;
    AVCodecContext* context;
    AVPacket packet;
    ce_bink_index indices[];
} ce_bink;

static bool ce_bink_test(ce_sound_probe* sound_probe)
{
    ce_bink_header header;
    if (ce_bink_header_read(&header, sound_probe->mem_file) && 0 != header.audio_track_count) {
        sound_probe->name = "bink";
        sound_probe->impl_size = sizeof(ce_bink) + sizeof(ce_bink_index) * header.frame_count;
        sound_probe->input_buffer_capacity = header.largest_frame_size + FF_INPUT_BUFFER_PADDING_SIZE;
        sound_probe->output_buffer_capacity = MAX_AUDIO_FRAME_SIZE;

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

    if (!ce_bink_index_read(bink->indices, bink->header.frame_count, sound_resource->mem_file)) {
        ce_logging_error("bink: invalid frame index table");
        return false;
    }

    ce_sound_format_init(&sound_resource->sound_format,
                        16, bink->audio_track.sample_rate,
                        CE_BINK_AUDIO_FLAG_STEREO & bink->audio_track.flags ? 2 : 1);

    bink->codec = avcodec_find_decoder(AV_CODEC_ID_BINKAUDIO_RDFT);
    if (NULL == bink->codec) {
        ce_logging_error("bink: RDFT audio codec not found");
        return false;
    }

    bink->context = avcodec_alloc_context3(bink->codec);
    if (NULL == bink->context) {
        ce_logging_error("bink: could not allocate context");
        return false;
    }

    if (avcodec_open2(bink->context, bink->codec, NULL) < 0) {
        ce_logging_error("bink: could not open RDFT audio codec");
        return false;
    }

    bink->context->codec_tag = bink->header.four_cc;
    bink->context->sample_rate = sound_resource->sound_format.samples_per_second;
    bink->context->channels = sound_resource->sound_format.channel_count;

    av_init_packet(&bink->packet);
    bink->packet.data = (uint8_t*)sound_resource->input_buffer;

    return true;
}

static void ce_bink_dtor(ce_sound_resource* sound_resource)
{
    ce_bink* bink = (ce_bink*)sound_resource->impl;

    if (NULL != bink->context) {
        avcodec_close(bink->context);
        av_free(bink->context);
    }
}

static int ce_decode_audio(AVCodecContext* avctx, int16_t* samples, int* frame_size_ptr, AVPacket* avpkt)
{
    AVFrame *frame = av_frame_alloc();
    int ret, got_frame = 0;

    if (!frame)
        return AVERROR(ENOMEM);
    if (avctx->get_buffer != avcodec_default_get_buffer) {
        av_log(avctx, AV_LOG_ERROR, "Custom get_buffer() for use with"
                                    "avcodec_decode_audio3() detected. Overriding with avcodec_default_get_buffer\n");
        av_log(avctx, AV_LOG_ERROR, "Please port your application to "
                                    "avcodec_decode_audio4()\n");
        avctx->get_buffer = avcodec_default_get_buffer;
        avctx->release_buffer = avcodec_default_release_buffer;
    }

    ret = avcodec_decode_audio4(avctx, frame, &got_frame, avpkt);

    if (ret >= 0 && got_frame) {
        int ch, plane_size;
        int planar    = av_sample_fmt_is_planar(avctx->sample_fmt);
        int data_size = av_samples_get_buffer_size(&plane_size, avctx->channels,
                                                   frame->nb_samples,
                                                   avctx->sample_fmt, 1);
        if (*frame_size_ptr < data_size) {
            av_log(avctx, AV_LOG_ERROR, "output buffer size is too small for "
                                        "the current frame (%d < %d)\n", *frame_size_ptr, data_size);
            av_frame_free(&frame);
            return AVERROR(EINVAL);
        }

        memcpy(samples, frame->extended_data[0], plane_size);

        if (planar && avctx->channels > 1) {
            uint8_t *out = ((uint8_t *)samples) + plane_size;
            for (ch = 1; ch < avctx->channels; ch++) {
                memcpy(out, frame->extended_data[ch], plane_size);
                out += plane_size;
            }
        }
        *frame_size_ptr = data_size;
    } else {
        *frame_size_ptr = 0;
    }
    av_frame_free(&frame);
    return ret;
}

static bool ce_bink_decode(ce_sound_resource* sound_resource)
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

        int size = MAX_AUDIO_FRAME_SIZE;
        int code = ce_decode_audio(bink->context,
            (int16_t*)sound_resource->output_buffer, &size, &bink->packet);

        if (code < 0 || (uint32_t)code != packet_size || size <= 0) {
            ce_logging_error("bink: codec error while decoding audio");
        } else {
            sound_resource->output_buffer_size = size;
        }
    }

    uint32_t frame_size = bink->indices[bink->frame_index++].length;
    frame_size -= packet_size + sizeof(packet_size);

    // skip video packet
    ce_mem_file_seek(sound_resource->mem_file, frame_size, CE_MEM_FILE_SEEK_CUR);

    return 0 == packet_size ? /* skip to next frame */
            ce_bink_decode(sound_resource) : true;
}

static bool ce_bink_reset(ce_sound_resource* sound_resource)
{
    ce_bink* bink = (ce_bink*)sound_resource->impl;

    bink->frame_index = 0;

    ce_mem_file_seek(sound_resource->mem_file,
        bink->indices[bink->frame_index].pos, CE_MEM_FILE_SEEK_SET);

    return true;
}

const ce_sound_resource_vtable ce_sound_resource_builtins[] = {
    {ce_vorbis_test,
    ce_vorbis_ctor, ce_vorbis_dtor,
    ce_vorbis_decode, ce_vorbis_reset},
    {ce_flac_test,
    ce_flac_ctor, ce_flac_dtor,
    ce_flac_decode, ce_flac_reset},
    {ce_wave_test,
    ce_wave_ctor, NULL,
    ce_wave_decode, ce_wave_reset},
#ifdef CE_ENABLE_PROPRIETARY
    {ce_mad_test,
    ce_mad_ctor, ce_mad_dtor,
    ce_mad_decode, ce_mad_reset},
#endif
    {ce_bink_test,
    ce_bink_ctor, ce_bink_dtor,
    ce_bink_decode, ce_bink_reset},
};

const size_t CE_SOUND_RESOURCE_BUILTIN_COUNT = sizeof(ce_sound_resource_builtins) /
                                            sizeof(ce_sound_resource_builtins[0]);
