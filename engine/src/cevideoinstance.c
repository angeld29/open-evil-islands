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
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "celib.h"
#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
#include "cevideoinstance.h"

static void ce_videoinstance_exec(ce_videoinstance* videoinstance)
{
	ce_mutex_lock(videoinstance->mutex);

	while (!videoinstance->done) {
		//ce_mutex_unlock(videoinstance->mutex);
		//ce_mutex_lock(videoinstance->mutex);
		ce_waitcond_wait(videoinstance->waitcond, videoinstance->mutex);
	}

	ce_mutex_unlock(videoinstance->mutex);
}

static void ce_videoinstance_report_pixelformat(th_info* info)
{
	switch (info->pixel_fmt) {
	case TH_PF_420:
		ce_logging_debug("videoinstance: pixel format is 4:2:0");
		break;
	case TH_PF_422:
		ce_logging_debug("videoinstance: pixel format is 4:2:2");
		break;
	case TH_PF_444:
		ce_logging_debug("videoinstance: pixel format is 4:4:4");
		break;
	case TH_PF_RSVD:
	default:
		ce_logging_error("videoinstance: unknown chroma sampling!");
		break;
	}
}

static void ce_videoinstance_report_colorspace(th_info* info)
{
	switch (info->colorspace) {
	case TH_CS_UNSPECIFIED:
		// nothing to report
		break;
	case TH_CS_ITU_REC_470M:
		ce_logging_debug("videoinstance: encoder specified ITU Rec 470M (NTSC) color");
		break;
	case TH_CS_ITU_REC_470BG:
		ce_logging_debug("videoinstance: encoder specified ITU Rec 470BG (PAL) color");
		break;
	default:
		ce_logging_warning("videoinstance: encoder specified unknown colorspace (%d)", info->colorspace);
		break;
	}
}

static void ce_videoinstance_report_comments(th_comment* comment)
{
	ce_logging_debug("videoinstance: encoded by %s", comment->vendor);
	if (0 != comment->comments) {
		ce_logging_debug("videoinstance: theora comment header:");
		for (int i = 0; i < comment->comments; ++i) {
			if (comment->user_comments[i]) {
				char buffer[comment->comment_lengths[i] + 1];
				ce_strlcpy(buffer, comment->user_comments[i], sizeof(buffer));
				ce_logging_debug("videoinstance: \t%s", buffer);
			}
		}
	}
}

static bool ce_videoinstance_pump(ce_videoinstance* videoinstance)
{
	const size_t size = 4096;
	char* buffer = ogg_sync_buffer(&videoinstance->sync, size);
	size_t bytes = ce_memfile_read(videoinstance->memfile, buffer, 1, size);
	ogg_sync_wrote(&videoinstance->sync, bytes);
	return 0 != bytes;
}

