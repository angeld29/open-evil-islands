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

#ifndef CE_VIDEOINSTANCE_H
#define CE_VIDEOINSTANCE_H

#include <stddef.h>
#include <stdbool.h>

#include "cethread.h"
#include "cemmpfile.h"
#include "cevideoresource.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum {
	CE_VIDEOINSTANCE_CACHE_SIZE = 8,
};

typedef struct {
	volatile bool done;
	float time; // synchronization/playing time
	int frame, desired_frame;
	ce_videoresource* videoresource;
	ce_mmpfile* cache[CE_VIDEOINSTANCE_CACHE_SIZE];
	ce_semaphore* prepared_frames;
	ce_semaphore* unprepared_frames;
	ce_thread* thread;
} ce_videoinstance;

extern ce_videoinstance* ce_videoinstance_new(ce_videoresource* videoresource);
extern void ce_videoinstance_del(ce_videoinstance* videoinstance);

// performs synchronization with other streams
extern void ce_videoinstance_sync(ce_videoinstance* videoinstance, float time);

// advances a video based on its FPS
extern void ce_videoinstance_advance(ce_videoinstance* videoinstance, float elapsed);

extern ce_mmpfile* ce_videoinstance_acquire_frame(ce_videoinstance* videoinstance);
extern void ce_videoinstance_release_frame(ce_videoinstance* videoinstance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VIDEOINSTANCE_H */
