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
#include <assert.h>

#include "celib.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceroot.h"
#include "cesoundhelper.h"
#include "cevideoinstance.h"

static void ce_videoinstance_exec(ce_videoinstance* videoinstance)
{
	for (size_t i = ce_semaphore_available(videoinstance->prepared_frames); ; ++i) {
		ce_semaphore_acquire(videoinstance->unprepared_frames, 1);

		if (videoinstance->done) {
			break;
		}

		if (!ce_videoresource_read(videoinstance->videoresource)) {
			videoinstance->state = CE_VIDEOINSTANCE_STATE_STOPPED;
			break;
		}

		ce_mmpfile* mmpfile = videoinstance->frames[i % CE_VIDEOINSTANCE_CACHE_SIZE];
		ce_ycbcr* ycbcr = &videoinstance->videoresource->ycbcr;

		unsigned char* y_data = mmpfile->texels;
		unsigned char* cb_data = y_data + mmpfile->width * mmpfile->height;
		unsigned char* cr_data = cb_data + (mmpfile->width / 2) * (mmpfile->height / 2);

		int y_offset = (ycbcr->crop_rect.x & ~1) + ycbcr->planes[0].stride * (ycbcr->crop_rect.y & ~1);
		int cb_offset = (ycbcr->crop_rect.x / 2) + ycbcr->planes[1].stride * (ycbcr->crop_rect.y / 2);
		int cr_offset = (ycbcr->crop_rect.x / 2) + ycbcr->planes[2].stride * (ycbcr->crop_rect.y / 2);

		for (unsigned int h = 0; h < ycbcr->crop_rect.height; ++h) {
			memcpy(y_data + h * ycbcr->crop_rect.width, ycbcr->planes[0].data +
				y_offset + h * ycbcr->planes[0].stride, ycbcr->crop_rect.width);
		}

		for (unsigned int h = 0; h < ycbcr->crop_rect.height / 2; ++h) {
			memcpy(cb_data + h * (ycbcr->crop_rect.width / 2), ycbcr->planes[1].data +
				cb_offset + h * ycbcr->planes[1].stride, ycbcr->crop_rect.width / 2);
			memcpy(cr_data + h * (ycbcr->crop_rect.width / 2), ycbcr->planes[2].data +
				cr_offset + h * ycbcr->planes[2].stride, ycbcr->crop_rect.width / 2);
		}

		ce_semaphore_release(videoinstance->prepared_frames, 1);
	}
}

ce_videoinstance* ce_videoinstance_new(ce_video_id video_id,
										ce_sound_id sound_id,
										ce_videoresource* videoresource)
{
	ce_videoinstance* videoinstance = ce_alloc_zero(sizeof(ce_videoinstance));
	videoinstance->video_id = video_id;
	videoinstance->sound_id = sound_id;
	videoinstance->frame = -1;
	videoinstance->desired_frame = -1;
	videoinstance->videoresource = videoresource;
	videoinstance->texture = ce_texture_new("frame", NULL);
	videoinstance->rgba = ce_mmpfile_new(videoresource->width,
										videoresource->height, 1,
										CE_MMPFILE_FORMAT_R8G8B8A8, 0);
	for (size_t i = 0; i < CE_VIDEOINSTANCE_CACHE_SIZE; ++i) {
		videoinstance->frames[i] = ce_mmpfile_new(videoresource->width,
												videoresource->height, 1,
												CE_MMPFILE_FORMAT_YCBCR, 0);
	}
	videoinstance->prepared_frames = ce_semaphore_new(0);
	videoinstance->unprepared_frames = ce_semaphore_new(CE_VIDEOINSTANCE_CACHE_SIZE);
	videoinstance->thread = ce_thread_new(ce_videoinstance_exec, videoinstance);

	return videoinstance;
}

void ce_videoinstance_del(ce_videoinstance* videoinstance)
{
	if (NULL != videoinstance) {
		videoinstance->done = true;

		ce_semaphore_release(videoinstance->unprepared_frames, 1);
		ce_thread_wait(videoinstance->thread);

		ce_thread_del(videoinstance->thread);
		ce_semaphore_del(videoinstance->unprepared_frames);
		ce_semaphore_del(videoinstance->prepared_frames);

		for (size_t i = 0; i < CE_VIDEOINSTANCE_CACHE_SIZE; ++i) {
			ce_mmpfile_del(videoinstance->frames[i]);
		}

		ce_mmpfile_del(videoinstance->rgba);
		ce_texture_del(videoinstance->texture);
		ce_videoresource_del(videoinstance->videoresource);

		ce_free(videoinstance, sizeof(ce_videoinstance));
	}
}

static inline void ce_videoinstance_replace_texture(ce_videoinstance* videoinstance)
{
	ce_mmpfile* ycbcr = videoinstance->frames[videoinstance->frame %
												CE_VIDEOINSTANCE_CACHE_SIZE];
	// TODO: shader ?
	ce_mmpfile_convert2(ycbcr, videoinstance->rgba);
	ce_texture_replace(videoinstance->texture, videoinstance->rgba);
}

static void ce_videoinstance_do_advance(ce_videoinstance* videoinstance)
{
	videoinstance->desired_frame = videoinstance->videoresource->fps *
									videoinstance->time;

	// if sound or time far away
	while (videoinstance->frame < videoinstance->desired_frame &&
			ce_semaphore_try_acquire(videoinstance->prepared_frames, 1)) {
		// skip frames to reach desired frame
		if (++videoinstance->frame == videoinstance->desired_frame ||
				// or use the closest frame
				0 == ce_semaphore_available(videoinstance->prepared_frames)) {
			ce_videoinstance_replace_texture(videoinstance);
		}
		ce_semaphore_release(videoinstance->unprepared_frames, 1);
	}
}

void ce_videoinstance_advance(ce_videoinstance* videoinstance, float elapsed)
{
	if (0 != videoinstance->sound_id) {
		// sync with sound
		videoinstance->time = ce_sound_helper_time(videoinstance->sound_id);
	} else {
		videoinstance->time += elapsed;
	}

	ce_videoinstance_do_advance(videoinstance);
}

void ce_videoinstance_progress(ce_videoinstance* videoinstance, int percents)
{
	videoinstance->time = (videoinstance->videoresource->frame_count /
		videoinstance->videoresource->fps) * (0.01f * percents);

	ce_videoinstance_do_advance(videoinstance);
}

void ce_videoinstance_render(ce_videoinstance* videoinstance)
{
	ce_rendersystem_draw_video_frame(ce_root.rendersystem, videoinstance->texture);
}

bool ce_videoinstance_is_stopped(ce_videoinstance* videoinstance)
{
	return 0 != videoinstance->sound_id ?
		ce_sound_helper_is_stopped(videoinstance->sound_id) :
		CE_VIDEOINSTANCE_STATE_STOPPED == videoinstance->state;
}

void ce_videoinstance_play(ce_videoinstance* videoinstance)
{
	videoinstance->state = CE_VIDEOINSTANCE_STATE_PLAYING;
}

void ce_videoinstance_pause(ce_videoinstance* videoinstance)
{
	videoinstance->state = CE_VIDEOINSTANCE_STATE_PAUSED;
}

void ce_videoinstance_stop(ce_videoinstance* videoinstance)
{
	videoinstance->state = CE_VIDEOINSTANCE_STATE_STOPPED;
}
