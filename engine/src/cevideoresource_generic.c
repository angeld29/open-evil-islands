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

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

#include <theora/theoradec.h>

#ifdef CE_ENABLE_PROPRIETARY
#include <libavcodec/avcodec.h>
#endif

#include "celib.h"
#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
#include "cebink.h"
#include "cevideoresource.h"

/*
 *  Ogg Theora (C) 2002-2007 Xiph.Org Foundation
 *
 *  See also:
 *  1. http://www.xiph.org/ogg/doc/
 *  2. http://theora.org/doc/
*/

typedef struct {
	float time; // absolute time in seconds
	ogg_sync_state sync;
	ogg_stream_state stream;
	ogg_page page;
	ogg_packet packet;
	th_setup_info* setup;
	th_info info;
	th_comment comment;
	th_dec_ctx* context;
} ce_theora;

static size_t ce_theora_size_hint(ce_memfile* memfile)
{
	ce_unused(memfile);
	return sizeof(ce_theora);
}

static void ce_theora_init(ce_theora* theora)
{
	memset(theora, 0, sizeof(ce_theora));

	// start up ogg stream synchronization layer
	ogg_sync_init(&theora->sync);

	// init supporting theora structures needed in header parsing
	th_info_init(&theora->info);
	th_comment_init(&theora->comment);
}

static void ce_theora_clean(ce_theora* theora)
{
	if (NULL != theora->context) {
		th_decode_free(theora->context);
	}

	th_comment_clear(&theora->comment);
	th_info_clear(&theora->info);

	if (NULL != theora->setup) {
		th_setup_free(theora->setup);
	}

	ogg_stream_clear(&theora->stream);
	ogg_sync_clear(&theora->sync);
}

static bool ce_theora_pump(ogg_sync_state* sync, ce_memfile* memfile)
{
	const size_t size = 4096;
	char* buffer = ogg_sync_buffer(sync, size);
	size_t bytes = ce_memfile_read(memfile, buffer, 1, size);
	ogg_sync_wrote(sync, bytes);
	return 0 != bytes;
}

static bool ce_theora_test(ce_memfile* memfile)
{
	ce_theora theora;
	ce_theora_init(&theora);

	while (ce_theora_pump(&theora.sync, memfile)) {
		while (ogg_sync_pageout(&theora.sync, &theora.page) > 0) {
			if (0 == ogg_page_bos(&theora.page)) {
				ce_theora_clean(&theora);
				return false;
			}

			ogg_stream_init(&theora.stream, ogg_page_serialno(&theora.page));
			ogg_stream_pagein(&theora.stream, &theora.page);
			ogg_stream_packetout(&theora.stream, &theora.packet);

			if (th_decode_headerin(&theora.info, &theora.comment,
									&theora.setup, &theora.packet) >= 0) {
				ce_theora_clean(&theora);
				return true;
			}

			ogg_stream_clear(&theora.stream);
		}
	}

	ce_theora_clean(&theora);
	return false;
}

static void ce_theora_report_pixelformat(ce_videoresource* videoresource)
{
	ce_theora* theora = (ce_theora*)videoresource->impl;
	switch (theora->info.pixel_fmt) {
	case TH_PF_420:
		ce_logging_debug("theora: pixel format is YCbCr 4:2:0");
		break;
	case TH_PF_422:
		ce_logging_debug("theora: pixel format is YCbCr 4:2:2");
		break;
	case TH_PF_444:
		ce_logging_debug("theora: pixel format is YCbCr 4:4:4");
		break;
	case TH_PF_RSVD:
	default:
		ce_logging_error("theora: unknown chroma sampling");
		break;
	}
}

