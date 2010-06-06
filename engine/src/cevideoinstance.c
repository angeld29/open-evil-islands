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
#include <assert.h>

#include "cealloc.h"
#include "celogging.h"
#include "cevideoinstance.h"

static void ce_videoinstance_exec(ce_videoinstance* videoinstance)
{
	for (size_t i = ce_semaphore_available(videoinstance->prepared_frames); ; ++i) {
		ce_semaphore_acquire(videoinstance->unprepared_frames, 1);

		if (videoinstance->done) {
			break;
		}

		size_t j = i % CE_VIDEOINSTANCE_CACHE_SIZE;
		if (!ce_videoresource_read(videoinstance->videoresource,
									videoinstance->cache[j]->texels)) {
			break;
		}

		ce_semaphore_release(videoinstance->prepared_frames, 1);
	}
}

ce_videoinstance* ce_videoinstance_new(ce_videoresource* videoresource)
{
	ce_videoinstance* videoinstance = ce_alloc_zero(sizeof(ce_videoinstance));
	videoinstance->frame = -1;
	videoinstance->desired_frame = -1;
	videoinstance->videoresource = videoresource;
	for (size_t i = 0; i < CE_VIDEOINSTANCE_CACHE_SIZE; ++i) {
		videoinstance->cache[i] = ce_mmpfile_new(videoresource->width,
												videoresource->height, 1,
												CE_MMPFILE_FORMAT_R8G8B8A8, 0);
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
			ce_mmpfile_del(videoinstance->cache[i]);
		}

		ce_videoresource_del(videoinstance->videoresource);
		ce_free(videoinstance, sizeof(ce_videoinstance));
	}
}

static inline void ce_videoinstance_advance_frame(ce_videoinstance* videoinstance)
{
	videoinstance->desired_frame = videoinstance->videoresource->fps *
									videoinstance->time;
}

void ce_videoinstance_sync(ce_videoinstance* videoinstance, float time)
{
	videoinstance->time = time;
	ce_videoinstance_advance_frame(videoinstance);
}

void ce_videoinstance_advance(ce_videoinstance* videoinstance, float elapsed)
{
	videoinstance->time += elapsed;
	ce_videoinstance_advance_frame(videoinstance);
}

ce_mmpfile* ce_videoinstance_acquire_frame(ce_videoinstance* videoinstance)
{
	// if other streams (or time) far away, skip frames to reach desired frame
	if (videoinstance->frame < videoinstance->desired_frame) {
		while (ce_semaphore_try_acquire(videoinstance->prepared_frames, 1)) {
			if (++videoinstance->frame == videoinstance->desired_frame ||
					// or return the closest frame if possible
					0 == ce_semaphore_available(videoinstance->prepared_frames)) {
				return videoinstance->cache[videoinstance->frame %
											CE_VIDEOINSTANCE_CACHE_SIZE];
			}
			ce_semaphore_release(videoinstance->unprepared_frames, 1);
		}
		// frame is not ready - wait for it!
	}
	return NULL;
}

void ce_videoinstance_release_frame(ce_videoinstance* videoinstance)
{
	ce_semaphore_release(videoinstance->unprepared_frames, 1);
}
