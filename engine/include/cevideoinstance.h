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

#include <vorbis/codec.h>
#include <theora/theoradec.h>

#include "cememfile.h"
#include "cethread.h"
#include "cemmpfile.h"
#include "cetexture.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
	volatile bool done;
	int width, height;
	float fps;
	int frame;
	float video_time;
	float sync_time;
	ce_memfile* memfile;
	ce_mmpfile* mmpfile;
	ce_texture* texture;
	ce_mutex* mutex;
	ce_waitcond* waitcond;
	ce_thread* thread;
	ogg_sync_state sync;
	ogg_stream_state stream;
	ogg_page page;
	ogg_packet packet;
	th_setup_info* setup;
	th_info info;
	th_comment comment;
	th_dec_ctx* context;
} ce_videoinstance;

extern ce_videoinstance* ce_videoinstance_new(ce_memfile* memfile);
extern void ce_videoinstance_del(ce_videoinstance* videoinstance);

extern void ce_videoinstance_sync(ce_videoinstance* videoinstance, float time);
extern void ce_videoinstance_advance(ce_videoinstance* videoinstance, float elapsed);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_VIDEOINSTANCE_H */