static void ce_theora_report_colorspace(ce_videoresource* videoresource)
{
	ce_theora* theora = (ce_theora*)videoresource->impl;
	switch (theora->info.colorspace) {
	case TH_CS_UNSPECIFIED:
		// nothing to report
		break;
	case TH_CS_ITU_REC_470M:
		ce_logging_debug("theora: encoder specified ITU Rec 470M (NTSC) color");
		break;
	case TH_CS_ITU_REC_470BG:
		ce_logging_debug("theora: encoder specified ITU Rec 470BG (PAL) color");
		break;
	default:
		ce_logging_warning("theora: encoder specified unknown colorspace (%d)", theora->info.colorspace);
		break;
	}
}

static void ce_theora_report_comments(ce_videoresource* videoresource)
{
	ce_theora* theora = (ce_theora*)videoresource->impl;
	ce_logging_debug("theora: encoded by %s", theora->comment.vendor);
	if (0 != theora->comment.comments) {
		ce_logging_debug("theora: theora comment header:");
		for (int i = 0; i < theora->comment.comments; ++i) {
			if (theora->comment.user_comments[i]) {
				char buffer[theora->comment.comment_lengths[i] + 1];
				ce_strlcpy(buffer, theora->comment.user_comments[i], sizeof(buffer));
				ce_logging_debug("theora: %s", buffer);
			}
		}
	}
}

static bool ce_theora_ctor(ce_videoresource* videoresource)
{
	ce_theora* theora = (ce_theora*)videoresource->impl;
	ce_theora_init(theora);

	bool done = false;
	int header_count = 0;
	int code;

	// parse the headers
	while (!done) {
		if (!ce_theora_pump(&theora->sync, videoresource->memfile)) {
			ce_logging_error("theora: end of stream while searching for headers");
			return false;
		}

		while (ogg_sync_pageout(&theora->sync, &theora->page) > 0) {
			// is this a mandated initial header?
			if (0 == ogg_page_bos(&theora->page)) {
				if (0 != header_count) {
					// don't leak the page; get it into the appropriate stream
					code = ogg_stream_pagein(&theora->stream, &theora->page);
				}
				done = true;
				break;
			}

			ogg_stream_state test;

			code = ogg_stream_init(&test, ogg_page_serialno(&theora->page));
			code = ogg_stream_pagein(&test, &theora->page);
			code = ogg_stream_packetout(&test, &theora->packet);

			// identify the codec
			if(0 == header_count && th_decode_headerin(&theora->info,
														&theora->comment,
														&theora->setup,
														&theora->packet) >= 0) {
				// it is theora
				memcpy(&theora->stream, &test, sizeof(test));
				header_count = 1;
			} else {
				// don't care about it
				ogg_stream_clear(&test);
			}
		}
	}

	// we're expecting more header packets
	while (0 != header_count && header_count < 3) {
		// look for further theora headers
		code = ogg_stream_packetout(&theora->stream, &theora->packet);

		if (code < 0) {
			ce_logging_error("theora: error parsing stream headers");
			return false;
		}

		if (code > 0) {
			code = th_decode_headerin(&theora->info,
										&theora->comment,
										&theora->setup,
										&theora->packet);
			if (code <= 0) {
				ce_logging_error("theora: error parsing stream headers");
				return false;
			}

			++header_count;
			continue;
		}

		// the header pages/packets will arrive before anything else we
		// care about, or the stream is not obeying spec
		if (ogg_sync_pageout(&theora->sync, &theora->page) > 0) {
			// demux into the appropriate stream
			code = ogg_stream_pagein(&theora->stream, &theora->page);
		} else {
			// someone needs more data
			if (!ce_theora_pump(&theora->sync, videoresource->memfile)) {
				ce_logging_error("theora: end of stream while searching for headers");
				return false;
			}
		}
	}

	if (0 == header_count) {
		ce_logging_error("theora: could not find headers");
		return false;
	}

	// initialize decoder
	theora->context = th_decode_alloc(&theora->info, theora->setup);

	videoresource->width = theora->info.pic_width;
	videoresource->height = theora->info.pic_height;
	videoresource->fps = (float)theora->info.fps_numerator /
								theora->info.fps_denominator;

	ce_logging_debug("theora: ogg logical stream %lx is theora %ux%u %.02f fps",
		theora->stream.serialno, videoresource->width,
		videoresource->height, videoresource->fps);

	if (theora->info.pic_width != theora->info.frame_width ||
			theora->info.pic_height != theora->info.frame_height) {
		ce_logging_debug("theora: frame content is %dx%d with offset (%d,%d)",
			theora->info.frame_width, theora->info.frame_height,
			theora->info.pic_x, theora->info.pic_y);
	}

	ce_theora_report_pixelformat(videoresource);
	ce_theora_report_colorspace(videoresource);
	ce_theora_report_comments(videoresource);

	if (TH_PF_420 != theora->info.pixel_fmt &&
			TH_PF_444 != theora->info.pixel_fmt) {
		ce_logging_error("theora: pixel format not supported");
		return false;
	}

	return true;
}