ce_videoinstance* ce_videoinstance_new(ce_memfile* memfile)
{
	ce_videoinstance* videoinstance = ce_alloc_zero(sizeof(ce_videoinstance));
	videoinstance->memfile = memfile;
	videoinstance->mutex = ce_mutex_new();
	videoinstance->waitcond = ce_waitcond_new();
	videoinstance->thread = ce_thread_new(ce_videoinstance_exec, videoinstance);
	videoinstance->prev_frame = -1.0f;

	ce_anmframe_reset(&videoinstance->anmframe);

	// start up ogg stream synchronization layer
	ogg_sync_init(&videoinstance->sync);

	// init supporting theora structures needed in header parsing
	th_info_init(&videoinstance->info);
	th_comment_init(&videoinstance->comment);

	bool done = false;
	int header_packet_count = 0;
	int code;

	// ogg file open; parse the headers
	while (!done) {
		if (!ce_videoinstance_pump(videoinstance)) {
			ce_logging_error("videoinstance: theora: end of file while searching for headers");
			ce_videoinstance_del(videoinstance);
			return NULL;
		}

		while (ogg_sync_pageout(&videoinstance->sync, &videoinstance->page) > 0) {
			// is this a mandated initial header?
			if (0 == ogg_page_bos(&videoinstance->page)) {
				if (0 != header_packet_count) {
					// don't leak the page; get it into the appropriate stream
					code = ogg_stream_pagein(&videoinstance->stream,
												&videoinstance->page);
				}
				done = true;
				break;
			}

			ogg_stream_state test;

			code = ogg_stream_init(&test, ogg_page_serialno(&videoinstance->page));
			code = ogg_stream_pagein(&test, &videoinstance->page);
			code = ogg_stream_packetout(&test, &videoinstance->packet);

			// identify the codec
			if(0 == header_packet_count &&
					th_decode_headerin(&videoinstance->info,
										&videoinstance->comment,
										&videoinstance->setup,
										&videoinstance->packet) >= 0) {
				// it is theora
				memcpy(&videoinstance->stream, &test, sizeof(test));
				header_packet_count = 1;
			} else {
				// don't care about it
				ogg_stream_clear(&test);
			}
		}
	}

	// we're expecting more header packets
	while (0 != header_packet_count && header_packet_count < 3) {
		// look for further theora headers
		code = ogg_stream_packetout(&videoinstance->stream, &videoinstance->packet);

		if (code < 0) {
			ce_logging_error("videoinstance: theora: error parsing "
							"stream headers; corrupt stream?");
			ce_videoinstance_del(videoinstance);
			return NULL;
		}

		if (code > 0) {
			code = th_decode_headerin(&videoinstance->info,
										&videoinstance->comment,
										&videoinstance->setup,
										&videoinstance->packet);
			if (code <= 0) {
				ce_logging_error("videoinstance: theora: error parsing "
								"stream headers; corrupt stream?");
				ce_videoinstance_del(videoinstance);
				return NULL;
			}

			++header_packet_count;
			continue;
		}

		// the header pages/packets will arrive before anything else we
		// care about, or the stream is not obeying spec
		if (ogg_sync_pageout(&videoinstance->sync, &videoinstance->page) > 0) {
			// demux into the appropriate stream
			code = ogg_stream_pagein(&videoinstance->stream, &videoinstance->page);
		} else {
			// someone needs more data
			if (!ce_videoinstance_pump(videoinstance)) {
				ce_logging_error("videoinstance: theora: end of file while searching for headers");
				ce_videoinstance_del(videoinstance);
				return NULL;
			}
		}
	}

	if (0 == header_packet_count) {
		ce_logging_error("videoinstance: theora: could not find headers");
		ce_videoinstance_del(videoinstance);
		return NULL;
	}

	// initialize decoders
	videoinstance->context = th_decode_alloc(&videoinstance->info, videoinstance->setup);

	videoinstance->width = videoinstance->info.pic_width;
	videoinstance->height = videoinstance->info.pic_height;
	videoinstance->fps = (float)videoinstance->info.fps_numerator /
								videoinstance->info.fps_denominator;

	videoinstance->mmpfile = ce_mmpfile_new(videoinstance->width,
		videoinstance->height, 1, CE_MMPFILE_FORMAT_R8G8B8A8, 0);

	ce_logging_debug("videoinstance: theora: ogg logical "
						"stream %lx is theora %dx%d %.02f fps",
					videoinstance->stream.serialno, videoinstance->width,
					videoinstance->height, videoinstance->fps);

	if (videoinstance->info.pic_width != videoinstance->info.frame_width ||
			videoinstance->info.pic_height != videoinstance->info.frame_height) {
		ce_logging_debug("videoinstance: theora: frame content is %dx%d with offset (%d,%d)",
			videoinstance->info.frame_width, videoinstance->info.frame_height,
			videoinstance->info.pic_x, videoinstance->info.pic_y);
	}

	ce_videoinstance_report_pixelformat(&videoinstance->info);
	ce_videoinstance_report_colorspace(&videoinstance->info);
	ce_videoinstance_report_comments(&videoinstance->comment);

	if (TH_PF_420 != videoinstance->info.pixel_fmt &&
			TH_PF_444 != videoinstance->info.pixel_fmt) {
		ce_logging_error("videoinstance: theora: pixel format not supported");
		ce_videoinstance_del(videoinstance);
		return NULL;
	}

	return videoinstance;
}

