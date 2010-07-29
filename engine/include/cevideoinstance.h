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

#ifndef CE_VIDEOINSTANCE_H
#define CE_VIDEOINSTANCE_H

#include <stddef.h>
#include <stdbool.h>

#include "cethread.h"
#include "cemmpfile.h"
#include "cetexture.h"
#include "cematerial.h"
#include "cesoundobject.h"
#include "cevideoobject.h"
#include "cevideoresource.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	CE_VIDEO_INSTANCE_CACHE_SIZE = 8,
};

enum {
	CE_VIDEO_INSTANCE_STATE_STOPPED,
	CE_VIDEO_INSTANCE_STATE_STOPPING,
	CE_VIDEO_INSTANCE_STATE_PAUSED,
	CE_VIDEO_INSTANCE_STATE_PLAYING,
};

typedef struct {
	ce_video_object video_object;
	ce_sound_object* sound_object;
	int state, frame;
	float play_time, sync_time; // playing/synchronization time in seconds
	ce_video_resource* video_resource;
	ce_texture* texture;
	ce_material* material;
	ce_mmpfile* rgba_frame;
	ce_mmpfile* ycbcr_frames[CE_VIDEO_INSTANCE_CACHE_SIZE];
	ce_semaphore* prepared_frames;
	ce_semaphore* unprepared_frames;
	ce_thread* thread;
	volatile bool done;
} ce_video_instance;

extern ce_video_instance* ce_video_instance_new(ce_video_object video_object,
												ce_sound_object* sound_object,
												ce_video_resource* video_resource);
extern void ce_video_instance_del(ce_video_instance* video_instance);

extern void ce_video_instance_advance(ce_video_instance* video_instance, float elapsed);
extern void ce_video_instance_progress(ce_video_instance* video_instance, int percents);

extern void ce_video_instance_render(ce_video_instance* video_instance);

extern bool ce_video_instance_is_stopped(ce_video_instance* video_instance);

extern void ce_video_instance_play(ce_video_instance* video_instance);
extern void ce_video_instance_pause(ce_video_instance* video_instance);
extern void ce_video_instance_stop(ce_video_instance* video_instance);

#ifdef __cplusplus
}
#endif

#endif /* CE_VIDEOINSTANCE_H */
