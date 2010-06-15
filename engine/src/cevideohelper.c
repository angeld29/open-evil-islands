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
#include <assert.h>

#include "ceroot.h"
#include "cevideohelper.h"

void ce_video_helper_advance(ce_video_id video_id, float elapsed)
{
	ce_videoinstance* videoinstance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != videoinstance) {
		ce_videoinstance_advance(videoinstance, elapsed);
	}
}

void ce_video_helper_progress(ce_video_id video_id, int percents)
{
	ce_videoinstance* videoinstance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != videoinstance) {
		ce_videoinstance_progress(videoinstance, percents);
	}
}

void ce_video_helper_render(ce_video_id video_id)
{
	ce_videoinstance* videoinstance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != videoinstance) {
		ce_videoinstance_render(videoinstance);
	}
}

bool ce_video_helper_is_stopped(ce_video_id video_id)
{
	ce_videoinstance* videoinstance = ce_video_manager_find(ce_root.video_manager, video_id);
	return NULL != videoinstance ? ce_videoinstance_is_stopped(videoinstance) : true;
}

void ce_video_helper_play(ce_video_id video_id)
{
	ce_videoinstance* videoinstance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != videoinstance) {
		ce_videoinstance_play(videoinstance);
	}
}

void ce_video_helper_pause(ce_video_id video_id)
{
	ce_videoinstance* videoinstance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != videoinstance) {
		ce_videoinstance_pause(videoinstance);
	}
}

void ce_video_helper_stop(ce_video_id video_id)
{
	ce_videoinstance* videoinstance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != videoinstance) {
		ce_videoinstance_stop(videoinstance);
	}
}
