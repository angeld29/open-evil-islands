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

#ifndef CE_VIDEOHELPER_H
#define CE_VIDEOHELPER_H

#include "cevideo.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void ce_video_helper_advance(ce_video_id video_id, float elapsed);
extern void ce_video_helper_progress(ce_video_id video_id, int percents);

extern void ce_video_helper_render(ce_video_id video_id);

extern bool ce_video_helper_is_stopped(ce_video_id video_id);

extern void ce_video_helper_play(ce_video_id video_id);
extern void ce_video_helper_pause(ce_video_id video_id);
extern void ce_video_helper_stop(ce_video_id video_id);

#ifdef __cplusplus
}
#endif

#endif /* CE_VIDEOHELPER_H */
