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

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include <theora/theoradec.h>

#include "celib.h"
#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
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

static void ce_theora_report_pixelformat(ce_videoresource* videoresource)
{
	ce_theora* theora = (ce_theora*)videoresource->impl;
	switch (theora->info.pixel_fmt) {
	case TH_PF_420:
		ce_logging_debug("theora: pixel format is 4:2:0");
		break;
	case TH_PF_422:
		ce_logging_debug("theora: pixel format is 4:2:2");
		break;
	case TH_PF_444:
		ce_logging_debug("theora: pixel format is 4:4:4");
		break;
	case TH_PF_RSVD:
	default:
		ce_logging_error("theora: unknown chroma sampling!");
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

static bool ce_theora_pump(ce_videoresource* videoresource)
{
	ce_theora* theora = (ce_theora*)videoresource->impl;
	const size_t size = 4096;
	char* buffer = ogg_sync_buffer(&theora->sync, size);
	size_t bytes = ce_memfile_read(videoresource->memfile, buffer, 1, size);
	ogg_sync_wrote(&theora->sync, bytes);
	return 0 != bytes;
}

bool ce_theora_test(ce_memfile* memfile)
{
	ce_unused(memfile);
	return true;
}

bool ce_theora_ctor(ce_videoresource* videoresource)
{
	ce_theora* theora = (ce_theora*)videoresource->impl;

	// start up ogg stream synchronization layer
	ogg_sync_init(&theora->sync);

	// init supporting theora structures needed in header parsing
	th_info_init(&theora->info);
	th_comment_init(&theora->comment);

	bool done = false;
	int header_count = 0;
	int code;

	// parse the headers
	while (!done) {
		if (!ce_theora_pump(videoresource)) {
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
			if (!ce_theora_pump(videoresource)) {
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

void ce_theora_dtor(ce_videoresource* videoresource)
{
	ce_theora* theora = (ce_theora*)videoresource->impl;

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

bool ce_theora_read(ce_videoresource* videoresource, void* data)
{
	ce_theora* theora = (ce_theora*)videoresource->impl;
	ogg_int64_t granulepos;

	while (ogg_stream_packetout(&theora->stream, &theora->packet) <= 0) {
		if (!ce_theora_pump(videoresource)) {
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

bool ce_theora_reset(ce_videoresource* videoresource)
{
	ce_unused(videoresource);
	return false;
}

const ce_videoresource_vtable ce_videoresource_builtins[] = {
	{sizeof(ce_theora), ce_theora_test, ce_theora_ctor,
	ce_theora_dtor, ce_theora_read, ce_theora_reset},
};

const size_t CE_VIDEORESOURCE_BUILTIN_COUNT = sizeof(ce_videoresource_builtins) /
											sizeof(ce_videoresource_builtins[0]);
