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
#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"
#include "ceoptionmanager.h"
#include "cesoundmanager.h"
#include "cevideomanager.h"

static const char* ce_video_dirs[] = {"Movies"};
static const char* ce_video_exts[] = {".ogv", ".bik"};

enum {
	CE_VIDEO_DIR_COUNT = sizeof(ce_video_dirs) / sizeof(ce_video_dirs[0]),
	CE_VIDEO_EXT_COUNT = sizeof(ce_video_exts) / sizeof(ce_video_exts[0]),
};

struct ce_video_manager* ce_video_manager;

void ce_video_manager_init(void)
{
	char path[ce_option_manager->ei_path->length + 16];

	for (size_t i = 0; i < CE_VIDEO_DIR_COUNT; ++i) {
		snprintf(path, sizeof(path), "%s/%s",
			ce_option_manager->ei_path->str, ce_video_dirs[i]);
		ce_logging_write("video manager: using path '%s'", path);
	}

	ce_video_manager = ce_alloc_zero(sizeof(struct ce_video_manager));
	ce_video_manager->video_instances = ce_vector_new();
}

void ce_video_manager_term(void)
{
	if (NULL != ce_video_manager) {
		ce_vector_for_each(ce_video_manager->video_instances, ce_video_instance_del);
		ce_vector_del(ce_video_manager->video_instances);
		ce_free(ce_video_manager, sizeof(struct ce_video_manager));
	}
}

void ce_video_manager_advance(float elapsed)
{
	ce_unused(elapsed);

	for (size_t i = 0; i < ce_video_manager->video_instances->count; ++i) {
		//ce_video_instance* video_instance = video_manager->video_instances->items[i];
		/*if (ce_video_instance_is_stopped(video_instance)) {
			ce_video_instance_del(video_instance);
			ce_vector_remove_unordered(video_manager->video_instances, i--);
		}*/
	}
}

static ce_memfile* ce_video_manager_open(const char* name)
{
	char path[ce_option_manager->ei_path->length + strlen(name) + 32];

	for (size_t i = 0; i < CE_VIDEO_DIR_COUNT; ++i) {
		for (size_t j = 0; j < CE_VIDEO_EXT_COUNT; ++j) {
			snprintf(path, sizeof(path), "%s/%s/%s",
				ce_option_manager->ei_path->str, ce_video_dirs[i], name);
			if (NULL == ce_strcasestr(path, ce_video_exts[j])) {
				ce_strlcat(path, ce_video_exts[j], sizeof(path));
			}
			ce_memfile* memfile = ce_memfile_open_path(path);
			if (NULL != memfile) {
				return memfile;
			}
		}
	}

	return NULL;
}

ce_video_object ce_video_manager_create(const char* name)
{
	ce_memfile* memfile = ce_video_manager_open(name);
	if (NULL == memfile) {
		return 0;
	}

	ce_video_resource* video_resource = ce_video_resource_new(memfile);
	if (NULL == video_resource) {
		ce_memfile_close(memfile);
		return 0;
	}

	ce_video_object video_object = ++ce_video_manager->last_video_object;
	ce_sound_object sound_object = ce_sound_manager_create(name);

	ce_video_instance* video_instance = ce_video_instance_new(video_object, sound_object, video_resource);
	if (NULL == video_instance) {
		ce_logging_error("video manager: could not create instance for '%s'", name);
		ce_video_resource_del(video_resource);
		return 0;
	}

	ce_vector_push_back(ce_video_manager->video_instances, video_instance);

	return video_object;
}

ce_video_instance* ce_video_manager_find(ce_video_object video_object)
{
	for (size_t i = 0; i < ce_video_manager->video_instances->count; ++i) {
		ce_video_instance* video_instance = ce_video_manager->video_instances->items[i];
		if (video_object == video_instance->video_object) {
			return video_instance;
		}
	}
	return NULL;
}