void ce_videoinstance_del(ce_videoinstance* videoinstance)
{
	if (NULL != videoinstance) {
		ce_mutex_lock(videoinstance->mutex);
		videoinstance->done = true;
		ce_mutex_unlock(videoinstance->mutex);

		ce_waitcond_wake_all(videoinstance->waitcond);
		ce_thread_wait(videoinstance->thread);

		if (NULL != videoinstance->context) {
			th_decode_free(videoinstance->context);
		}

		th_comment_clear(&videoinstance->comment);
		th_info_clear(&videoinstance->info);

		if (NULL != videoinstance->setup) {
			th_setup_free(videoinstance->setup);
		}

		ogg_stream_clear(&videoinstance->stream);
		ogg_sync_clear(&videoinstance->sync);

		ce_thread_del(videoinstance->thread);
		ce_waitcond_del(videoinstance->waitcond);
		ce_mutex_del(videoinstance->mutex);

		ce_texture_del(videoinstance->texture);
		ce_mmpfile_del(videoinstance->mmpfile);
		ce_memfile_close(videoinstance->memfile);

		ce_free(videoinstance, sizeof(ce_videoinstance));
	}
}

static void ce_videoinstance_decode(ce_videoinstance* videoinstance)
{
	ogg_int64_t granulepos;

	while (ogg_stream_packetout(&videoinstance->stream, &videoinstance->packet) <= 0) {
		if (!ce_videoinstance_pump(videoinstance)) {
			return;
		}

		while (ogg_sync_pageout(&videoinstance->sync, &videoinstance->page) > 0) {
			ogg_stream_pagein(&videoinstance->stream, &videoinstance->page);
		}
	}

	if (videoinstance->packet.granulepos >= 0) {
		th_decode_ctl(videoinstance->context,
						TH_DECCTL_SET_GRANPOS,
						&videoinstance->packet.granulepos,
						sizeof(videoinstance->packet.granulepos));
	}

	if (0 == th_decode_packetin(videoinstance->context,
								&videoinstance->packet,
								&granulepos)) {
		videoinstance->time = th_granule_time(videoinstance->context, granulepos);
		++videoinstance->frame_count;

		th_ycbcr_buffer ycbcr;
		th_decode_ycbcr_out(videoinstance->context, ycbcr);

		// TODO: move conversion details to mmpfile
		unsigned char* texels = videoinstance->mmpfile->texels;

		ogg_uint32_t y_offset = (videoinstance->info.pic_x & ~1) +
								ycbcr[0].stride * (videoinstance->info.pic_y & ~1);

		ogg_uint32_t cbcr_offset = (videoinstance->info.pic_x / 2) +
									ycbcr[1].stride * (videoinstance->info.pic_y / 2);

		for (ogg_uint32_t h = 0; h < videoinstance->info.pic_height; ++h) {
			ogg_uint32_t y_shift = y_offset + ycbcr[0].stride * h;
			ogg_uint32_t cb_shift = cbcr_offset + ycbcr[1].stride * (h / 2);
			ogg_uint32_t cr_shift = cbcr_offset + ycbcr[2].stride * (h / 2);

			for (ogg_uint32_t w = 0; w < videoinstance->info.pic_width; ++w) {
				size_t index = (h * videoinstance->info.pic_width + w) * 4;

				int y = 298 * (ycbcr[0].data[y_shift + w] - 16);
				int cb = ycbcr[1].data[cb_shift + w / 2] - 128;
				int cr = ycbcr[2].data[cr_shift + w / 2] - 128;

				texels[index + 0] = ce_clamp((y + 409 * cr + 128) / 256, 0, UCHAR_MAX);
				texels[index + 1] = ce_clamp((y - 100 * cb - 208 * cr + 128) / 256, 0, UCHAR_MAX);
				texels[index + 2] = ce_clamp((y + 516 * cb + 128) / 256, 0, UCHAR_MAX);
				texels[index + 3] = UCHAR_MAX;
			}
		}

		ce_texture_del(videoinstance->texture);
		videoinstance->texture = ce_texture_new("frame", videoinstance->mmpfile);
	}
}

void ce_videoinstance_advance(ce_videoinstance* videoinstance, float elapsed)
{
	ce_anmframe_advance(&videoinstance->anmframe, videoinstance->fps * elapsed);
	if (videoinstance->prev_frame != videoinstance->anmframe.prev_frame) {
		ce_videoinstance_decode(videoinstance);
		videoinstance->prev_frame = videoinstance->anmframe.prev_frame;
	}
}
