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
#include "cesoundmanager.h"

static const char* ce_sound_dirs[] = {"Stream", "Movies"};
static const char* ce_sound_exts[] = {".wav", ".oga", ".mp3", ".ogv", ".bik"};

enum {
	CE_SOUND_DIR_COUNT = sizeof(ce_sound_dirs) / sizeof(ce_sound_dirs[0]),
	CE_SOUND_EXT_COUNT = sizeof(ce_sound_exts) / sizeof(ce_sound_exts[0]),
};

ce_soundmanager* ce_soundmanager_new(void)
{
	char path[ce_root.ei_path->length + 16];

	for (size_t i = 0; i < CE_SOUND_DIR_COUNT; ++i) {
		snprintf(path, sizeof(path), "%s/%s",
			ce_root.ei_path->str, ce_sound_dirs[i]);
		ce_logging_write("sound manager: using path '%s'", path);
	}

	ce_soundmanager* soundmanager = ce_alloc_zero(sizeof(ce_soundmanager));
	soundmanager->soundinstances = ce_vector_new();
	return soundmanager;
}

void ce_soundmanager_del(ce_soundmanager* soundmanager)
{
	if (NULL != soundmanager) {
		ce_vector_for_each(soundmanager->soundinstances, ce_soundinstance_del);
		ce_vector_del(soundmanager->soundinstances);
		ce_free(soundmanager, sizeof(ce_soundmanager));
	}
}

void ce_soundmanager_advance(ce_soundmanager* soundmanager, float elapsed)
{
	ce_unused(elapsed);

	for (size_t i = 0; i < soundmanager->soundinstances->count; ++i) {
		ce_soundinstance* sound_instance = soundmanager->soundinstances->items[i];
		if (ce_soundinstance_is_stopped(sound_instance)) {
			ce_soundinstance_del(sound_instance);
			ce_vector_remove_unordered(soundmanager->soundinstances, i--);
		}
	}
}

static ce_memfile* ce_soundmanager_open(const char* name)
{
	char path[ce_root.ei_path->length + strlen(name) + 32];

	for (size_t i = 0; i < CE_SOUND_DIR_COUNT; ++i) {
		for (size_t j = 0; j < CE_SOUND_EXT_COUNT; ++j) {
			snprintf(path, sizeof(path), "%s/%s/%s%s",
				ce_root.ei_path->str, ce_sound_dirs[i], name, ce_sound_exts[j]);
			ce_memfile* memfile = ce_memfile_open_path(path);
			if (NULL != memfile) {
				return memfile;
			}
		}
	}

	return NULL;
}

ce_sound_id ce_soundmanager_create(ce_soundmanager* soundmanager, const char* name)
{
	ce_memfile* memfile = ce_soundmanager_open(name);
	if (NULL == memfile) {
		return 0;
	}

	ce_soundresource* soundresource = ce_soundresource_new_builtin(memfile);
	if (NULL == soundresource) {
		ce_memfile_close(memfile);
		return 0;
	}

	ce_soundinstance* soundinstance =
		ce_soundinstance_new(++soundmanager->last_sound_id, soundresource);
	if (NULL == soundinstance) {
		ce_logging_error("sound manager: could not create instance for '%s'", name);
		ce_soundresource_del(soundresource);
		return 0;
	}

	ce_vector_push_back(soundmanager->soundinstances, soundinstance);

	return soundinstance->sound_id;
}

ce_soundinstance* ce_soundmanager_find(ce_soundmanager* soundmanager, ce_sound_id sound_id)
{
	for (size_t i = 0; i < soundmanager->soundinstances->count; ++i) {
		ce_soundinstance* sound_instance = soundmanager->soundinstances->items[i];
		if (sound_id == sound_instance->sound_id) {
			return sound_instance;
		}
	}
	return NULL;
}
