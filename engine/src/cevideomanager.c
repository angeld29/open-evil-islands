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
#include "cevideomanager.h"

ce_video_manager* ce_video_manager_new(const char* path)
{
	ce_video_manager* video_manager = ce_alloc_zero(sizeof(ce_video_manager));
	video_manager->path = ce_string_new_str(path);
	video_manager->video_instances = ce_vector_new();
	ce_logging_write("video manager: root path is '%s'", path);
	return video_manager;
}

void ce_video_manager_del(ce_video_manager* video_manager)
{
	if (NULL != video_manager) {
		ce_vector_for_each(video_manager->video_instances, ce_videoinstance_del);
		ce_vector_del(video_manager->video_instances);
		ce_string_del(video_manager->path);
		ce_free(video_manager, sizeof(ce_video_manager));
	}
}

void ce_video_manager_advance(ce_video_manager* video_manager, float elapsed)
{
	ce_unused(elapsed);

	for (size_t i = 0; i < video_manager->video_instances->count; ++i) {
		ce_videoinstance* videoinstance = video_manager->video_instances->items[i];
		if (ce_videoinstance_is_stopped(videoinstance)) {
			ce_videoinstance_del(videoinstance);
			ce_vector_remove_unordered(video_manager->video_instances, i--);
		}
	}
}

static ce_memfile* ce_video_manager_open(ce_video_manager* video_manager, const char* name)
{
	char path[video_manager->path->length + strlen(name) + 16];
	const char* extensions[] = {"ogv", "bik"};

	for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); ++i) {
		snprintf(path, sizeof(path), "%s/%s.%s", video_manager->path->str, name, extensions[i]);
		ce_memfile* memfile = ce_memfile_open_path(path);
		if (NULL != memfile) {
			return memfile;
		}
	}

	return NULL;
}

ce_video_id ce_video_manager_create(ce_video_manager* video_manager, const char* name)
{
	ce_memfile* memfile = ce_video_manager_open(video_manager, name);
	if (NULL == memfile) {
		return 0;
	}

	ce_videoresource* video_resource = ce_videoresource_new(memfile);
	if (NULL == video_resource) {
		ce_logging_error("video manager: could not find decoder for '%s'", name);
		ce_memfile_close(memfile);
		return 0;
	}

	ce_video_id video_id = ++video_manager->last_video_id;
	ce_sound_id sound_id = ce_soundmanager_create(ce_root.soundmanager, name);

	ce_videoinstance* video_instance = ce_videoinstance_new(video_id, sound_id, video_resource);
	if (NULL == video_instance) {
		ce_logging_error("video manager: could not create instance for '%s'", name);
		ce_videoresource_del(video_resource);
		return 0;
	}

	ce_vector_push_back(video_manager->video_instances, video_instance);

	return video_id;
}

ce_videoinstance* ce_video_manager_find(ce_video_manager* video_manager, ce_video_id video_id)
{
	for (size_t i = 0; i < video_manager->video_instances->count; ++i) {
		ce_videoinstance* videoinstance = video_manager->video_instances->items[i];
		if (video_id == videoinstance->video_id) {
			return videoinstance;
		}
	}
	return NULL;
}
