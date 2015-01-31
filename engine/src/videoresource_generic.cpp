/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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
 *  Ogg Theora
 *  Bink (limited, for backward compatibility with original EI resources)
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <theora/theoradec.h>
#include <libavcodec/avcodec.h>

#include "str.hpp"
#include "alloc.hpp"
#include "logging.hpp"
#include "bink.hpp"
#include "videoresource.hpp"

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
    th_ycbcr_buffer ycbcr;
} ce_theora;

static size_t ce_theora_size_hint(ce_mem_file* CE_UNUSED(mem_file))
{
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

static bool ce_theora_pump(ogg_sync_state* sync, ce_mem_file* mem_file)
{
    const size_t size = 4096;
    char* buffer = ogg_sync_buffer(sync, size);
    size_t bytes = ce_mem_file_read(mem_file, buffer, 1, size);
    ogg_sync_wrote(sync, bytes);
    return 0 != bytes;
}

static bool ce_theora_test(ce_mem_file* mem_file)
{
    ce_theora theora;
    ce_theora_init(&theora);

    while (ce_theora_pump(&theora.sync, mem_file)) {
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

static void ce_theora_report_pixelformat(ce_video_resource* video_resource)
{
    ce_theora* theora = (ce_theora*)video_resource->impl;
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

static void ce_theora_report_colorspace(ce_video_resource* video_resource)
{
    ce_theora* theora = (ce_theora*)video_resource->impl;
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

static void ce_theora_report_comments(ce_video_resource* video_resource)
{
    ce_theora* theora = (ce_theora*)video_resource->impl;
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

static bool ce_theora_ctor(ce_video_resource* video_resource)
{
    ce_theora* theora = (ce_theora*)video_resource->impl;
    ce_theora_init(theora);

    bool done = false;
    int header_count = 0;
    int code;

    // parse the headers
    while (!done) {
        if (!ce_theora_pump(&theora->sync, video_resource->mem_file)) {
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
            if (!ce_theora_pump(&theora->sync, video_resource->mem_file)) {
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

    video_resource->width = theora->info.pic_width;
    video_resource->height = theora->info.pic_height;
    video_resource->fps = (float)theora->info.fps_numerator /
                                theora->info.fps_denominator;

    ce_logging_debug("theora: ogg logical stream %lx is theora %d bit/s, %ux%u %.02f fps",
        theora->stream.serialno, theora->info.target_bitrate,
        video_resource->width, video_resource->height, video_resource->fps);

    video_resource->ycbcr.crop_rect.x = theora->info.pic_x;
    video_resource->ycbcr.crop_rect.y = theora->info.pic_y;
    video_resource->ycbcr.crop_rect.width = theora->info.pic_width;
    video_resource->ycbcr.crop_rect.height = theora->info.pic_height;

    if (theora->info.pic_width != theora->info.frame_width ||
            theora->info.pic_height != theora->info.frame_height) {
        ce_logging_debug("theora: frame content is %dx%d with offset (%d,%d)",
            theora->info.frame_width, theora->info.frame_height,
            theora->info.pic_x, theora->info.pic_y);
    }

    ce_theora_report_pixelformat(video_resource);
    ce_theora_report_colorspace(video_resource);
    ce_theora_report_comments(video_resource);

    if (TH_PF_420 != theora->info.pixel_fmt &&
            TH_PF_444 != theora->info.pixel_fmt) {
        ce_logging_error("theora: pixel format not supported");
        return false;
    }

    return true;
}

static void ce_theora_dtor(ce_video_resource* video_resource)
{
    ce_theora* theora = (ce_theora*)video_resource->impl;
    ce_theora_clean(theora);
}

static bool ce_theora_read(ce_video_resource* video_resource)
{
    ce_theora* theora = (ce_theora*)video_resource->impl;

    while (ogg_stream_packetout(&theora->stream, &theora->packet) <= 0) {
        if (!ce_theora_pump(&theora->sync, video_resource->mem_file)) {
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

    ogg_int64_t granulepos;
    if (0 == th_decode_packetin(theora->context, &theora->packet, &granulepos)) {
        theora->time = th_granule_time(theora->context, granulepos);
        ++video_resource->frame_index;

        th_decode_ycbcr_out(theora->context, theora->ycbcr);

        for (size_t i = 0; i < 3; ++i) {
            video_resource->ycbcr.planes[i].stride = theora->ycbcr[i].stride;
            video_resource->ycbcr.planes[i].data = theora->ycbcr[i].data;
        }

        return true;
    }

    return false;
}

static bool ce_theora_reset(ce_video_resource* CE_UNUSED(video_resource))
{
    // TODO: implementation
    return false;
}

/*
 *  Bink Video (C) RAD Game Tools, Inc.
 *
 *  See also:
 *  1. http://wiki.multimedia.cx/index.php?title=Bink_Video
 *  2. FFmpeg (C) Michael Niedermayer
*/

typedef struct {
    ce_bink_header header;
    ce_bink_index* indices;
    AVCodec* codec;
    AVCodecContext* context;
    AVFrame* frame;
    AVPacket packet;
    uint8_t extradata[4 + FF_INPUT_BUFFER_PADDING_SIZE];
    uint8_t data[];
} ce_bink;

static size_t ce_bink_size_hint(ce_mem_file* mem_file)
{
    ce_bink_header header;
    return sizeof(ce_bink) + (!ce_bink_header_read(&header, mem_file) ? 0 :
        sizeof(ce_bink_index) * header.frame_count +
        header.largest_frame_size + FF_INPUT_BUFFER_PADDING_SIZE);
}

static bool ce_bink_test(ce_mem_file* mem_file)
{
    ce_bink_header header;
    return ce_bink_header_read(&header, mem_file);
}

static bool ce_bink_ctor(ce_video_resource* video_resource)
{
    ce_bink* bink = (ce_bink*)video_resource->impl;

    if (!ce_bink_header_read(&bink->header, video_resource->mem_file)) {
        ce_logging_error("bink: input does not appear to be a Bink video");
        return false;
    }

    // skip audio headers
    if (!ce_bink_audio_track_skip(bink->header.audio_track_count, video_resource->mem_file)) {
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

    video_resource->width = bink->header.video_width;
    video_resource->height = bink->header.video_height;
    video_resource->fps = (float)bink->header.fps_dividend / bink->header.fps_divider;

    ce_logging_debug("bink: video is %ux%u %.02f fps",
        video_resource->width, video_resource->height, video_resource->fps);

    video_resource->frame_count = bink->header.frame_count;

    video_resource->ycbcr.crop_rect.x = 0;
    video_resource->ycbcr.crop_rect.y = 0;
    video_resource->ycbcr.crop_rect.width = bink->header.video_width;
    video_resource->ycbcr.crop_rect.height = bink->header.video_height;

    bink->indices = (ce_bink_index*)bink->data;
    if (!ce_bink_index_read(bink->indices, bink->header.frame_count, video_resource->mem_file)) {
        ce_logging_error("bink: invalid frame index table");
        return false;
    }

    if (NULL == (bink->codec = avcodec_find_decoder(CODEC_ID_BINKVIDEO))) {
        ce_logging_error("bink: video codec not found");
        return false;
    }

    if (NULL == (bink->context = avcodec_alloc_context())) {
        ce_logging_error("bink: could not allocate context");
        return false;
    }

    bink->context->codec_tag = bink->header.four_cc;
    bink->context->width = bink->header.video_width;
    bink->context->height = bink->header.video_height;
    bink->context->extradata = bink->extradata;
    bink->context->extradata_size = 4;

    memcpy(bink->context->extradata, &bink->header.video_flags, 4);

    if (avcodec_open2(bink->context, bink->codec, NULL) < 0) {
        ce_logging_error("bink: could not open video codec");
        return false;
    }

    if (PIX_FMT_YUV420P != bink->context->pix_fmt) {
        ce_logging_error("bink: only YCbCr 4:2:0 supported");
        return false;
    }

    if (NULL == (bink->frame = avcodec_alloc_frame())) {
        ce_logging_error("bink: could not allocate context");
        return false;
    }

    av_init_packet(&bink->packet);
    bink->packet.data = bink->data + sizeof(ce_bink_index) * bink->header.frame_count;

    return true;
}

static void ce_bink_dtor(ce_video_resource* video_resource)
{
    ce_bink* bink = (ce_bink*)video_resource->impl;

    if (NULL != bink->frame) {
        av_free(bink->frame);
    }

    if (NULL != bink->context) {
        avcodec_close(bink->context);
        av_free(bink->context);
    }
}

static bool ce_bink_read(ce_video_resource* video_resource)
{
    ce_bink* bink = (ce_bink*)video_resource->impl;

    if (video_resource->frame_index == video_resource->frame_count) {
        assert(ce_mem_file_eof(video_resource->mem_file));
        return false;
    }

    uint32_t frame_size = bink->indices[video_resource->frame_index++].length;

    if (0 != bink->header.audio_track_count) {
        uint32_t packet_size;
        ce_mem_file_read(video_resource->mem_file, &packet_size, 4, 1);

        // skip audio packet
        ce_mem_file_seek(video_resource->mem_file, packet_size, CE_MEM_FILE_SEEK_CUR);

        frame_size -= packet_size + sizeof(packet_size);
    }

    // our input buffer is largest_frame_size bytes
    assert(frame_size <= bink->header.largest_frame_size);
    if (frame_size > bink->header.largest_frame_size) {
        ce_logging_error("bink: internal error while decoding video");
        return false;
    }

    bink->packet.size = ce_mem_file_read(video_resource->mem_file, bink->packet.data, 1, frame_size);

    int got_frame = 0;
    int code = avcodec_decode_video2(bink->context, bink->frame, &got_frame, &bink->packet);

    if (code < 0 || (uint32_t)code != frame_size || 0 == got_frame) {
        ce_logging_error("bink: codec error while decoding video");
        return false;
    }

    for (size_t i = 0; i < 3; ++i) {
        video_resource->ycbcr.planes[i].stride = bink->frame->linesize[i];
        video_resource->ycbcr.planes[i].data = bink->frame->data[i];
    }

    return true;
}

static bool ce_bink_reset(ce_video_resource* video_resource)
{
    ce_bink* bink = (ce_bink*)video_resource->impl;

    ce_mem_file_seek(video_resource->mem_file, bink->indices[video_resource->frame_index].pos, CE_MEM_FILE_SEEK_SET);

    return true;
}

const ce_video_resource_vtable ce_video_resource_builtins[] = {
    {ce_theora_size_hint, ce_theora_test, ce_theora_ctor, ce_theora_dtor, ce_theora_read, ce_theora_reset},
    {ce_bink_size_hint, ce_bink_test, ce_bink_ctor, ce_bink_dtor, ce_bink_read, ce_bink_reset},
};

const size_t CE_VIDEO_RESOURCE_BUILTIN_COUNT = sizeof(ce_video_resource_builtins) / sizeof(ce_video_resource_builtins[0]);