static void ce_theora_dtor(ce_videoresource* videoresource)
{
	ce_theora* theora = (ce_theora*)videoresource->impl;
	ce_theora_clean(theora);
}

static bool ce_theora_read(ce_videoresource* videoresource, void* data)
{
	ce_theora* theora = (ce_theora*)videoresource->impl;
	ogg_int64_t granulepos;

	while (ogg_stream_packetout(&theora->stream, &theora->packet) <= 0) {
		if (!ce_theora_pump(&theora->sync, videoresource->memfile)) {
			return false;
		}

		while (ogg_sync_pageout(&theora->sync, &theora->page) > 0) {
			ogg_stream_pagein(&theora->stream, &theora->page);
		}
	}

	// TODO: explore it
	if (theora->packet.granulepos >= 0) {
		th_decode_ctl(theora->context, TH_DECCTL_SET_GRANPOS,
						&theora->packet.granulepos,
						sizeof(theora->packet.granulepos));
	}

	if (0 == th_decode_packetin(theora->context, &theora->packet, &granulepos)) {
		theora->time = th_granule_time(theora->context, granulepos);

		th_ycbcr_buffer ycbcr;
		th_decode_ycbcr_out(theora->context, ycbcr);

		// TODO: move conversion details to mmpfile
		unsigned char* texels = data;

		ogg_uint32_t y_offset = (theora->info.pic_x & ~1) +
								ycbcr[0].stride * (theora->info.pic_y & ~1);

		ogg_uint32_t cbcr_offset = (theora->info.pic_x / 2) +
									ycbcr[1].stride * (theora->info.pic_y / 2);

		for (ogg_uint32_t h = 0; h < theora->info.pic_height; ++h) {
			ogg_uint32_t y_shift = y_offset + ycbcr[0].stride * h;
			ogg_uint32_t cb_shift = cbcr_offset + ycbcr[1].stride * (h / 2);
			ogg_uint32_t cr_shift = cbcr_offset + ycbcr[2].stride * (h / 2);

			for (ogg_uint32_t w = 0; w < theora->info.pic_width; ++w) {
				size_t index = (h * theora->info.pic_width + w) * 4;

				int y = 298 * (ycbcr[0].data[y_shift + w] - 16);
				int cb = ycbcr[1].data[cb_shift + w / 2] - 128;
				int cr = ycbcr[2].data[cr_shift + w / 2] - 128;

				texels[index + 0] = ce_clamp((y + 409 * cr + 128) / 256, 0, UCHAR_MAX);
				texels[index + 1] = ce_clamp((y - 100 * cb - 208 * cr + 128) / 256, 0, UCHAR_MAX);
				texels[index + 2] = ce_clamp((y + 516 * cb + 128) / 256, 0, UCHAR_MAX);
				texels[index + 3] = UCHAR_MAX;
			}
		}

		return true;
	}

	return false;
}

static bool ce_theora_reset(ce_videoresource* videoresource)
{
	// TODO: not implemented
	ce_unused(videoresource);
	return false;
}

#ifdef CE_ENABLE_PROPRIETARY
/*
 *  Bink Video (C) RAD Game Tools, Inc.
 *
 *  See also:
 *  1. http://wiki.multimedia.cx/index.php?title=Bink_Video
 *  2. FFmpeg (C) Michael Niedermayer
*/

