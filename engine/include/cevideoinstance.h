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
#include "cesound.h"
#include "cevideo.h"
#include "cevideoresource.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	CE_VIDEOINSTANCE_CACHE_SIZE = 8,
};

enum {
	CE_VIDEOINSTANCE_STATE_STOPPED,
	CE_VIDEOINSTANCE_STATE_STOPPING,
	CE_VIDEOINSTANCE_STATE_PAUSED,
	CE_VIDEOINSTANCE_STATE_PLAYING,
};

typedef struct {
	ce_video_id video_id;
	ce_sound_id sound_id;
	int state;
	float time; // synchronization/playing time in seconds
	int frame;
	ce_videoresource* videoresource;
	ce_texture* texture;
	ce_mmpfile* rgba_frame; // TODO: shader
	ce_mmpfile* ycbcr_frames[CE_VIDEOINSTANCE_CACHE_SIZE];
	ce_semaphore* prepared_frames;
	ce_semaphore* unprepared_frames;
	ce_thread* thread;
	volatile bool done;
} ce_videoinstance;

extern ce_videoinstance* ce_videoinstance_new(ce_video_id video_id,
												ce_sound_id sound_id,
												ce_videoresource* videoresource);
extern void ce_videoinstance_del(ce_videoinstance* videoinstance);

extern void ce_videoinstance_advance(ce_videoinstance* videoinstance, float elapsed);
extern void ce_videoinstance_progress(ce_videoinstance* videoinstance, int percents);

extern void ce_videoinstance_render(ce_videoinstance* videoinstance);

extern bool ce_videoinstance_is_stopped(ce_videoinstance* videoinstance);

extern void ce_videoinstance_play(ce_videoinstance* videoinstance);
extern void ce_videoinstance_pause(ce_videoinstance* videoinstance);
extern void ce_videoinstance_stop(ce_videoinstance* videoinstance);

#ifdef __cplusplus
}
#endif

#endif /* CE_VIDEOINSTANCE_H */
