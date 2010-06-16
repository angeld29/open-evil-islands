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

static void ce_video_instance_exec(ce_video_instance* video_instance)
{
	for (size_t i = ce_semaphore_available(video_instance->prepared_frames); ; ++i) {
		ce_semaphore_acquire(video_instance->unprepared_frames, 1);

		if (video_instance->done) {
			break;
		}

		if (!ce_video_resource_read(video_instance->video_resource)) {
			video_instance->state = CE_VIDEO_INSTANCE_STATE_STOPPING;
			break;
		}

		ce_mmpfile* ycbcr_frame = video_instance->ycbcr_frames[i % CE_VIDEO_INSTANCE_CACHE_SIZE];
		ce_ycbcr* ycbcr = &video_instance->video_resource->ycbcr;

		unsigned char* y_data = ycbcr_frame->texels;
		unsigned char* cb_data = y_data + ycbcr_frame->width * ycbcr_frame->height;
		unsigned char* cr_data = cb_data + (ycbcr_frame->width / 2) * (ycbcr_frame->height / 2);

		int y_offset = (ycbcr->crop_rect.x & ~1) +
						ycbcr->planes[0].stride * (ycbcr->crop_rect.y & ~1);
		int cb_offset = (ycbcr->crop_rect.x / 2) +
						ycbcr->planes[1].stride * (ycbcr->crop_rect.y / 2);
		int cr_offset = (ycbcr->crop_rect.x / 2) +
						ycbcr->planes[2].stride * (ycbcr->crop_rect.y / 2);

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

		ce_semaphore_release(video_instance->prepared_frames, 1);
	}
}

ce_video_instance* ce_video_instance_new(ce_video_object video_object,
										ce_sound_id sound_id,
										ce_video_resource* video_resource)
{
	ce_video_instance* video_instance = ce_alloc_zero(sizeof(ce_video_instance));
	video_instance->video_object = video_object;
	video_instance->sound_id = sound_id;
	video_instance->frame = -1;
	video_instance->video_resource = video_resource;
	video_instance->texture = ce_texture_new("frame", NULL);
	video_instance->rgba_frame = ce_mmpfile_new(video_resource->width,
		video_resource->height, 1, CE_MMPFILE_FORMAT_R8G8B8A8, 0);
	for (size_t i = 0; i < CE_VIDEO_INSTANCE_CACHE_SIZE; ++i) {
		video_instance->ycbcr_frames[i] = ce_mmpfile_new(video_resource->width,
			video_resource->height, 1, CE_MMPFILE_FORMAT_YCBCR, 0);
	}
	video_instance->prepared_frames = ce_semaphore_new(0);
	video_instance->unprepared_frames = ce_semaphore_new(CE_VIDEO_INSTANCE_CACHE_SIZE);
	video_instance->thread = ce_thread_new(ce_video_instance_exec, video_instance);

	return video_instance;
}

void ce_video_instance_del(ce_video_instance* video_instance)
{
	if (NULL != video_instance) {
		video_instance->done = true;

		ce_semaphore_release(video_instance->unprepared_frames, 1);
		ce_thread_wait(video_instance->thread);

		ce_thread_del(video_instance->thread);
		ce_semaphore_del(video_instance->unprepared_frames);
		ce_semaphore_del(video_instance->prepared_frames);

		for (size_t i = 0; i < CE_VIDEO_INSTANCE_CACHE_SIZE; ++i) {
			ce_mmpfile_del(video_instance->ycbcr_frames[i]);
		}

		ce_mmpfile_del(video_instance->rgba_frame);
		ce_texture_del(video_instance->texture);
		ce_video_resource_del(video_instance->video_resource);

		ce_free(video_instance, sizeof(ce_video_instance));
	}
}

static void ce_video_instance_do_advance(ce_video_instance* video_instance)
{
	bool acquired = false;
	int desired_frame = video_instance->video_resource->fps * video_instance->time;

	// if sound or time far away
	while (video_instance->frame < desired_frame &&
			ce_semaphore_try_acquire(video_instance->prepared_frames, 1)) {
		// skip frames to reach desired frame
		if (++video_instance->frame == desired_frame ||
				// or use the closest frame
				0 == ce_semaphore_available(video_instance->prepared_frames)) {
			ce_mmpfile* ycbcr_frame = video_instance->ycbcr_frames
				[video_instance->frame % CE_VIDEO_INSTANCE_CACHE_SIZE];
			ce_mmpfile_convert2(ycbcr_frame, video_instance->rgba_frame);
			ce_texture_replace(video_instance->texture, video_instance->rgba_frame);
			acquired = true;
		}
		ce_semaphore_release(video_instance->unprepared_frames, 1);
	}

	// TODO: think again how to hold last frame
	if (CE_VIDEO_INSTANCE_STATE_STOPPING == video_instance->state && !acquired &&
			0 == ce_semaphore_available(video_instance->prepared_frames)) {
		video_instance->state = CE_VIDEO_INSTANCE_STATE_STOPPED;
	}
}

void ce_video_instance_advance(ce_video_instance* video_instance, float elapsed)
{
	if (0 != video_instance->sound_id) {
		// sync with sound
		video_instance->time = ce_sound_helper_time(video_instance->sound_id);
	} else {
		video_instance->time += elapsed;
	}

	ce_video_instance_do_advance(video_instance);
}

void ce_video_instance_progress(ce_video_instance* video_instance, int percents)
{
	video_instance->time = (video_instance->video_resource->frame_count /
		video_instance->video_resource->fps) * (0.01f * percents);

	ce_video_instance_do_advance(video_instance);
}

void ce_video_instance_render(ce_video_instance* video_instance)
{
	ce_rendersystem_draw_fullscreen_texture(ce_root.rendersystem, video_instance->texture);
}

bool ce_video_instance_is_stopped(ce_video_instance* video_instance)
{
	return 0 != video_instance->sound_id ?
		ce_sound_helper_is_stopped(video_instance->sound_id) :
		CE_VIDEO_INSTANCE_STATE_STOPPED == video_instance->state;
}

void ce_video_instance_play(ce_video_instance* video_instance)
{
	ce_sound_helper_play(video_instance->sound_id);
	video_instance->state = CE_VIDEO_INSTANCE_STATE_PLAYING;
}

void ce_video_instance_pause(ce_video_instance* video_instance)
{
	ce_sound_helper_pause(video_instance->sound_id);
	video_instance->state = CE_VIDEO_INSTANCE_STATE_PAUSED;
}

void ce_video_instance_stop(ce_video_instance* video_instance)
{
	ce_sound_helper_stop(video_instance->sound_id);
	video_instance->state = CE_VIDEO_INSTANCE_STATE_STOPPED;
}