typedef struct {
	size_t frame_index;
	ce_binkheader header;
	ce_binkindex* indices;
	AVCodec* codec;
	AVCodecContext codec_context;
	AVPacket packet;
	AVFrame picture;
	uint8_t extradata[4 + FF_INPUT_BUFFER_PADDING_SIZE];
	uint8_t data[];
} ce_bink;

static size_t ce_bink_size_hint(ce_memfile* memfile)
{
	ce_binkheader header;
	return sizeof(ce_bink) + (!ce_binkheader_read(&header, memfile) ? 0 :
		sizeof(ce_binkindex) * header.frame_count +
		header.largest_frame_size + FF_INPUT_BUFFER_PADDING_SIZE);
}

static bool ce_bink_test(ce_memfile* memfile)
{
	ce_binkheader header;
	return ce_binkheader_read(&header, memfile);
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

static bool ce_bink_ctor(ce_videoresource* videoresource)
{
	ce_bink* bink = (ce_bink*)videoresource->impl;

	if (!ce_binkheader_read(&bink->header, videoresource->memfile)) {
		ce_logging_error("bink: input does not appear to be a Bink video");
		return false;
	}

	// skip audio headers
	if (!ce_binktrack_skip(bink->header.audio_track_count, videoresource->memfile)) {
		ce_logging_error("bink: input does not appear to be a Bink video");
		return false;
	}

	if (0 == bink->header.frame_count ||
			0 == bink->header.video_width || 0 == bink->header.video_height ||
			0 == bink->header.fps_dividend || 0 == bink->header.fps_divider ||
			bink->header.largest_frame_size > bink->header.file_size ||
			bink->header.frame_count > CE_BINK_MAX_FRAMES ||
			bink->header.video_width > CE_BINK_MAX_VIDEO_WIDTH ||
			bink->header.video_height > CE_BINK_MAX_VIDEO_HEIGHT) {
		ce_logging_error("bink: unsupported video");
		return false;
	}

	if (CE_BINK_VIDEO_FLAG_HAS_ALPHA_PLANE & bink->header.video_flags) {
		ce_logging_warning("bink: alpha plane not supported");
	}

	videoresource->width = bink->header.video_width;
	videoresource->height = bink->header.video_height;
	videoresource->fps = (float)bink->header.fps_dividend /
								bink->header.fps_divider;

	ce_logging_debug("bink: video is %ux%u %.02f fps",
		videoresource->width, videoresource->height, videoresource->fps);

	bink->indices = (ce_binkindex*)bink->data;
	if (!ce_binkindex_read(bink->indices, bink->header.frame_count, videoresource->memfile)) {
		ce_logging_error("bink: invalid frame index table");
		return false;
	}

	av_log_set_callback(ce_bink_error);

	avcodec_init();
	avcodec_register_all();

	bink->codec = avcodec_find_decoder(CODEC_ID_BINKVIDEO);
	if (NULL == bink->codec) {
		ce_logging_error("bink: video codec not found");
		return false;
	}

	avcodec_get_context_defaults(&bink->codec_context);
	bink->codec_context.codec_tag = bink->header.four_cc;
	bink->codec_context.width  = bink->header.video_width;
	bink->codec_context.height = bink->header.video_height;
	bink->codec_context.extradata = bink->extradata;
	bink->codec_context.extradata_size = 4;

	memcpy(bink->codec_context.extradata, &bink->header.video_flags, 4);

	if (avcodec_open(&bink->codec_context, bink->codec) < 0) {
		ce_logging_error("bink: could not open video codec");
		return false;
	}

	if (PIX_FMT_YUV420P != bink->codec_context.pix_fmt) {
		ce_logging_error("bink: only YCbCr 4:2:0 supported");
		return false;
	}

	av_init_packet(&bink->packet);
	bink->packet.data = bink->data + sizeof(ce_binkindex) * bink->header.frame_count;

	avcodec_get_frame_defaults(&bink->picture);

	return true;
}

static void ce_bink_dtor(ce_videoresource* videoresource)
{
	ce_bink* bink = (ce_bink*)videoresource->impl;

	if (NULL != bink->codec && NULL != bink->codec_context.codec) {
		avcodec_close(&bink->codec_context);
	}
}

static bool ce_bink_read(ce_videoresource* videoresource, void* data)
{
	ce_bink* bink = (ce_bink*)videoresource->impl;

	if (bink->frame_index == bink->header.frame_count) {
		assert(ce_memfile_eof(videoresource->memfile));
		return false;
	}

	uint32_t frame_size = bink->indices[bink->frame_index++].length;

	if (0 != bink->header.audio_track_count) {
		uint32_t packet_size;
		ce_memfile_read(videoresource->memfile, &packet_size, 4, 1);

		// skip audio packet
		ce_memfile_seek(videoresource->memfile, packet_size, CE_MEMFILE_SEEK_CUR);

		frame_size -= packet_size + sizeof(packet_size);
	}

	// our input buffer is largest_frame_size bytes
	assert(frame_size <= bink->header.largest_frame_size);
	if (frame_size > bink->header.largest_frame_size) {
		ce_logging_error("bink: internal error while decoding video");
		return false;
	}

	bink->packet.size = ce_memfile_read(videoresource->memfile,
										bink->packet.data, 1, frame_size);

	int got_picture = 0;
	int code = avcodec_decode_video2(&bink->codec_context,
		&bink->picture, &got_picture, &bink->packet);

	if (code < 0 || (uint32_t)code != frame_size || 0 == got_picture) {
		ce_logging_error("bink: codec error while decoding video");
	} else {
		// TODO: move conversion details to mmpfile
		unsigned char* texels = data;

		for (uint32_t h = 0; h < bink->header.video_height; ++h) {
			int y_shift = bink->picture.linesize[0] * h;
			int cb_shift = bink->picture.linesize[1] * (h / 2);
			int cr_shift = bink->picture.linesize[2] * (h / 2);

			for (uint32_t w = 0; w < bink->header.video_width; ++w) {
				size_t index = (h * bink->header.video_width + w) * 4;

				int y = 298 * (bink->picture.data[0][y_shift + w] - 16);
				int cb = bink->picture.data[1][cb_shift + w / 2] - 128;
				int cr = bink->picture.data[2][cr_shift + w / 2] - 128;

				texels[index + 0] = ce_clamp((y + 409 * cr + 128) / 256, 0, UCHAR_MAX);
				texels[index + 1] = ce_clamp((y - 100 * cb - 208 * cr + 128) / 256, 0, UCHAR_MAX);
				texels[index + 2] = ce_clamp((y + 516 * cb + 128) / 256, 0, UCHAR_MAX);
				texels[index + 3] = UCHAR_MAX;
			}
		}
	}

	return true;
}

static bool ce_bink_reset(ce_videoresource* videoresource)
{
	ce_bink* bink = (ce_bink*)videoresource->impl;

	bink->frame_index = 0;

	ce_memfile_seek(videoresource->memfile,
		bink->indices[bink->frame_index].pos, CE_MEMFILE_SEEK_SET);

	return true;
}
#endif /* CE_ENABLE_PROPRIETARY */

const ce_videoresource_vtable ce_videoresource_builtins[] = {
	{ce_theora_size_hint, ce_theora_test, ce_theora_ctor,
	ce_theora_dtor, ce_theora_read, ce_theora_reset},
#ifdef CE_ENABLE_PROPRIETARY
	{ce_bink_size_hint, ce_bink_test, ce_bink_ctor,
	ce_bink_dtor, ce_bink_read, ce_bink_reset},
#endif
};

const size_t CE_VIDEORESOURCE_BUILTIN_COUNT = sizeof(ce_videoresource_builtins) /
											sizeof(ce_videoresource_builtins[0]);
