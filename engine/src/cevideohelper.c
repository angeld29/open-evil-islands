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
	ce_video_instance* video_instance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != video_instance) {
		ce_video_instance_advance(video_instance, elapsed);
	}
}

void ce_video_helper_progress(ce_video_id video_id, int percents)
{
	ce_video_instance* video_instance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != video_instance) {
		ce_video_instance_progress(video_instance, percents);
	}
}

void ce_video_helper_render(ce_video_id video_id)
{
	ce_video_instance* video_instance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != video_instance) {
		ce_video_instance_render(video_instance);
	}
}

bool ce_video_helper_is_stopped(ce_video_id video_id)
{
	ce_video_instance* video_instance = ce_video_manager_find(ce_root.video_manager, video_id);
	return NULL != video_instance ? ce_video_instance_is_stopped(video_instance) : true;
}

void ce_video_helper_play(ce_video_id video_id)
{
	ce_video_instance* video_instance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != video_instance) {
		ce_video_instance_play(video_instance);
	}
}

void ce_video_helper_pause(ce_video_id video_id)
{
	ce_video_instance* video_instance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != video_instance) {
		ce_video_instance_pause(video_instance);
	}
}

void ce_video_helper_stop(ce_video_id video_id)
{
	ce_video_instance* video_instance = ce_video_manager_find(ce_root.video_manager, video_id);
	if (NULL != video_instance) {
		ce_video_instance_stop(video_instance);
	}
}
